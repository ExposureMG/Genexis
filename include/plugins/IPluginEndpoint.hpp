#pragma once

#include <cstdint>
#include <expected>
#include <functional>
#include <string>

#include <nlohmann/json.hpp>

namespace gxapi::Plugins {

struct PluginCommand {
  std::string action;        // Action name e.g. "build_nand", "read_flash"
  nlohmann::json parameters; // Parameters payload
};

struct PluginResult {
  bool success{false};
  int exitCode{0};
  std::string errorMessage;
  nlohmann::json responseData;
};

using PluginProgressCallback = std::function<void(uint64_t done, uint64_t total, const std::string& status)>;

class IPluginEndpoint {
public:
  virtual ~IPluginEndpoint() = default;

  /// Initialize endpoint resource (open handles, libraries, or processes)
  virtual bool initialize() = 0;

  /// Shutdown endpoint resource
  virtual void shutdown() = 0;

  /// Execute command synchronously or asynchronously
  virtual PluginResult execute(const PluginCommand& cmd, PluginProgressCallback cb = nullptr) = 0;
};

} // namespace gxapi::Plugins
