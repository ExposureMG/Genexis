#include "backend/adapters/Ftdi2SpiAdapter.hpp"
#include "backend/adapters/Ftdi2SpiApi.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <optional>
#include <string>
#include <system_error>
#include <thread>
#include <vector>

namespace gxapi::backend {

namespace {
std::mutex g_ftdiMutex;

// FTDI2SPI addresses SPI flash in 16 KB units (32 pages of 0x200 bytes).
constexpr uint64_t kUnitBytes = 16 * 1024;
// Raw NAND page as stored in dumps: 0x200 data + 0x10 spare.
constexpr uint64_t kRawPageBytes = 0x210;
constexpr uint64_t kPagesPerUnit = 32;
constexpr uint64_t kEmmcBlockBytes = 512;

// spi() modes
constexpr int kSpiModeConfig = 0;
constexpr int kSpiModeRead = 1;
constexpr int kSpiModeWrite = 3;
constexpr int kSpiModeErase = 5;

std::string interpretFtdiError(int rc) {
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
  case -12:
    return "USB transfer to the FTDI device failed (device unplugged or "
           "unresponsive)";
  case -20:
    return "eMMC block transfer failed";
  case -21:
    return "File read/write failure during eMMC transfer";
  case -30:
    return "Unable to determine input file size";
  default:
    return "FTDI operation failed with error code: " + std::to_string(rc);
  }
}

// Total NAND size in bytes for a flash config word. Mirrors SFC_init() in
// extern/FTDI2SPI/src/sfc.cpp, which does not export the result.
std::optional<uint64_t> nandBytesFromConfig(uint32_t config) {
  const uint32_t sfcType = (config >> 17) & 0x3;
  const uint32_t sizeSel = (config >> 4) & 0x3;
  constexpr uint64_t MB = 1024 * 1024;

  if (sfcType == 3)
    return std::nullopt;

  if (sfcType == 0) {
    switch (sizeSel) {
    case 1:
      return 16 * MB;
    case 2:
      return 32 * MB;
    case 3:
      return 64 * MB;
    default:
      return std::nullopt;
    }
  }

  switch (sizeSel) {
  case 0:
    return sfcType == 1 ? std::nullopt : std::optional<uint64_t>(16 * MB);
  case 1:
    return sfcType == 1 ? 16 * MB : 64 * MB;
  default: // 2, 3: large block, size encoded in the config word
    return uint64_t{1} << (((config >> 19) & 0x3) + ((config >> 21) & 0xF) +
                           0x17);
  }
}

void report(const FlashProgressCallback &cb, uint64_t done, uint64_t total,
            float pct, std::string msg) {
  if (!cb)
    return;
  cb(FlashProgressInfo{.bytesDone = done,
                       .totalBytes = total,
                       .percentage = pct,
                       .statusMessage = std::move(msg)});
}

// Polls the blocking FTDI2SPI call from a helper thread and forwards progress.
// `current` returns the library's position or a negative value when idle.
class ProgressPoller {
public:
  ProgressPoller(FlashProgressCallback cb, int (*current)(), uint64_t first,
                 uint64_t total, uint64_t unitBytes, float pctBase,
                 float pctSpan, std::string message)
      : m_cb(std::move(cb)) {
    if (!m_cb || total == 0)
      return;
    m_thread = std::jthread([=, this](std::stop_token st) {
      int last = -1;
      while (!st.stop_requested()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
        const int pos = current();
        if (pos < 0 || pos == last)
          continue;
        last = pos;
        const uint64_t done =
            std::min<uint64_t>(total, pos > static_cast<int64_t>(first)
                                          ? pos - first
                                          : 0);
        report(m_cb, done * unitBytes, total * unitBytes,
               pctBase + pctSpan * static_cast<float>(done) /
                             static_cast<float>(total),
               message);
      }
    });
  }

private:
  FlashProgressCallback m_cb;
  std::jthread m_thread; // joins (after requesting stop) on destruction
};

int currentSpiUnit() { return spiGetBlocks(); }
int currentEmmcBlock() { return static_cast<int>(emmcGetBlocks()); }

// Removes a temporary readback file on scope exit.
struct TempFile {
  std::filesystem::path path;
  explicit TempFile(const char *tag) {
    const auto stamp =
        std::chrono::steady_clock::now().time_since_epoch().count();
    path = std::filesystem::temp_directory_path() /
           (std::string("genexis_") + tag + "_" + std::to_string(stamp) +
            ".bin");
  }
  ~TempFile() {
    std::error_code ec;
    std::filesystem::remove(path, ec);
  }
};

// Compares the first `length` bytes of two files.
std::expected<void, std::string> compareFiles(const std::filesystem::path &a,
                                              const std::filesystem::path &b,
                                              uint64_t length) {
  std::ifstream fa(a, std::ios::binary), fb(b, std::ios::binary);
  if (!fa || !fb)
    return std::unexpected("Verify failed: unable to open files for compare");

  constexpr size_t kChunk = 1 << 20;
  std::vector<char> ba(kChunk), bb(kChunk);
  uint64_t offset = 0;
  while (offset < length) {
    const size_t want =
        static_cast<size_t>(std::min<uint64_t>(kChunk, length - offset));
    fa.read(ba.data(), want);
    fb.read(bb.data(), want);
    if (static_cast<size_t>(fa.gcount()) != want ||
        static_cast<size_t>(fb.gcount()) != want)
      return std::unexpected("Verify failed: readback is shorter than the "
                             "written data (at offset " +
                             std::to_string(offset) + ")");
    for (size_t i = 0; i < want; ++i) {
      if (ba[i] != bb[i]) {
        char buf[64];
        std::snprintf(buf, sizeof buf, "%llx",
                      static_cast<unsigned long long>(offset + i));
        return std::unexpected(
            std::string("Verify failed: data mismatch at offset 0x") + buf);
      }
    }
    offset += want;
  }
  return {};
}

// Reads the flash config word from the connected NAND. Caller holds the mutex.
std::expected<uint32_t, std::string> readConfig() {
  const int rc = spi(kSpiModeConfig, 16, nullptr, 0, 0);
  if (rc != 0)
    return std::unexpected(interpretFtdiError(rc));
  return static_cast<uint32_t>(spiGetConfig());
}

// Reads the config word and resolves the NAND size. Caller holds the mutex.
std::expected<uint64_t, std::string> detectNandBytes() {
  auto cfg = readConfig();
  if (!cfg)
    return std::unexpected(cfg.error());
  auto bytes = nandBytesFromConfig(*cfg);
  if (!bytes)
    return std::unexpected(interpretFtdiError(-4));
  return *bytes;
}

// Resolves [startBlock, startBlock + blockCount) in 16 KB units against the
// NAND size; blockCount == 0 means "to the end of the NAND".
std::expected<uint64_t, std::string>
resolveUnitCount(uint32_t startBlock, uint32_t blockCount, uint64_t nandBytes) {
  const uint64_t totalUnits = nandBytes / kUnitBytes;
  if (startBlock >= totalUnits)
    return std::unexpected("Start block " + std::to_string(startBlock) +
                           " is beyond the end of the NAND (" +
                           std::to_string(totalUnits) + " blocks)");
  const uint64_t count = blockCount ? blockCount : totalUnits - startBlock;
  if (startBlock + count > totalUnits)
    return std::unexpected("Requested range exceeds the NAND size (" +
                           std::to_string(totalUnits) + " blocks)");
  return count;
}
} // namespace

