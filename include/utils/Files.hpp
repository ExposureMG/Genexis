#pragma once

#include "utils/files/XebuildIni.hpp"

#include <QString>
#include <QUrl>

#include <cstdint>
#include <expected>
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace genexis::utils::files {

/// Diagnostic summary returned by validateBuildEnvironment().
struct BuildEnvironmentValidation {
    bool valid{false};
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
    std::vector<std::string> versions;
};

/// Converts a std::filesystem::path to a QString.
[[nodiscard]] QString pathToQString(const std::filesystem::path &path);

/// Converts a QString local path to a std::filesystem::path.
[[nodiscard]] std::filesystem::path qStringToPath(const QString &path);

/// Returns the platform-specific AppLocalDataLocation directory.
[[nodiscard]] std::filesystem::path appDataDir();

/// Returns the xeBuild data directory (env var -> Settings -> AppLocalDataLocation).
[[nodiscard]] std::filesystem::path xebuildDataDir();

/// Returns the SMC data directory under AppLocalDataLocation.
[[nodiscard]] std::filesystem::path smcDataDir();

/// Returns the XeLL images directory under AppLocalDataLocation.
[[nodiscard]] std::filesystem::path xellDataDir();

/// Returns the build output directory (Settings -> AppLocalDataLocation).
[[nodiscard]] std::filesystem::path outputDir();

/// Creates all required runtime data and output directories.
[[nodiscard]] std::expected<void, std::string> initDirectories();

/// Returns true if \p dir exists and contains at least one _*.ini build definition.
[[nodiscard]] bool isValidVersionDir(const std::filesystem::path &dir);

/// Lists valid version directories under \p xebuildDir, sorted descending.
[[nodiscard]] std::vector<std::string> listVersions(const std::filesystem::path &xebuildDir);

/// Lists image types (stripped from _*.ini) under <xebuildDir>/<version>, sorted ascending.
[[nodiscard]] std::vector<std::string> listImageTypes(const std::filesystem::path &xebuildDir,
                                                      std::string_view version);

/// Lists console target metadata defined in <xebuildDir>/<version>/_<imageType>.ini.
[[nodiscard]] std::vector<xebuildini::ConsoleSection>
listConsoleTargets(const std::filesystem::path &xebuildDir, std::string_view version,
                   std::string_view imageType);

/// Lists console target stems defined in <xebuildDir>/<version>/_<imageType>.ini.
[[nodiscard]] std::vector<std::string> listConsoles(const std::filesystem::path &xebuildDir,
                                                    std::string_view version,
                                                    std::string_view imageType);

/// Lists add-on patch stems in <xebuildDir>/<version>/bin/*.bin, excluding patches_*.bin.
[[nodiscard]] std::vector<std::string> listAddonPatches(const std::filesystem::path &xebuildDir,
                                                        std::string_view version);

/// Lists matching *.bin SMC filenames directly inside \p smcDir (flat layout).
[[nodiscard]] std::vector<std::string> listSmcFiles(const std::filesystem::path &smcDir,
                                                    std::string_view consoleModel);

/// Lists matching *.bin SMC filenames across multiple standard search directories (deduplicated).
[[nodiscard]] std::vector<std::string>
listSmcFiles(std::span<const std::filesystem::path> searchDirs, std::string_view consoleModel);

/// Lists XeLL hack subdirectories under \p xellDir, sorted ascending.
[[nodiscard]] std::vector<std::string> listXellHacks(const std::filesystem::path &xellDir);

/// Lists *.bin and *.ecc XeLL image filenames in <xellDir>/<hack> (case-insensitive folder lookup).
[[nodiscard]] std::vector<std::string> listXellImages(const std::filesystem::path &xellDir,
                                                      std::string_view hack);

/// Validates the xeBuild data root, common/ directory, and available version folders.
[[nodiscard]] BuildEnvironmentValidation
validateBuildEnvironment(const std::filesystem::path &xebuildDir);

/// Converts a QUrl (handling file:// URLs cross-platform) to a filesystem path.
[[nodiscard]] std::filesystem::path urlToPath(const QUrl &url);

/// Converts a plain local path string or file:// URL string to a filesystem path.
[[nodiscard]] std::filesystem::path stringToPath(const QString &pathOrUrl);

/// Reads an entire regular binary file into memory.
[[nodiscard]] std::expected<std::vector<uint8_t>, std::string>
readBinaryFile(const std::filesystem::path &path);

/// Writes binary bytes to \p path, creating parent directories if needed.
[[nodiscard]] std::expected<void, std::string>
writeBinaryFile(const std::filesystem::path &path, std::span<const uint8_t> data);

/// Looks for a sibling cpukey.txt (case-insensitive) next to \p nandPath and extracts the first 32-hex key.
[[nodiscard]] std::optional<QString> detectSiblingCpuKey(const std::filesystem::path &nandPath);

} // namespace genexis::utils::files
