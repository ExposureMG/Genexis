#include "backend/adapters/Ftdi2SpiAdapter.hpp"

#include <filesystem>
#include <mutex>
#include <string>

// Declarations from FTDI2SPI
extern "C" {
int spi(int mode, int size, char *file, int startblock, int length);
int emmc_read(const char *file, int startblock, int length);
int emmc_write(const char *file, int startblock, int length);
}

namespace gxapi::backend {

namespace {
static std::mutex g_ftdiMutex;

static std::string interpretFtdiError(int rc) {
  switch (rc) {
  case -1:
    return "FTDI operation interrupted or stopped";
  case -2:
    return "Unable to initialize FTDI hardware device (xFlasher not detected "
           "or busy)";
  case -3:
    return "Bad connection to NAND/eMMC flash chip (check wiring / solder "
           "points)";
  case -4:
    return "Unknown or unsupported NAND flash configuration";
  case -8:
    return "Unable to open target file for reading or writing";
  case -9:
    return "Memory allocation error during FTDI buffer preparation";
  case -10:
    return "Invalid FTDI operation mode requested";
  case -11:
    return "File write stream failure";
  default:
    return "FTDI operation failed with error code: " + std::to_string(rc);
  }
}
} // namespace

std::expected<FlashInfo, std::string>
Ftdi2SpiAdapter::getFlashInfo(const FlashDeviceConfig &config) {
  std::lock_guard<std::mutex> lock(g_ftdiMutex);
  (void)config;

  int rc = spi(0, 16, nullptr, 0, 0); // mode 0 = Get Flash Config Only
  if (rc != 0) {
    return std::unexpected(interpretFtdiError(rc));
  }

  FlashInfo info;
  info.hardwareName = "xFlasher (FTDI2SPI)";
  info.flashType = "Xbox 360 SFC NAND / eMMC";
  return info;
}

std::expected<void, std::string>
Ftdi2SpiAdapter::readNand(const std::filesystem::path &outputPath,
                          uint32_t startBlock, uint32_t blockCount,
                          const FlashDeviceConfig &config,
                          FlashProgressCallback progressCb) {
  std::lock_guard<std::mutex> lock(g_ftdiMutex);

  std::string pathStr = outputPath.string();

  if (progressCb) {
    FlashProgressInfo pInfo{.bytesDone = 0,
                            .totalBytes = 0,
                            .percentage = 0.0f,
                            .statusMessage = "Initializing FTDI read..."};
    progressCb(pInfo);
  }

  int rc = 0;
  if (config.media == FlashMediaType::Emmc) {
    rc = emmc_read(pathStr.c_str(), static_cast<int>(startBlock),
                   static_cast<int>(blockCount));
  } else {
    // Mode 1: read with spare (0x210 block size, standard
    // 16MB/64MB/256MB/512MB)
    int sizeMb =
        (blockCount > 0) ? static_cast<int>((blockCount * 16) / 1024) : 16;
    if (sizeMb < 16)
      sizeMb = 16;
    rc = spi(1, sizeMb, const_cast<char *>(pathStr.c_str()),
             static_cast<int>(startBlock), static_cast<int>(blockCount));
  }

  if (rc != 0) {
    return std::unexpected(interpretFtdiError(rc));
  }

  if (progressCb) {
    FlashProgressInfo pInfo{.bytesDone = 100,
                            .totalBytes = 100,
                            .percentage = 100.0f,
                            .statusMessage = "FTDI Read complete."};
    progressCb(pInfo);
  }

  return {};
}

std::expected<void, std::string>
Ftdi2SpiAdapter::writeNand(const std::filesystem::path &inputPath,
                           uint32_t startBlock, bool eraseFirst,
                           bool verifyAfter, const FlashDeviceConfig &config,
                           FlashProgressCallback progressCb) {
  (void)eraseFirst;
  (void)verifyAfter;

  if (!std::filesystem::exists(inputPath)) {
    return std::unexpected("Input file does not exist: " + inputPath.string());
  }

  std::lock_guard<std::mutex> lock(g_ftdiMutex);
  std::string pathStr = inputPath.string();

  if (progressCb) {
    FlashProgressInfo pInfo{.bytesDone = 0,
                            .totalBytes = 0,
                            .percentage = 0.0f,
                            .statusMessage = "Initializing FTDI write..."};
    progressCb(pInfo);
  }

  int rc = 0;
  if (config.media == FlashMediaType::Emmc) {
    rc = emmc_write(pathStr.c_str(), static_cast<int>(startBlock), 0);
  } else {
    // Mode 3: standard write, Mode 4: write with ECC patch
    rc = spi(3, 16, const_cast<char *>(pathStr.c_str()),
             static_cast<int>(startBlock), 0);
  }

  if (rc != 0) {
    return std::unexpected(interpretFtdiError(rc));
  }

  if (progressCb) {
    FlashProgressInfo pInfo{.bytesDone = 100,
                            .totalBytes = 100,
                            .percentage = 100.0f,
                            .statusMessage = "FTDI Write complete."};
    progressCb(pInfo);
  }

  return {};
}

std::expected<void, std::string>
Ftdi2SpiAdapter::eraseNand(uint32_t startBlock, uint32_t blockCount,
                           const FlashDeviceConfig &config) {
  (void)config;
  std::lock_guard<std::mutex> lock(g_ftdiMutex);

  int sizeMb =
      (blockCount > 0) ? static_cast<int>((blockCount * 16) / 1024) : 16;
  if (sizeMb < 16)
    sizeMb = 16;

  int rc = spi(5, sizeMb, nullptr, static_cast<int>(startBlock),
               static_cast<int>(blockCount));
  if (rc != 0) {
    return std::unexpected(interpretFtdiError(rc));
  }
  return {};
}

} // namespace gxapi::backend
