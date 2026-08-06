#include "plugins/endpoints/DynamicLibraryEndpoint.hpp"

#include <array>
#include <iostream>

#if defined(_WIN32) || defined(WIN32)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace gxapi::Plugins {

DynamicLibraryEndpoint::DynamicLibraryEndpoint(PluginManifest manifest)
    : m_manifest(std::move(manifest)) {
  std::filesystem::path manifestDir = m_manifest.manifestPath.parent_path();

#if defined(_WIN32) || defined(WIN32)
  m_libraryPath = manifestDir / m_manifest.library.windows;
#elif defined(__APPLE__) || defined(__MACH__)
  m_libraryPath = manifestDir / (m_manifest.library.macos.empty() ? m_manifest.library.linuxLib : m_manifest.library.macos);
#else
  m_libraryPath = manifestDir / m_manifest.library.linuxLib;
#endif
}

DynamicLibraryEndpoint::~DynamicLibraryEndpoint() {
  shutdown();
}

bool DynamicLibraryEndpoint::initialize() {
  if (m_handle) {
    return true;
  }
  if (!std::filesystem::exists(m_libraryPath)) {
    return false;
  }

#if defined(_WIN32) || defined(WIN32)
  HMODULE mod = LoadLibraryA(m_libraryPath.string().c_str());
  if (!mod) return false;
  m_handle = reinterpret_cast<void*>(mod);
  m_pExec = reinterpret_cast<FnGenexisPluginExec>(GetProcAddress(mod, "genexis_plugin_exec"));
#else
  void* handle = dlopen(m_libraryPath.c_str(), RTLD_LAZY);
  if (!handle) return false;
  m_handle = handle;
  m_pExec = reinterpret_cast<FnGenexisPluginExec>(dlsym(handle, "genexis_plugin_exec"));
#endif

  return (m_handle != nullptr && m_pExec != nullptr);
}

void DynamicLibraryEndpoint::shutdown() {
  if (m_handle) {
#if defined(_WIN32) || defined(WIN32)
    FreeLibrary(reinterpret_cast<HMODULE>(m_handle));
#else
    dlclose(m_handle);
#endif
    m_handle = nullptr;
    m_pExec = nullptr;
  }
}

PluginResult DynamicLibraryEndpoint::execute(const PluginCommand& cmd, PluginProgressCallback cb) {
  if (!m_handle && !initialize()) {
    return PluginResult{
        .success = false,
        .exitCode = -1,
        .errorMessage = "Failed to load shared library: " + m_libraryPath.string()};
  }

  nlohmann::json payload;
  payload["action"] = cmd.action;
  payload["parameters"] = cmd.parameters;
  std::string jsonStr = payload.dump();

  std::array<char, 8192> outBuf;
  outBuf.fill(0);

  int code = m_pExec(jsonStr.c_str(), outBuf.data(), outBuf.size(), reinterpret_cast<void*>(&cb));
  bool ok = (code == 0);

  nlohmann::json resJ;
  try {
    if (outBuf[0] != '\0') {
      resJ = nlohmann::json::parse(outBuf.data());
    }
  } catch (...) {}

  return PluginResult{
      .success = ok,
      .exitCode = code,
      .errorMessage = ok ? "" : "Library execution failed with code " + std::to_string(code),
      .responseData = resJ};
}

} // namespace gxapi::Plugins
