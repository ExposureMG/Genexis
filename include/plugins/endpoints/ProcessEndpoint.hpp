#pragma once

#include "plugins/IPluginEndpoint.hpp"
#include "plugins/PluginManifest.hpp"

#include <filesystem>
#include <string>

namespace gxapi::Plugins {

class ProcessEndpoint : public IPluginEndpoint {
public:
  explicit ProcessEndpoint(PluginManifest manifest);
  ~ProcessEndpoint() override;

  bool initialize() override;
  void shutdown() override;
  PluginResult execute(const PluginCommand& cmd, PluginProgressCallback cb = nullptr) override;

private:
  PluginManifest m_manifest;
  std::filesystem::path m_binaryPath;
  std::filesystem::path m_workingDir;
  bool m_initialized{false};
};

} // namespace gxapi::Plugins
