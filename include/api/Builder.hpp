#pragma once

#include "api/Metadata.hpp"
#include "plugins/IPluginEndpoint.hpp"

#include <cstdint>
#include <expected>
#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace gxapi::Builder {

using ProgressCallback = std::function<void(uint64_t done, uint64_t total, const std::string& status)>;

struct BuildResult {
  bool success{false};
  int exitCode{0};
  std::filesystem::path outputImagePath;
  std::string logOutput;
};

/// High-level API method to assemble a NAND or eMMC image using registered plugin engines
std::expected<BuildResult, std::string> buildNandImage(
    const gxapi::Metabuild::BuildConfig& config,
    const gxapi::Metabuild::UserSession& session,
    const std::filesystem::path& outputImagePath,
    ProgressCallback progressCb = nullptr);

} // namespace gxapi::Builder