#include "plugins/endpoints/ProcessEndpoint.hpp"

#include <array>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <set>

namespace gxapi::Plugins {

ProcessEndpoint::ProcessEndpoint(PluginManifest manifest)
    : m_manifest(std::move(manifest)) {
  std::filesystem::path manifestDir = m_manifest.manifestPath.parent_path();
  m_binaryPath = manifestDir / m_manifest.executable.binary;
  if (!m_manifest.executable.workingDir.empty()) {
    m_workingDir = manifestDir / m_manifest.executable.workingDir;
  } else {
    m_workingDir = manifestDir;
  }
}

ProcessEndpoint::~ProcessEndpoint() {
  shutdown();
}

bool ProcessEndpoint::initialize() {
  if (std::filesystem::exists(m_binaryPath)) {
    m_initialized = true;
    return true;
  }
  return false;
}

void ProcessEndpoint::shutdown() {
  m_initialized = false;
}

PluginResult ProcessEndpoint::execute(const PluginCommand& cmd, PluginProgressCallback cb) {
  if (!m_initialized && !initialize()) {
    return PluginResult{
        .success = false,
        .exitCode = -1,
        .errorMessage = "Process binary not found: " + m_binaryPath.string()};
  }

  const auto* cap = m_manifest.findCapability(cmd.action);

  // --- Build command line ---
  std::string cmdLine = m_binaryPath.string();

  // 1. Capability command prefix (positional subcommands, e.g. "nand read")
  if (cap && !cap->command.empty()) {
    for (const auto& tok : cap->command) {
      cmdLine += " " + tok;
    }
  }

  // 2. Declared positional params in order (e.g. the <i/o> file argument)
  //    These are emitted bare, not as --flag value.
  std::set<std::string> positionalKeys;
  if (cap && !cap->positional.empty() && cmd.parameters.is_object()) {
    for (const auto& paramName : cap->positional) {
      positionalKeys.insert(paramName);
      if (cmd.parameters.contains(paramName)) {
        const auto& val = cmd.parameters[paramName];
        if (!val.is_null()) {
          cmdLine += " " + (val.is_string() ? val.get<std::string>() : val.dump());
        }
      }
    }
  }

  // 3. Named --flag [value] params (skip positionals already emitted)
  if (cmd.parameters.is_object()) {
    for (auto& [key, val] : cmd.parameters.items()) {
      if (positionalKeys.count(key)) continue; // already emitted positionally
      if (val.is_null()) continue;             // null → omit entirely
      if (val.is_boolean() && !val.get<bool>()) continue; // false bool → omit

      // Look up this param's schema (if capability declared one) to get
      // flagName and flagStyle; fall back to GNU --key value for unknown params.
      const CapabilityParam* schema = nullptr;
      if (cap) {
        for (const auto& p : cap->params) {
          if (p.name == key) { schema = &p; break; }
        }
      }

      const std::string& flagName = (schema && !schema->flagName.empty()) ? schema->flagName : key;
      const std::string& style    = schema ? schema->flagStyle : "";

      std::string valStr;
      if (!val.is_boolean()) {
        valStr = val.is_string() ? val.get<std::string>() : val.dump();
      }

      if (style == "colon") {
        // -flag:value  (RGBuild style)
        cmdLine += " -" + flagName;
        if (!val.is_boolean()) cmdLine += ":" + valStr;
      } else if (style == "dash") {
        // -flag value  (xeBuild style)
        cmdLine += " -" + flagName;
        if (!val.is_boolean()) cmdLine += " " + valStr;
      } else {
        // --flag value  (GNU default)
        cmdLine += " --" + flagName;
        if (!val.is_boolean()) cmdLine += " " + valStr;
      }
    }
  }

  // Merge stderr into stdout so both streams are captured live through the
  // pipe. Every line — stdout and stderr alike — is delivered to the optional
  // callback in real time, and the full combined log is available after exit.
  cmdLine += " 2>&1";

#if defined(_WIN32) || defined(WIN32)
  FILE* pipe = _popen(cmdLine.c_str(), "r");
#else
  FILE* pipe = popen(cmdLine.c_str(), "r");
#endif

  if (!pipe) {
    return PluginResult{
        .success = false,
        .exitCode = -1,
        .errorMessage = "Failed to launch process: " + cmdLine};
  }

  // Stream combined stdout+stderr, delivering each chunk to the live callback
  std::array<char, 256> buffer;
  std::string combinedOutput;
  while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr) {
    std::string_view chunk(buffer.data());
    combinedOutput.append(chunk);
    if (cb) {
      cb(0, 0, std::string(chunk));
    }
  }

#if defined(_WIN32) || defined(WIN32)
  int exitCode = _pclose(pipe);
#else
  int exitCode = pclose(pipe);
#endif

  bool ok = (exitCode == 0);

  // On failure the combined output already contains stderr — use it as the
  // error message so callers get the actual diagnostic, not just an exit code.
  std::string errorMsg;
  if (!ok) {
    errorMsg = combinedOutput.empty()
        ? "Process exited with code " + std::to_string(exitCode)
        : combinedOutput;
  }

  // Reflect the output file path in responseData if the capability declared one
  nlohmann::json responseData = {{"output", combinedOutput}};
  if (cap && !cap->outputParam.empty() && cmd.parameters.is_object()
      && cmd.parameters.contains(cap->outputParam)) {
    responseData["outputPath"] = cmd.parameters[cap->outputParam];
  }

  return PluginResult{
      .success = ok,
      .exitCode = exitCode,
      .errorMessage = errorMsg,
      .stderrOutput = ok ? std::string{} : combinedOutput,
      .responseData = std::move(responseData)};
}

} // namespace gxapi::Plugins
