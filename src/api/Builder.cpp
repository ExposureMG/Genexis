#include "api/Builder.hpp"
#include "plugins/PluginRegistry.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>

namespace gxapi::Builder {

std::expected<BuildResult, std::string> buildNandImage(
    const gxapi::Metabuild::BuildConfig& config,
    const gxapi::Metabuild::UserSession& session,
    const std::filesystem::path& outputImagePath,
    ProgressCallback progressCb) {

  // 1. Resolve builder plugin from registry
  auto availableBuilders = gxapi::Plugins::PluginRegistry::instance().getPluginsByType(gxapi::Plugins::PluginType::Builder);
  if (availableBuilders.empty()) {
    return std::unexpected("No builder plugins registered in Genexis.");
  }

  // Find plugin by preference (e.g. xebuild, rgbuild, gxbuild3)
  std::shared_ptr<gxapi::Plugins::RegisteredPlugin> targetPlugin = nullptr;
  std::string preferredName = "xebuild";
  if (config.meta.imageType == gxapi::Metabuild::ImageType::RGLoader) {
    preferredName = "rgbuild";
  }

  targetPlugin = gxapi::Plugins::PluginRegistry::instance().findPlugin(preferredName);
  if (!targetPlugin || !targetPlugin->endpoint) {
    targetPlugin = std::make_shared<gxapi::Plugins::RegisteredPlugin>(availableBuilders.front());
  }

  if (!targetPlugin || !targetPlugin->endpoint) {
    return std::unexpected("Selected builder plugin endpoint is unavailable.");
  }

  // 2. Perform Metadata Translation if metaFormat specified
  std::string metaFormat = targetPlugin->manifest.metaFormat;
  std::filesystem::path tempDir = std::filesystem::temp_directory_path() / "genexis_build";
  std::error_code ec;
  std::filesystem::create_directories(tempDir, ec);

  std::filesystem::path sourceNandStr = session.sourceNandImage.value_or("");
  std::string cpuKeyStr;
  if (session.cpuKey) {
    for (uint8_t b : *session.cpuKey) {
      char buf[3];
      snprintf(buf, sizeof(buf), "%02X", b);
      cpuKeyStr += buf;
    }
  }

  if (metaFormat == "xebuild") {
    std::filesystem::path iniFile = tempDir / "options.ini";
    auto writeRes = gxapi::Metabuild::writeXeBuildIniFile(config, iniFile);
    if (!writeRes) {
      return std::unexpected("Failed to generate xeBuild metadata: " + writeRes.error());
    }
  } else if (metaFormat == "rgbuild") {
    auto writeRes = gxapi::Metabuild::writeRGBuildIniFiles(config, tempDir);
    if (!writeRes) {
      return std::unexpected("Failed to generate RGBuild metadata: " + writeRes.error());
    }
  }

  // 3. Assemble PluginCommand parameters
  gxapi::Plugins::PluginCommand cmd{
      .action = "build_image",
      .parameters = {
          {"consoleTarget", gxapi::Metabuild::ConsoleTargetToString(config.meta.consoleTarget)},
          {"buildType", config.meta.buildType},
          {"dashboardVersion", config.meta.version},
          {"cpuKey", cpuKeyStr},
          {"sourceNand", sourceNandStr.string()},
          {"outputImage", outputImagePath.string()},
          {"workingDir", tempDir.string()}}};

  gxapi::Plugins::PluginProgressCallback cbWrapper = nullptr;
  if (progressCb) {
    cbWrapper = [progressCb](uint64_t done, uint64_t total, const std::string& status) {
      progressCb(done, total, status);
    };
  }

  // 4. Execute Main Plugin Build Command
  auto res = targetPlugin->endpoint->execute(cmd, cbWrapper);

  BuildResult result;
  result.success = res.success;
  result.exitCode = res.exitCode;
  result.outputImagePath = outputImagePath;
  result.logOutput = res.errorMessage;

  if (!res.success) {
    return std::unexpected(res.errorMessage.empty() ? "NAND image build failed." : res.errorMessage);
  }

  return result;
}

} // namespace gxapi::Builder