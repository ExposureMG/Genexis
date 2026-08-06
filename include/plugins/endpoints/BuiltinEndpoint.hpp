#pragma once

#include "plugins/IPluginEndpoint.hpp"
#include "plugins/PluginManifest.hpp"

#include <functional>
#include <string>

namespace gxapi::Plugins {

using BuiltinHandler = std::function<PluginResult(const PluginCommand& cmd, PluginProgressCallback cb)>;

class BuiltinEndpoint : public IPluginEndpoint {
public:
  explicit BuiltinEndpoint(PluginManifest manifest, BuiltinHandler handler = nullptr);
  ~BuiltinEndpoint() override = default;

  bool initialize() override;
  void shutdown() override;
  PluginResult execute(const PluginCommand& cmd, PluginProgressCallback cb = nullptr) override;

  void setHandler(BuiltinHandler handler);

private:
  PluginManifest m_manifest;
  BuiltinHandler m_handler;
};

} // namespace gxapi::Plugins
