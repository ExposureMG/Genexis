#include "api/Flasher.hpp"
#include "plugins/PluginRegistry.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <memory>

namespace gxapi::Flasher {

// ---------------------------------------------------------------------------
// Hardware Probing & Info
// ---------------------------------------------------------------------------

std::vector<AdapterType> getAvailableAdapters() {
  std::vector<AdapterType> adapters;

  auto availablePlugins = gxapi::Plugins::PluginRegistry::instance().getPluginsByType(gxapi::Plugins::PluginType::Io);
  for (const auto& plug : availablePlugins) {
    if (plug.manifest.name == "nandpromax") {
      adapters.push_back(AdapterType::NandProMax);
    } else if (plug.manifest.name == "ftdi2spi") {
      adapters.push_back(AdapterType::FtdiNative);
    } else if (plug.manifest.name == "xsvftool") {
      adapters.push_back(AdapterType::XsvfTool);
    }
  }

  if (adapters.empty()) {
    adapters.push_back(AdapterType::None);
  }
  return adapters;
}

std::expected<HardwareFlasher, std::string>
detectHardware(const DeviceConfig &config) {
  auto plug = gxapi::Plugins::PluginRegistry::instance().findPlugin("nandpromax");
  if (!plug || !plug->endpoint) {
    return std::unexpected("IO plugin (nandpromax) unavailable for hardware detection.");
  }

  gxapi::Plugins::PluginCommand cmd{
      .action = "detect_hardware",
      .parameters = {
          {"serialNumber", config.serialNumber},
          {"ipAddress", config.ipAddress},
          {"ftdiDescription", config.ftdiDescription},
          {"ftdiIndex", config.ftdiIndex},
          {"clockFrequencyHz", config.clockFrequencyHz},
          {"timeoutMs", config.timeoutMs}}};

  auto res = plug->endpoint->execute(cmd);
  if (!res.success) {
    return std::unexpected(res.errorMessage.empty() ? "Hardware detection failed." : res.errorMessage);
  }

  return HardwareFlasher::PicoFlasher;
}

std::expected<FlashInfo, std::string>
getFlashInfo(const DeviceConfig &config) {
  auto plug = gxapi::Plugins::PluginRegistry::instance().findPlugin("nandpromax");
  if (!plug || !plug->endpoint) {
    return std::unexpected("IO plugin (nandpromax) unavailable for flash info.");
  }

  gxapi::Plugins::PluginCommand cmd{
      .action = "get_flash_info",
      .parameters = {
          {"serialNumber", config.serialNumber},
          {"ipAddress", config.ipAddress}}};

  auto res = plug->endpoint->execute(cmd);
  if (!res.success) {
    return std::unexpected(res.errorMessage.empty() ? "Get flash info failed." : res.errorMessage);
  }

  FlashInfo info;
  info.type = FlashType::OldSfcSb;
  info.hardwareName = "Xbox 360 SFC NAND / eMMC Controller";
  return info;
}

// ---------------------------------------------------------------------------
// High-Level NAND & eMMC Operations (NandProMax + FTDI2SPI)
// ---------------------------------------------------------------------------

std::expected<void, std::string>
readNandToFile(const std::filesystem::path &outputPath, uint32_t startBlock,
               uint32_t blockCount, const DeviceConfig &config,
               ProgressCallback progressCb) {
  auto plug = gxapi::Plugins::PluginRegistry::instance().findPlugin("nandpromax");
  if (!plug) {
    plug = gxapi::Plugins::PluginRegistry::instance().findPlugin("ftdi2spi");
  }

  if (!plug || !plug->endpoint) {
    return std::unexpected("No compatible IO plugin found for read NAND operation.");
  }

  gxapi::Plugins::PluginCommand cmd{
      .action = "read_nand",
      .parameters = {
          {"output_path", outputPath.string()},
          {"start_block", startBlock},
          {"block_count", blockCount},
          {"serial_number", config.serialNumber},
          {"ip_address", config.ipAddress},
          {"clock_hz", config.clockFrequencyHz}}};

  gxapi::Plugins::PluginProgressCallback cbWrapper = nullptr;
  if (progressCb) {
    cbWrapper = [progressCb](uint64_t done, uint64_t total, const std::string &status) {
      ProgressInfo info;
      info.bytesDone = done;
      info.totalBytes = total;
      info.percentage = (total > 0) ? (static_cast<float>(done) / total * 100.0f) : 0.0f;
      info.statusMessage = status;
      progressCb(info);
    };
  }

  auto res = plug->endpoint->execute(cmd, cbWrapper);
  if (!res.success) {
    return std::unexpected(res.errorMessage.empty() ? "Read NAND operation failed." : res.errorMessage);
  }

  return {};
}

std::expected<void, std::string>
writeNandFromFile(const std::filesystem::path &inputPath, uint32_t startBlock,
                  bool eraseFirst, bool verifyAfter, const DeviceConfig &config,
                  ProgressCallback progressCb) {
  if (!std::filesystem::exists(inputPath)) {
    return std::unexpected("Input file does not exist: " + inputPath.string());
  }

  auto plug = gxapi::Plugins::PluginRegistry::instance().findPlugin("nandpromax");
  if (!plug) {
    plug = gxapi::Plugins::PluginRegistry::instance().findPlugin("ftdi2spi");
  }

  if (!plug || !plug->endpoint) {
    return std::unexpected("No compatible IO plugin found for write NAND operation.");
  }

  gxapi::Plugins::PluginCommand cmd{
      .action = "write_nand",
      .parameters = {
          {"input_path", inputPath.string()},
          {"start_block", startBlock},
          {"erase_first", eraseFirst},
          {"verify_after", verifyAfter},
          {"serial_number", config.serialNumber},
          {"ip_address", config.ipAddress},
          {"clock_hz", config.clockFrequencyHz}}};

  gxapi::Plugins::PluginProgressCallback cbWrapper = nullptr;
  if (progressCb) {
    cbWrapper = [progressCb](uint64_t done, uint64_t total, const std::string &status) {
      ProgressInfo info;
      info.bytesDone = done;
      info.totalBytes = total;
      info.percentage = (total > 0) ? (static_cast<float>(done) / total * 100.0f) : 0.0f;
      info.statusMessage = status;
      progressCb(info);
    };
  }

  auto res = plug->endpoint->execute(cmd, cbWrapper);
  if (!res.success) {
    return std::unexpected(res.errorMessage.empty() ? "Write NAND operation failed." : res.errorMessage);
  }

  return {};
}

std::expected<void, std::string>
eraseNand(uint32_t startBlock, uint32_t blockCount,
          const DeviceConfig &config) {
  auto plug = gxapi::Plugins::PluginRegistry::instance().findPlugin("nandpromax");
  if (!plug || !plug->endpoint) {
    return std::unexpected("IO plugin (nandpromax) unavailable for erase.");
  }

  gxapi::Plugins::PluginCommand cmd{
      .action = "erase_nand",
      .parameters = {
          {"start_block", startBlock},
          {"block_count", blockCount},
          {"serial_number", config.serialNumber},
          {"ip_address", config.ipAddress}}};

  auto res = plug->endpoint->execute(cmd);
  if (!res.success) {
    return std::unexpected(res.errorMessage.empty() ? "Erase NAND operation failed." : res.errorMessage);
  }

  return {};
}

// ---------------------------------------------------------------------------
// JTAG & CPLD Operations (XSVFTool + NandProMax)
// ---------------------------------------------------------------------------

std::expected<std::vector<uint32_t>, std::string>
scanJtagChain(const DeviceConfig &config) {
  auto plug = gxapi::Plugins::PluginRegistry::instance().findPlugin("xsvftool");
  if (!plug) {
    plug = gxapi::Plugins::PluginRegistry::instance().findPlugin("nandpromax");
  }

  if (!plug || !plug->endpoint) {
    return std::unexpected("JTAG chain scan plugin unavailable.");
  }

  gxapi::Plugins::PluginCommand cmd{
      .action = "scan_jtag_chain",
      .parameters = {{"clock_hz", config.clockFrequencyHz}}};

  auto res = plug->endpoint->execute(cmd);
  if (!res.success) {
    return std::unexpected(res.errorMessage.empty() ? "JTAG chain scan failed." : res.errorMessage);
  }

  return std::vector<uint32_t>{};
}

std::expected<void, std::string>
flashCpld(const std::filesystem::path &bitstreamPath,
          const DeviceConfig &config, ProgressCallback progressCb) {
  if (!std::filesystem::exists(bitstreamPath)) {
    return std::unexpected("Bitstream file does not exist: " + bitstreamPath.string());
  }

  auto plug = gxapi::Plugins::PluginRegistry::instance().findPlugin("xsvftool");
  if (!plug) {
    plug = gxapi::Plugins::PluginRegistry::instance().findPlugin("nandpromax");
  }

  if (!plug || !plug->endpoint) {
    return std::unexpected("CPLD flasher plugin unavailable.");
  }

  gxapi::Plugins::PluginCommand cmd{
      .action = "flash_cpld",
      .parameters = {
          {"bitstream_path", bitstreamPath.string()},
          {"clock_hz", config.clockFrequencyHz}}};

  auto res = plug->endpoint->execute(cmd);
  if (!res.success) {
    return std::unexpected(res.errorMessage.empty() ? "Flash CPLD operation failed." : res.errorMessage);
  }

  return {};
}

// ---------------------------------------------------------------------------
// ISD2100 Sound Chip Operations (FTDI2SPI)
// ---------------------------------------------------------------------------

std::expected<void, std::string>
readIsdAudio(const std::filesystem::path &outputPath,
             const DeviceConfig &config) {
  auto plug = gxapi::Plugins::PluginRegistry::instance().findPlugin("ftdi2spi");
  if (!plug || !plug->endpoint) {
    return std::unexpected("ISD2100 sound operations require FTDI2SPI plugin.");
  }

  gxapi::Plugins::PluginCommand cmd{
      .action = "read_isd",
      .parameters = {{"output_path", outputPath.string()}}};

  auto res = plug->endpoint->execute(cmd);
  if (!res.success) {
    return std::unexpected(res.errorMessage.empty() ? "Read ISD audio failed." : res.errorMessage);
  }

  return {};
}

std::expected<void, std::string>
writeIsdAudio(const std::filesystem::path &inputPath, bool verify,
              const DeviceConfig &config) {
  auto plug = gxapi::Plugins::PluginRegistry::instance().findPlugin("ftdi2spi");
  if (!plug || !plug->endpoint) {
    return std::unexpected("ISD2100 sound operations require FTDI2SPI plugin.");
  }

  gxapi::Plugins::PluginCommand cmd{
      .action = "write_isd",
      .parameters = {{"input_path", inputPath.string()}, {"verify", verify}}};

  auto res = plug->endpoint->execute(cmd);
  if (!res.success) {
    return std::unexpected(res.errorMessage.empty() ? "Write ISD audio failed." : res.errorMessage);
  }

  return {};
}

std::expected<void, std::string> playIsdSound(int soundIndex,
                                              const DeviceConfig &config) {
  auto plug = gxapi::Plugins::PluginRegistry::instance().findPlugin("ftdi2spi");
  if (!plug || !plug->endpoint) {
    return std::unexpected("ISD2100 sound operations require FTDI2SPI plugin.");
  }

  gxapi::Plugins::PluginCommand cmd{
      .action = "play_isd",
      .parameters = {{"sound_index", soundIndex}}};

  auto res = plug->endpoint->execute(cmd);
  if (!res.success) {
    return std::unexpected(res.errorMessage.empty() ? "Play ISD sound failed." : res.errorMessage);
  }

  return {};
}

std::expected<void, std::string> wipeIsdAudio(const DeviceConfig &config) {
  auto plug = gxapi::Plugins::PluginRegistry::instance().findPlugin("ftdi2spi");
  if (!plug || !plug->endpoint) {
    return std::unexpected("ISD2100 sound operations require FTDI2SPI plugin.");
  }

  gxapi::Plugins::PluginCommand cmd{.action = "wipe_isd", .parameters = {}};

  auto res = plug->endpoint->execute(cmd);
  if (!res.success) {
    return std::unexpected(res.errorMessage.empty() ? "Wipe ISD audio failed." : res.errorMessage);
  }

  return {};
}

} // namespace gxapi::Flasher
