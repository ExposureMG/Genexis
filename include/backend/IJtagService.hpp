#pragma once

#include <cstdint>
#include <expected>
#include <filesystem>
#include <functional>
#include <string>
#include <vector>

namespace gxapi::backend {

struct JtagDeviceConfig {
  std::string backend; // e.g. "FTDI", "DirtyJTAG"
  uint32_t clockFrequencyHz{0};
};

struct JtagProgressInfo {
  uint64_t bytesDone{0};
  uint64_t totalBytes{0};
  float percentage{0.0f};
  std::string statusMessage;
};

using JtagProgressCallback = std::function<void(const JtagProgressInfo &)>;

class IJtagService {
public:
  virtual ~IJtagService() = default;

  [[nodiscard]] virtual std::string serviceName() const = 0;

  virtual std::expected<std::vector<uint32_t>, std::string>
  scanChain(const JtagDeviceConfig &config = {}) = 0;

  virtual std::expected<void, std::string>
  flashCpld(const std::filesystem::path &bitstreamPath,
            const JtagDeviceConfig &config = {},
            JtagProgressCallback progressCb = nullptr) = 0;
};

} // namespace gxapi::backend
