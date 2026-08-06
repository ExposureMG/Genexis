#include "plugins/PluginManifest.hpp"

#include <algorithm>
#include <fstream>

namespace gxapi::Plugins {

namespace {

static std::string ToLower(std::string_view sv) {
  std::string result(sv);
  std::transform(result.begin(), result.end(), result.begin(), ::tolower);
  return result;
}

} // namespace

bool isPlatformSupported(std::string_view platform) noexcept {
  std::string p = ToLower(platform);
  if (p == "cross-platform" || p == "all" || p.empty()) {
    return true;
  }
#if defined(_WIN32) || defined(WIN32)
  return (p == "windows" || p == "win32" || p == "win");
#elif defined(__APPLE__) || defined(__MACH__)
  return (p == "macos" || p == "darwin" || p == "apple");
#elif defined(__linux__)
  return (p == "linux" || p == "unix");
#else
  return false;
#endif
}

std::expected<PluginManifest, std::string> parseManifest(const std::filesystem::path& manifestPath) {
  if (!std::filesystem::exists(manifestPath)) {
    return std::unexpected("Manifest file does not exist: " + manifestPath.string());
  }

  std::ifstream f(manifestPath);
  if (!f.is_open()) {
    return std::unexpected("Failed to open manifest file: " + manifestPath.string());
  }

  nlohmann::json j;
  try {
    f >> j;
  } catch (const std::exception& e) {
    return std::unexpected("JSON parse error in " + manifestPath.string() + ": " + e.what());
  }

  PluginManifest manifest;
  manifest.manifestPath = manifestPath;

  if (!j.contains("name") || !j["name"].is_string()) {
    return std::unexpected("Missing or invalid 'name' field in " + manifestPath.string());
  }
  manifest.name = j["name"].get<std::string>();

  if (j.contains("version") && j["version"].is_string()) {
    manifest.version = j["version"].get<std::string>();
  }

  if (j.contains("platform") && j["platform"].is_string()) {
    manifest.platform = j["platform"].get<std::string>();
  }

  // Parse endpointType
  if (j.contains("endpointType") && j["endpointType"].is_string()) {
    std::string epType = ToLower(j["endpointType"].get<std::string>());
    if (epType == "executable" || epType == "process" || epType == "exe") {
      manifest.endpointType = EndpointType::Executable;
    } else if (epType == "library" || epType == "shared" || epType == "dll" || epType == "so") {
      manifest.endpointType = EndpointType::Library;
    } else {
      manifest.endpointType = EndpointType::Builtin;
    }
  }

  // Parse main plugin type
  if (j.contains("type") && j["type"].is_string()) {
    std::string t = ToLower(j["type"].get<std::string>());
    if (t == "io" || t == "flasher" || t == "adapter") {
      manifest.type = PluginType::Io;
    } else if (t == "format" || t == "filesystem") {
      manifest.type = PluginType::Format;
    } else if (t == "builder" || t == "metadata" || t == "nand") {
      manifest.type = PluginType::Builder;
    }
  }

  // SubTypes
  if (j.contains("subTypes") && j["subTypes"].is_array()) {
    for (const auto& item : j["subTypes"]) {
      if (item.is_string()) {
        manifest.subTypes.push_back(item.get<std::string>());
      }
    }
  }

  // Provides capability object
  if (j.contains("provides")) {
    manifest.provides = j["provides"];
  }

  // Executable info
  if (j.contains("executable") && j["executable"].is_object()) {
    const auto& execJ = j["executable"];
    if (execJ.contains("binary") && execJ["binary"].is_string()) {
      manifest.executable.binary = execJ["binary"].get<std::string>();
    }
    if (execJ.contains("workingDir") && execJ["workingDir"].is_string()) {
      manifest.executable.workingDir = execJ["workingDir"].get<std::string>();
    }
  }

  // Library info
  if (j.contains("library") && j["library"].is_object()) {
    const auto& libJ = j["library"];
    if (libJ.contains("windows") && libJ["windows"].is_string()) {
      manifest.library.windows = libJ["windows"].get<std::string>();
    }
    if (libJ.contains("linux") && libJ["linux"].is_string()) {
      manifest.library.linuxLib = libJ["linux"].get<std::string>();
    }
    if (libJ.contains("macos") && libJ["macos"].is_string()) {
      manifest.library.macos = libJ["macos"].get<std::string>();
    }
  }

  if (j.contains("metaFormat") && j["metaFormat"].is_string()) {
    manifest.metaFormat = j["metaFormat"].get<std::string>();
  }

  // PostCommands array
  if (j.contains("postCommands") && j["postCommands"].is_array()) {
    for (const auto& item : j["postCommands"]) {
      if (!item.is_object()) continue;
      PostCommand pcmd;
      if (item.contains("name") && item["name"].is_string()) {
        pcmd.name = item["name"].get<std::string>();
      }
      if (item.contains("binary") && item["binary"].is_string()) {
        pcmd.binary = item["binary"].get<std::string>();
      }
      if (item.contains("continueOnError") && item["continueOnError"].is_boolean()) {
        pcmd.continueOnError = item["continueOnError"].get<bool>();
      }
      if (item.contains("args") && item["args"].is_array()) {
        for (const auto& argItem : item["args"]) {
          if (argItem.is_string()) {
            pcmd.args.push_back(argItem.get<std::string>());
          }
        }
      }
      manifest.postCommands.push_back(pcmd);
    }
  }

  return manifest;
}

} // namespace gxapi::Plugins
