#pragma once

#include "plugins/IPluginEndpoint.hpp"
#include "plugins/PluginManifest.hpp"

#include <filesystem>
#include <memory>
#include <mutex>
#include <string>

namespace gxapi::Plugins {

struct RegisteredPlugin {
  PluginManifest manifest;
  std::shared_ptr<IPluginEndpoint> endpoint;
};

class PluginRegistry {
public:
  static PluginRegistry& instance();

  /// Register a plugin programmatically
  void registerPlugin(PluginManifest manifest, std::shared_ptr<IPluginEndpoint> endpoint);

  /// Scan a directory containing plugin subfolders with plugin.json manifests
  size_t scanDirectory(const std::filesystem::path& pluginsDir);

  /// Retrieve a registered plugin by name
  std::shared_ptr<RegisteredPlugin> findPlugin(const std::string& name) const;

  /// Retrieve all registered plugins that are supported on the current platform
  std::vector<RegisteredPlugin> getAvailablePlugins() const;

  /// Retrieve plugins filtered by main type (Io, Format, Builder)
  std::vector<RegisteredPlugin> getPluginsByType(PluginType type) const;

  /// Clear all registered plugins
  void clear();

private:
  PluginRegistry() = default;
  ~PluginRegistry() = default;
  PluginRegistry(const PluginRegistry&) = delete;
  PluginRegistry& operator=(const PluginRegistry&) = delete;

  mutable std::mutex m_mutex;
  std::vector<RegisteredPlugin> m_plugins;
};

} // namespace gxapi::Plugins
