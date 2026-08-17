#pragma once

#include "backend/IFlashService.hpp"

namespace gxapi::backend {

class Ftdi2SpiAdapter : public IFlashService {
public:
  Ftdi2SpiAdapter() = default;
  ~Ftdi2SpiAdapter() override = default;

  [[nodiscard]] std::string serviceName() const override { return "FTDI2SPI"; }

  std::expected<FlashInfo, std::string>
  getFlashInfo(const FlashDeviceConfig &config = {}) override;

  std::expected<void, std::string>
  readNand(const std::filesystem::path &outputPath, uint32_t startBlock = 0,
           uint32_t blockCount = 0, const FlashDeviceConfig &config = {},
           FlashProgressCallback progressCb = nullptr) override;

  std::expected<void, std::string>
  writeNand(const std::filesystem::path &inputPath, uint32_t startBlock = 0,
            bool eraseFirst = true, bool verifyAfter = false,
            const FlashDeviceConfig &config = {},
            FlashProgressCallback progressCb = nullptr) override;

  std::expected<void, std::string>
  eraseNand(uint32_t startBlock, uint32_t blockCount,
            const FlashDeviceConfig &config = {}) override;
};

} 
