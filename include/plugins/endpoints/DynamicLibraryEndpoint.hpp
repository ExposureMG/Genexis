#pragma once

#include "plugins/IPluginEndpoint.hpp"
#include "plugins/PluginManifest.hpp"

#include <filesystem>
#include <string>

namespace gxapi::Plugins {

// C-ABI function signature exported by shared library plugins
using FnGenexisPluginExec = int (*)(const char* jsonCmd, char* outBuf, size_t outBufLen, void* progressCb);

class DynamicLibraryEndpoint : public IPluginEndpoint {
public:
  explicit DynamicLibraryEndpoint(PluginManifest manifest);
  ~DynamicLibraryEndpoint() override;

  bool initialize() override;
  void shutdown() override;
  PluginResult execute(const PluginCommand& cmd, PluginProgressCallback cb = nullptr) override;

private:
  PluginManifest m_manifest;
  std::filesystem::path m_libraryPath;
  void* m_handle{nullptr};
  FnGenexisPluginExec m_pExec{nullptr};
};

} // namespace gxapi::Plugins