std::expected<FlashInfo, std::string>
Ftdi2SpiAdapter::getFlashInfo(const FlashDeviceConfig &config) {
  std::lock_guard<std::mutex> lock(g_ftdiMutex);

  FlashInfo info;
  info.hardwareName = "xFlasher (FTDI2SPI)";

  if (config.media == FlashMediaType::Emmc) {
    // The library cannot report eMMC capacity; probe by reading one block.
    TempFile tmp("probe");
    const int rc = emmc_read(tmp.path.string().c_str(), 0, 1);
    if (rc != 0)
      return std::unexpected(interpretFtdiError(rc));
    info.flashType = "Xbox 360 eMMC";
    return info;
  }

  auto cfg = readConfig();
  if (!cfg)
    return std::unexpected(cfg.error());

  info.flashType = "Xbox 360 SFC NAND";
  info.configWord = *cfg;
  if (auto bytes = nandBytesFromConfig(*cfg)) {
    info.totalBytes = *bytes;
    info.totalBlocks = static_cast<uint32_t>(*bytes / kUnitBytes);
  }
  return info;
}

std::expected<void, std::string>
Ftdi2SpiAdapter::readNand(const std::filesystem::path &outputPath,
                          uint32_t startBlock, uint32_t blockCount,
                          const FlashDeviceConfig &config,
                          FlashProgressCallback progressCb) {
  std::lock_guard<std::mutex> lock(g_ftdiMutex);
  const std::string pathStr = outputPath.string();

  report(progressCb, 0, 0, 0.0f, "Initializing FTDI read...");

  int rc = 0;
  if (config.media == FlashMediaType::Emmc) {
    // emmc_read treats a zero length as "unbounded" and the library has no way
    // to query eMMC capacity, so the caller must say how much to read.
    if (blockCount == 0)
      return std::unexpected(
          "eMMC read needs an explicit block count (512-byte blocks); the "
          "eMMC size cannot be detected");
    ProgressPoller poller(progressCb, currentEmmcBlock, startBlock, blockCount,
                          kEmmcBlockBytes, 0.0f, 100.0f, "Reading eMMC...");
    rc = emmc_read(pathStr.c_str(), static_cast<int>(startBlock),
                   static_cast<int>(blockCount));
  } else {
    auto nandBytes = detectNandBytes();
    if (!nandBytes)
      return std::unexpected(nandBytes.error());
    auto units = resolveUnitCount(startBlock, blockCount, *nandBytes);
    if (!units)
      return std::unexpected(units.error());

    ProgressPoller poller(progressCb, currentSpiUnit, startBlock, *units,
                          kUnitBytes, 0.0f, 100.0f, "Reading NAND...");
    rc = spi(kSpiModeRead, 0, const_cast<char *>(pathStr.c_str()),
             static_cast<int>(startBlock), static_cast<int>(*units));
  }

  if (rc != 0)
    return std::unexpected(interpretFtdiError(rc));

  report(progressCb, 100, 100, 100.0f, "FTDI Read complete.");
  return {};
}

