#include "plugins/endpoints/BuiltinEndpoint.hpp"

namespace gxapi::Plugins {

BuiltinEndpoint::BuiltinEndpoint(PluginManifest manifest, BuiltinHandler handler)
    : m_manifest(std::move(manifest)), m_handler(std::move(handler)) {}

bool BuiltinEndpoint::initialize() {
  return true;
}

void BuiltinEndpoint::shutdown() {}

void BuiltinEndpoint::setHandler(BuiltinHandler handler) {
  m_handler = std::move(handler);
}

PluginResult BuiltinEndpoint::execute(const PluginCommand& cmd, PluginProgressCallback cb) {
  if (!m_handler) {
    return PluginResult{
        .success = false,
        .exitCode = -1,
        .errorMessage = "No handler registered for built-in plugin: " + m_manifest.name};
  }
  return m_handler(cmd, cb);
}

} // namespace gxapi::Plugins
