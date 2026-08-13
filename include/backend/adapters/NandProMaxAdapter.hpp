#pragma once

#include "backend/IFlashService.hpp"
#include "backend/IJtagService.hpp"

namespace gxapi::backend {

class NandProMaxAdapter : public IFlashService, public IJtagService {
public:
  NandProMaxAdapter() = default;
  ~NandProMaxAdapter() override = default;

  [[nodiscard]] std::string serviceName() const override {
    return "NandProMax";
  }

  // IFlashService implementation
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

  // IJtagService implementation
  std::expected<std::vector<uint32_t>, std::string>
  scanChain(const JtagDeviceConfig &config = {}) override;

  std::expected<void, std::string>
  flashCpld(const std::filesystem::path &bitstreamPath,
            const JtagDeviceConfig &config = {},
            JtagProgressCallback progressCb = nullptr) override;
};

} // namespace gxapi::backend
