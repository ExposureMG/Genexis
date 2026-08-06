#include "plugins/endpoints/ProcessEndpoint.hpp"

#include <array>
#include <cstdio>
#include <cstdlib>
#include <iostream>

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

  // Construct CLI arguments from command parameters
  std::string cmdLine = m_binaryPath.string();
  if (cmd.parameters.is_object()) {
    for (auto& [key, val] : cmd.parameters.items()) {
      cmdLine += " --" + key + " " + (val.is_string() ? val.get<std::string>() : val.dump());
    }
  }

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

  std::array<char, 256> buffer;
  std::string output;
  while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr) {
    std::string line(buffer.data());
    output += line;
    if (cb) {
      cb(0, 0, line);
    }
  }

#if defined(_WIN32) || defined(WIN32)
  int exitCode = _pclose(pipe);
#else
  int exitCode = pclose(pipe);
#endif

  bool ok = (exitCode == 0);
  return PluginResult{
      .success = ok,
      .exitCode = exitCode,
      .errorMessage = ok ? "" : "Process exited with code " + std::to_string(exitCode),
      .responseData = nlohmann::json{{"output", output}}};
}

} // namespace gxapi::Plugins
