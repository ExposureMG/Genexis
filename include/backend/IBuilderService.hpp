#pragma once

#include <cstdint>
#include <expected>
#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace gxapi::backend {

struct NandBuildConfig {
  std::string version;
  std::string imageType;
  std::string consoleModel;
  std::string cpuKeyHex;
  std::string outputPath;

  std::optional<std::filesystem::path> sourceNandPath;
  std::optional<std::filesystem::path> customKvPath;
  std::optional<std::filesystem::path> customSmcPath;
  std::optional<std::filesystem::path> customSmcConfigPath;
  std::optional<std::filesystem::path> xboxupdPath;

  std::vector<std::string> patches;
  std::vector<std::pair<std::string, std::string>> rawOptions;
};

struct BuilderProgressInfo {
  int percentage{0};
  std::string statusMessage;
};

using BuilderProgressCallback =
    std::function<void(const BuilderProgressInfo &)>;

struct BuildResult {
  bool success{false};
  std::string outputPath;
  std::string logOutput;
  std::string errorMessage;
};

class IBuilderService {
public:
  virtual ~IBuilderService() = default;

  [[nodiscard]] virtual std::string serviceName() const = 0;

  
  virtual std::vector<std::string> getAvailableVersions() = 0;
  virtual std::vector<std::string>
  getAvailableImageTypes(const std::string &version) = 0;
  virtual std::vector<std::string>
  getAvailableConsoles(const std::string &version,
                       const std::string &imageType) = 0;
  virtual std::vector<std::string>
  getAvailablePatches(const std::string &version) = 0;
  virtual std::vector<std::string>
  getAvailableSmcFiles(const std::string &consoleModel) = 0;

  
  virtual std::vector<std::string> getXellHacks() = 0;
  virtual std::vector<std::string> getXellImages(const std::string &hack) = 0;

  
  virtual std::vector<std::string> getSimpleVersions() = 0;
  virtual std::vector<std::string>
  getSimpleImageTypes(const std::string &version) = 0;
  virtual std::vector<std::string>
  getSimpleHacks(const std::string &version, const std::string &simpleType) = 0;
  virtual std::string
  resolveUnderlyingImageType(const std::string &simpleType,
                             const std::string &simpleHack) = 0;

  
  virtual std::expected<BuildResult, std::string>
  buildImage(const NandBuildConfig &config,
             BuilderProgressCallback progressCb = nullptr) = 0;
};

} 
