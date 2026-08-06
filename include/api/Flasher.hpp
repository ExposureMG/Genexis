#pragma once

#include <array>
#include <chrono>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <functional>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace gxapi::Flasher {

// ---------------------------------------------------------------------------
// Extended Modern Enumerations & Domain Models
// ---------------------------------------------------------------------------

enum class AdapterType {
  Auto,
  NandProMax, // Multi-hardware library (PicoFlasher, FTDI, LPC, JRP, DemoN)
  FtdiNative, // Direct FTDI2SPI driver
  XsvfTool,   // JTAG/CPLD programmer
  None
};

enum class HardwareFlasher {
  Auto,
  PicoFlasher,
  Ftdi,
  Lpc,          // NAND-X, Matrix, etc.
  JpProgrammer, // JR-Programmer
  DemoN,        // TX DemoN
  Esp,          // ESP-based flasher
  None
};

enum class FlashMedia {
  Auto,
  Spi,    // SPI NAND (16MB, 64MB, 256MB, 512MB)
  Emmc,   // eMMC (4GB Corona/Wafer)
  Isd2100,// ISD Sound Chip
  Xsvf    // JTAG / CPLD programming
};

enum class FlashType {
  OldSfcSb,    // XSB SFC Small Block (16mb)
  OldSfcDevSb, // XSB SFC Small Block Devkit (64mb)
  NewSfcSb,    // PSB SFC Small Block (16mb)
  NewSfcBb,    // PSB SFC Big Block (256/512mb)
  NewSfcBbDev, // PSB SFC Small Block Devkit (64mb)
  KsbSfcSb,    // KSB SFC Small Block (16mb)
  KsbSfcMmc,   // KSB SFC eMMC (4GB)
  Unknown
};

enum class FtdiPageFormat { Auto, Small, Big };

struct ProgressInfo {
  uint64_t bytesDone{0};
  uint64_t totalBytes{0};
  float percentage{0.0f};
  std::string statusMessage;
};

using ProgressCallback = std::function<void(const ProgressInfo&)>;

struct DeviceConfig {
  AdapterType adapter{AdapterType::Auto};
  HardwareFlasher hardware{HardwareFlasher::Auto};
  FlashMedia media{FlashMedia::Auto};

  std::string serialNumber;                  // USB Serial number
  std::string ipAddress{"192.168.4.1:3232"}; // Network target address if applicable
  std::string ftdiDescription{"auto"};       // FTDI Device string
  int32_t ftdiIndex{0};

  uint32_t clockFrequencyHz{6000000};
  FtdiPageFormat pageFormat{FtdiPageFormat::Auto};
  uint64_t timeoutMs{3000};
};

struct FlashInfo {
  FlashType type{FlashType::OldSfcSb};
  std::array<uint8_t, 8> configBytes{0};
  uint32_t totalBlocks{0};
  uint64_t totalBytes{0};
  std::string hardwareName;
};

// ---------------------------------------------------------------------------
// Hardware Probing & Info
// ---------------------------------------------------------------------------

std::vector<AdapterType> getAvailableAdapters();
std::expected<HardwareFlasher, std::string> detectHardware(const DeviceConfig& config = {});
std::expected<FlashInfo, std::string> getFlashInfo(const DeviceConfig& config = {});

// ---------------------------------------------------------------------------
// High-Level NAND & eMMC Operations (NandProMax + FTDI2SPI)
// ---------------------------------------------------------------------------

std::expected<void, std::string> readNandToFile(
    const std::filesystem::path& outputPath,
    uint32_t startBlock = 0,
    uint32_t blockCount = 0,
    const DeviceConfig& config = {},
    ProgressCallback progressCb = nullptr);

std::expected<void, std::string> writeNandFromFile(
    const std::filesystem::path& inputPath,
    uint32_t startBlock = 0,
    bool eraseFirst = true,
    bool verifyAfter = false,
    const DeviceConfig& config = {},
    ProgressCallback progressCb = nullptr);

std::expected<void, std::string> eraseNand(
    uint32_t startBlock,
    uint32_t blockCount,
    const DeviceConfig& config = {});

// ---------------------------------------------------------------------------
// JTAG & CPLD Operations (XSVFTool + NandProMax)
// ---------------------------------------------------------------------------

std::expected<std::vector<uint32_t>, std::string> scanJtagChain(
    const DeviceConfig& config = {});

std::expected<void, std::string> flashCpld(
    const std::filesystem::path& bitstreamPath,
    const DeviceConfig& config = {},
    ProgressCallback progressCb = nullptr);

// ---------------------------------------------------------------------------
// ISD2100 Sound Chip Operations (FTDI2SPI)
// ---------------------------------------------------------------------------

std::expected<void, std::string> readIsdAudio(
    const std::filesystem::path& outputPath,
    const DeviceConfig& config = {});

std::expected<void, std::string> writeIsdAudio(
    const std::filesystem::path& inputPath,
    bool verify = true,
    const DeviceConfig& config = {});

std::expected<void, std::string> playIsdSound(
    int soundIndex,
    const DeviceConfig& config = {});

std::expected<void, std::string> wipeIsdAudio(const DeviceConfig& config = {});

} // namespace gxapi::Flasher
