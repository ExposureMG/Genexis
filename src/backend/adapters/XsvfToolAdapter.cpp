#include "backend/adapters/XsvfToolAdapter.hpp"
#include <xsvftool.h>

#include <mutex>

namespace gxapi::backend {

namespace {
static std::mutex g_xsvfMutex;
}

std::expected<std::vector<uint32_t>, std::string>
XsvfToolAdapter::scanChain(const JtagDeviceConfig &config) {
  std::lock_guard<std::mutex> lock(g_xsvfMutex);

  const char *backendName =
      config.backend.empty() ? nullptr : config.backend.c_str();
  int rc = xsvftool_scan_chain(backendName,
                               static_cast<int>(config.clockFrequencyHz));
  if (rc != 0) {
    return std::unexpected("xsvftool JTAG scan chain failed with exit code: " +
                           std::to_string(rc));
  }

  return std::vector<uint32_t>{};
}

std::expected<void, std::string>
XsvfToolAdapter::flashCpld(const std::filesystem::path &bitstreamPath,
                           const JtagDeviceConfig &config,
                           JtagProgressCallback progressCb) {
  if (!std::filesystem::exists(bitstreamPath)) {
    return std::unexpected("Bitstream file does not exist: " +
                           bitstreamPath.string());
  }

  std::lock_guard<std::mutex> lock(g_xsvfMutex);

  if (progressCb) {
    JtagProgressInfo pInfo{.bytesDone = 0,
                           .totalBytes = 0,
                           .percentage = 0.0f,
                           .statusMessage = "Starting xsvftool CPLD flash..."};
    progressCb(pInfo);
  }

  std::string pathStr = bitstreamPath.string();
  std::string ext = bitstreamPath.extension().string();
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

  const char *backendName =
      config.backend.empty() ? nullptr : config.backend.c_str();
  int freq = static_cast<int>(config.clockFrequencyHz);

  int rc = 0;
  if (ext == ".svf") {
    rc = xsvftool_play_svf(pathStr.c_str(), backendName, freq);
  } else {
    rc = xsvftool_play_xsvf(pathStr.c_str(), backendName, freq);
  }

  if (rc != 0) {
    return std::unexpected(
        "xsvftool CPLD timing flash failed with exit code: " +
        std::to_string(rc));
  }

  if (progressCb) {
    JtagProgressInfo pInfo{.bytesDone = 100,
                           .totalBytes = 100,
                           .percentage = 100.0f,
                           .statusMessage = "CPLD Timing Flash complete."};
    progressCb(pInfo);
  }

  return {};
}

} 
