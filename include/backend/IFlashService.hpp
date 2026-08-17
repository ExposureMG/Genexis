#pragma once

#include <cstdint>
#include <expected>
#include <filesystem>
#include <functional>
#include <string>
#include <vector>

namespace gxapi::backend {

enum class FlashHardwareType {
  Auto,
  PicoFlasher,
  XFlasher,
  NandX,
  Lpc,
  DemoN,
  UpdServer,
  None
};

enum class FlashMediaType { Auto, Spi, Emmc };

struct FlashDeviceConfig {
  FlashHardwareType hardware{FlashHardwareType::Auto};
  FlashMediaType media{FlashMediaType::Auto};
  std::string serialNumber;
  std::string ipAddress{"192.168.4.1:3232"};
  std::string ftdiDescription{"auto"};
  int32_t ftdiIndex{-1};
  uint32_t clockFrequencyHz{0};
  uint64_t timeoutMs{5000};
};

struct FlashProgressInfo {
  uint64_t bytesDone{0};
  uint64_t totalBytes{0};
  float percentage{0.0f};
  std::string statusMessage;
};

using FlashProgressCallback = std::function<void(const FlashProgressInfo &)>;

struct FlashInfo {
  std::string hardwareName;
  std::string flashType;
  uint32_t configWord{0};
  uint32_t totalBlocks{0};
  uint64_t totalBytes{0};
};

class IFlashService {
public:
  virtual ~IFlashService() = default;

  [[nodiscard]] virtual std::string serviceName() const = 0;

  virtual std::expected<FlashInfo, std::string>
  getFlashInfo(const FlashDeviceConfig &config = {}) = 0;

  virtual std::expected<void, std::string>
  readNand(const std::filesystem::path &outputPath, uint32_t startBlock = 0,
           uint32_t blockCount = 0, const FlashDeviceConfig &config = {},
           FlashProgressCallback progressCb = nullptr) = 0;

  virtual std::expected<void, std::string>
  writeNand(const std::filesystem::path &inputPath, uint32_t startBlock = 0,
            bool eraseFirst = true, bool verifyAfter = false,
            const FlashDeviceConfig &config = {},
            FlashProgressCallback progressCb = nullptr) = 0;

  virtual std::expected<void, std::string>
  eraseNand(uint32_t startBlock, uint32_t blockCount,
            const FlashDeviceConfig &config = {}) = 0;
};

} 
