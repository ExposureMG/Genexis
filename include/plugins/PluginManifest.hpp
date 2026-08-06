#pragma once

#include <expected>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

#include <nlohmann/json.hpp>

namespace gxapi::Plugins {

enum class EndpointType {
  Executable,
  Library,
  Builtin
};

enum class PluginType {
  Io,
  Format,
  Builder,
  Unknown
};

struct ExecutableInfo {
  std::string binary;
  std::string workingDir;
};

struct LibraryInfo {
  std::string windows;
  std::string linuxLib;
  std::string macos;
};

struct PostCommand {
  std::string name;
  std::string binary;
  std::vector<std::string> args;
  bool continueOnError{false};
};

struct PluginManifest {
  std::string name;
  std::string version{"1.0"};
  std::string platform{"cross-platform"}; // "windows", "linux", "macos", "cross-platform"
  EndpointType endpointType{EndpointType::Builtin};
  PluginType type{PluginType::Unknown};
  std::string metaFormat;                 // "xebuild", "rgbuild", "build360", "gxbuild3"
  std::vector<std::string> subTypes;
  nlohmann::json provides;

  ExecutableInfo executable;
  LibraryInfo library;
  std::vector<PostCommand> postCommands;
  std::filesystem::path manifestPath;
};

// Check if a platform string matches the current compile-time operating system
bool isPlatformSupported(std::string_view platform) noexcept;

// Parse a plugin.json manifest file
std::expected<PluginManifest, std::string> parseManifest(const std::filesystem::path& manifestPath);

} // namespace gxapi::Plugins
