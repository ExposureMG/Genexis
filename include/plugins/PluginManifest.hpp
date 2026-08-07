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

/// A single named parameter within a capability.
struct CapabilityParam {
  std::string name;         // param name used in PluginCommand::parameters JSON
  std::string flagName;     // CLI flag name emitted (defaults to name if empty)
  std::string type;         // "string", "int", "bool", "path"
  std::string flagStyle;    // "gnu" (--key value), "dash" (-key value), "colon" (-key:value)
                            // defaults to "gnu" if empty
  bool required{false};
  std::string defaultValue;
  std::string description;
};

/// A discrete action a plugin can perform, with its CLI command template
/// and parameter schema.
struct Capability {
  std::string name;                    // e.g. "read_nand"
  std::vector<std::string> command;    // argv prefix e.g. ["nand", "read"]
  std::vector<std::string> positional; // ordered param names passed as positional args after command
  std::string outputParam;             // param name whose value is the output file path (empty = none)
  std::vector<CapabilityParam> params;
  std::string description;
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
  std::vector<Capability> capabilities;  // Typed capability definitions

  ExecutableInfo executable;
  LibraryInfo library;
  std::vector<PostCommand> postCommands;
  std::filesystem::path manifestPath;

  /// Find a capability by name, returns nullptr if not found.
  const Capability* findCapability(std::string_view capName) const noexcept;
};


// Check if a platform string matches the current compile-time operating system
bool isPlatformSupported(std::string_view platform) noexcept;

// Parse a plugin.json manifest file
std::expected<PluginManifest, std::string> parseManifest(const std::filesystem::path& manifestPath);

} // namespace gxapi::Plugins
