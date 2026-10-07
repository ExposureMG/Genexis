#pragma once

#include "backend/IBuilderService.hpp"
#include "cli/BuildInputResolver.hpp"

namespace gxapi::backend {

class GxBuild3Adapter : public IBuilderService {
public:
  GxBuild3Adapter() = default;
  explicit GxBuild3Adapter(std::filesystem::path xeBuildDataPath);
  ~GxBuild3Adapter() override = default;

  [[nodiscard]] std::string serviceName() const override { return "gxbuild3"; }

  std::vector<std::string> getAvailableVersions() override;
  std::vector<std::string>
  getAvailableImageTypes(const std::string &version) override;
  std::vector<std::string>
  getAvailableConsoles(const std::string &version,
                       const std::string &imageType) override;
  std::vector<std::string>
  getAvailablePatches(const std::string &version) override;
  std::vector<std::string>
  getAvailableSmcFiles(const std::string &consoleModel) override;

  std::vector<std::string> getXellHacks() override;
  std::vector<std::string> getXellImages(const std::string &hack) override;

  std::vector<std::string> getSimpleVersions() override;
  std::vector<std::string>
  getSimpleImageTypes(const std::string &version) override;
  std::vector<std::string>
  getSimpleHacks(const std::string &version,
                 const std::string &simpleType) override;
  std::string
  resolveUnderlyingImageType(const std::string &simpleType,
                             const std::string &simpleHack) override;

  std::expected<BuildResult, std::string>
  buildImage(const NandBuildConfig &config,
             BuilderProgressCallback progressCb = nullptr) override;

  [[nodiscard]]
  std::expected<gxbuild3::cli::BuildRequest, std::string>
  resolveBuildRequest(const NandBuildConfig &config,
                      const std::filesystem::path &stagingRoot) const;

private:
  std::filesystem::path getXeBuildDataPath() const;
  std::filesystem::path getXellDataPath() const;
  std::filesystem::path getSmcDataPath() const;

  std::filesystem::path m_xeBuildDataPath;
};

} // namespace gxapi::backend
