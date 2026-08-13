#pragma once

#include "backend/IJtagService.hpp"

namespace gxapi::backend {

class XsvfToolAdapter : public IJtagService {
public:
  XsvfToolAdapter() = default;
  ~XsvfToolAdapter() override = default;

  [[nodiscard]] std::string serviceName() const override { return "xsvftool"; }

  std::expected<std::vector<uint32_t>, std::string>
  scanChain(const JtagDeviceConfig &config = {}) override;

  std::expected<void, std::string>
  flashCpld(const std::filesystem::path &bitstreamPath,
            const JtagDeviceConfig &config = {},
            JtagProgressCallback progressCb = nullptr) override;
};

} // namespace gxapi::backend
