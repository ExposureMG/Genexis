#include "plugins/PluginRegistry.hpp"
#include "plugins/endpoints/BuiltinEndpoint.hpp"
#include "plugins/endpoints/DynamicLibraryEndpoint.hpp"
#include "plugins/endpoints/ProcessEndpoint.hpp"

#include <algorithm>
#include <iostream>

namespace gxapi::Plugins {

PluginRegistry& PluginRegistry::instance() {
  static PluginRegistry reg;
  return reg;
}

void PluginRegistry::registerPlugin(PluginManifest manifest, std::shared_ptr<IPluginEndpoint> endpoint) {
  std::lock_guard<std::mutex> lock(m_mutex);

  // Check if plugin with same name exists and replace it
  auto it = std::find_if(m_plugins.begin(), m_plugins.end(),
                         [&manifest](const RegisteredPlugin& item) {
                           return item.manifest.name == manifest.name;
                         });

  if (it != m_plugins.end()) {
    it->manifest = manifest;
    it->endpoint = endpoint;
  } else {
    m_plugins.push_back(RegisteredPlugin{
        .manifest = std::move(manifest),
        .endpoint = std::move(endpoint)});
  }
}

size_t PluginRegistry::scanDirectory(const std::filesystem::path& pluginsDir) {
  std::error_code ec;
  if (!std::filesystem::exists(pluginsDir, ec) || !std::filesystem::is_directory(pluginsDir, ec)) {
    return 0;
  }

  size_t loadedCount = 0;
  for (const auto& entry : std::filesystem::directory_iterator(pluginsDir, ec)) {
    if (!entry.is_directory(ec)) continue;

    std::filesystem::path manifestPath = entry.path() / "plugin.json";
    if (!std::filesystem::exists(manifestPath, ec)) continue;

    auto manifestRes = parseManifest(manifestPath);
    if (!manifestRes) {
      continue;
    }

    PluginManifest manifest = *manifestRes;

    // Check platform compatibility
    if (!isPlatformSupported(manifest.platform)) {
      continue;
    }

    std::shared_ptr<IPluginEndpoint> endpoint;
    if (manifest.endpointType == EndpointType::Executable) {
      endpoint = std::make_shared<ProcessEndpoint>(manifest);
    } else if (manifest.endpointType == EndpointType::Library) {
      endpoint = std::make_shared<DynamicLibraryEndpoint>(manifest);
    } else {
      endpoint = std::make_shared<BuiltinEndpoint>(manifest);
    }

    if (endpoint->initialize()) {
      registerPlugin(manifest, endpoint);
      loadedCount++;
    }
  }

  return loadedCount;
}

std::shared_ptr<RegisteredPlugin> PluginRegistry::findPlugin(const std::string& name) const {
  std::lock_guard<std::mutex> lock(m_mutex);
  for (const auto& item : m_plugins) {
    if (item.manifest.name == name) {
      return std::make_shared<RegisteredPlugin>(item);
    }
  }
  return nullptr;
}

std::vector<RegisteredPlugin> PluginRegistry::getAvailablePlugins() const {
  std::lock_guard<std::mutex> lock(m_mutex);
  std::vector<RegisteredPlugin> result;
  for (const auto& item : m_plugins) {
    if (isPlatformSupported(item.manifest.platform)) {
      result.push_back(item);
    }
  }
  return result;
}

std::vector<RegisteredPlugin> PluginRegistry::getPluginsByType(PluginType type) const {
  std::lock_guard<std::mutex> lock(m_mutex);
  std::vector<RegisteredPlugin> result;
  for (const auto& item : m_plugins) {
    if (item.manifest.type == type && isPlatformSupported(item.manifest.platform)) {
      result.push_back(item);
    }
  }
  return result;
}

void PluginRegistry::clear() {
  std::lock_guard<std::mutex> lock(m_mutex);
  for (auto& item : m_plugins) {
    if (item.endpoint) {
      item.endpoint->shutdown();
    }
  }
  m_plugins.clear();
}

} // namespace gxapi::Plugins