std::expected<void, std::string>
Ftdi2SpiAdapter::writeNand(const std::filesystem::path &inputPath,
                           uint32_t startBlock, bool eraseFirst,
                           bool verifyAfter, const FlashDeviceConfig &config,
                           FlashProgressCallback progressCb) {
  // The library erases every block immediately before programming it, so
  // eraseFirst cannot be turned off.
  (void)eraseFirst;

  std::error_code ec;
  if (!std::filesystem::exists(inputPath, ec)) {
    return std::unexpected("Input file does not exist: " + inputPath.string());
  }
  const uint64_t fileSize = std::filesystem::file_size(inputPath, ec);
  if (ec)
    return std::unexpected("Unable to read input file size: " +
                           inputPath.string());

  const bool emmc = config.media == FlashMediaType::Emmc;
  if (emmc ? (fileSize == 0 || fileSize % kEmmcBlockBytes != 0)
           : (fileSize == 0 || fileSize % kRawPageBytes != 0)) {
    return std::unexpected(
        emmc ? "eMMC image size must be a multiple of 512 bytes"
             : "NAND image size must be a multiple of 0x210 bytes (raw dump "
               "with spare data)");
  }

  std::lock_guard<std::mutex> lock(g_ftdiMutex);
  const std::string pathStr = inputPath.string();
  const float writeSpan = verifyAfter ? 50.0f : 100.0f;

  report(progressCb, 0, 0, 0.0f, "Initializing FTDI write...");

  int rc = 0;
  uint64_t verifyBytes = 0;
  uint64_t verifyUnits = 0;
  if (emmc) {
    const uint64_t blocks = fileSize / kEmmcBlockBytes;
    ProgressPoller poller(progressCb, currentEmmcBlock, startBlock, blocks,
                          kEmmcBlockBytes, 0.0f, writeSpan, "Writing eMMC...");
    rc = emmc_write(pathStr.c_str(), static_cast<int>(startBlock));
    verifyBytes = fileSize;
    verifyUnits = blocks;
  } else {
    // The library mis-computes the end block for a non-zero start block and
    // would silently truncate the image, so only whole-image writes from the
    // start of the NAND are accepted.
    if (startBlock != 0)
      return std::unexpected(
          "Writing NAND from a non-zero start block is not supported by "
          "FTDI2SPI");

    auto nandBytes = detectNandBytes();
    if (!nandBytes)
      return std::unexpected(nandBytes.error());

    const uint64_t pages = fileSize / kRawPageBytes;
    if (pages * 0x200 > *nandBytes)
      return std::unexpected("Image is larger than the detected NAND (" +
                             std::to_string(*nandBytes / (1024 * 1024)) +
                             " MB)");
    const uint64_t units = (pages + kPagesPerUnit - 1) / kPagesPerUnit;

    ProgressPoller poller(progressCb, currentSpiUnit, 0, units, kUnitBytes,
                          0.0f, writeSpan, "Writing NAND...");
    rc = spi(kSpiModeWrite, 0, const_cast<char *>(pathStr.c_str()), 0,
             static_cast<int>(units));
    verifyBytes = fileSize;
    verifyUnits = units;
  }

  if (rc != 0)
    return std::unexpected(interpretFtdiError(rc));

  if (verifyAfter) {
    TempFile tmp("verify");
    const std::string tmpStr = tmp.path.string();
    {
      report(progressCb, 0, 0, 50.0f, "Verifying...");
      ProgressPoller poller(progressCb, emmc ? currentEmmcBlock : currentSpiUnit,
                            startBlock, verifyUnits,
                            emmc ? kEmmcBlockBytes : kUnitBytes, 50.0f, 50.0f,
                            "Verifying...");
      rc = emmc ? emmc_read(tmpStr.c_str(), static_cast<int>(startBlock),
                            static_cast<int>(verifyUnits))
                : spi(kSpiModeRead, 0, const_cast<char *>(tmpStr.c_str()), 0,
                      static_cast<int>(verifyUnits));
    }
    if (rc != 0)
      return std::unexpected("Verify readback failed: " +
                             interpretFtdiError(rc));
    if (auto cmp = compareFiles(inputPath, tmp.path, verifyBytes); !cmp)
      return cmp;
  }

  report(progressCb, 100, 100, 100.0f, "FTDI Write complete.");
  return {};
}

std::expected<void, std::string>
Ftdi2SpiAdapter::eraseNand(uint32_t startBlock, uint32_t blockCount,
                           const FlashDeviceConfig &config) {
  if (config.media == FlashMediaType::Emmc)
    return std::unexpected("Erase is not supported for eMMC");

  std::lock_guard<std::mutex> lock(g_ftdiMutex);

  auto nandBytes = detectNandBytes();
  if (!nandBytes)
    return std::unexpected(nandBytes.error());
  auto units = resolveUnitCount(startBlock, blockCount, *nandBytes);
  if (!units)
    return std::unexpected(units.error());

  const int rc = spi(kSpiModeErase, 0, nullptr, static_cast<int>(startBlock),
                     static_cast<int>(*units));
  if (rc != 0)
    return std::unexpected(interpretFtdiError(rc));
  return {};
}

} // namespace gxapi::backend
