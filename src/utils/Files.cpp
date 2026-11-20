#include "utils/Files.hpp"

#include "utils/Settings.hpp"

#include <QRegularExpression>
#include <QStandardPaths>

#include <algorithm>
#include <array>
#include <cctype>
#include <charconv>
#include <fstream>
#include <system_error>
#include <unordered_set>

namespace genexis::utils::files {

namespace {

std::string toLowerAscii(std::string_view sv) {
    std::string out(sv);
    std::transform(out.begin(), out.end(), out.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return out;
}

bool equalsIgnoreCase(std::string_view a, std::string_view b) {
    if (a.size() != b.size()) {
        return false;
    }
    for (size_t i = 0; i < a.size(); ++i) {
        if (std::tolower(static_cast<unsigned char>(a[i])) !=
            std::tolower(static_cast<unsigned char>(b[i]))) {
            return false;
        }
    }
    return true;
}

bool startsWithIgnoreCase(std::string_view text, std::string_view prefix) {
    return text.size() >= prefix.size() &&
           equalsIgnoreCase(text.substr(0, prefix.size()), prefix);
}

bool endsWithIgnoreCase(std::string_view text, std::string_view suffix) {
    return text.size() >= suffix.size() &&
           equalsIgnoreCase(text.substr(text.size() - suffix.size()), suffix);
}

bool isBuildIniFilename(std::string_view filename) {
    return filename.size() > 5 && filename.front() == '_' && endsWithIgnoreCase(filename, ".ini");
}

std::optional<uint64_t> parseUnsigned(std::string_view sv) {
    if (sv.empty()) {
        return std::nullopt;
    }
    uint64_t val = 0;
    const char *begin = sv.data();
    const char *end = sv.data() + sv.size();
    const auto [ptr, ec] = std::from_chars(begin, end, val);
    if (ec == std::errc{} && ptr == end) {
        return val;
    }
    return std::nullopt;
}

std::optional<std::filesystem::path>
findChildCaseInsensitive(const std::filesystem::path &parent, std::string_view targetName,
                         bool wantDirectory) {
    std::error_code ec;
    if (!std::filesystem::is_directory(parent, ec)) {
        return std::nullopt;
    }

    for (const auto &entry : std::filesystem::directory_iterator(parent, ec)) {
        if (ec) {
            break;
        }
        const std::string name = entry.path().filename().string();
        if (!equalsIgnoreCase(name, targetName)) {
            continue;
        }
        std::error_code typeEc;
        if (wantDirectory && entry.is_directory(typeEc)) {
            return entry.path();
        }
        if (!wantDirectory && entry.is_regular_file(typeEc)) {
            return entry.path();
        }
    }
    return std::nullopt;
}

std::vector<std::string>
listFilesWithExtensions(const std::filesystem::path &dir,
                        std::span<const std::string_view> extensions) {
    std::vector<std::string> result;
    std::error_code ec;
    if (!std::filesystem::is_directory(dir, ec)) {
        return result;
    }

    for (const auto &entry : std::filesystem::directory_iterator(dir, ec)) {
        if (ec) {
            break;
        }
        std::error_code fileEc;
        if (!entry.is_regular_file(fileEc)) {
            continue;
        }
        const std::string name = entry.path().filename().string();
        const bool matched = std::any_of(extensions.begin(), extensions.end(),
                                         [&](std::string_view ext) {
                                             return endsWithIgnoreCase(name, ext);
                                         });
        if (matched) {
            result.push_back(name);
        }
    }

    std::sort(result.begin(), result.end());
    return result;
}

bool matchesSmcFilename(std::string_view filename, std::string_view baseConsole) {
    if (!endsWithIgnoreCase(filename, ".bin")) {
        return false;
    }
    if (baseConsole.empty()) {
        return startsWithIgnoreCase(filename, "smc");
    }
    if (equalsIgnoreCase(filename, "smc.bin")) {
        return true;
    }
    const std::string prefix = "smc_" + std::string(baseConsole) + "_";
    return startsWithIgnoreCase(filename, prefix);
}

} // namespace

QString pathToQString(const std::filesystem::path &path) {
#ifdef Q_OS_WIN
    return QString::fromStdWString(path.wstring());
#else
    return QString::fromStdString(path.string());
#endif
}

std::filesystem::path qStringToPath(const QString &path) {
    if (path.isEmpty()) {
        return {};
    }
#ifdef Q_OS_WIN
    return std::filesystem::path(path.toStdWString());
#else
    return std::filesystem::path(path.toStdString());
#endif
}

std::filesystem::path appDataDir() {
    return qStringToPath(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
}

std::filesystem::path xebuildDataDir() {
    const QString envOverride = qEnvironmentVariable("GENEXIS_XEBUILD_DATA");
    if (!envOverride.trimmed().isEmpty()) {
        return stringToPath(envOverride.trimmed());
    }

    const QString settingsOverride =
        genexis::utils::settings::SettingsController::instance().settings().xebuildDataPath();
    if (!settingsOverride.trimmed().isEmpty()) {
        return stringToPath(settingsOverride.trimmed());
    }

    return appDataDir() / "data" / "nand" / "xebuild";
}

std::filesystem::path smcDataDir() {
    return appDataDir() / "data" / "nand" / "smc";
}

std::filesystem::path xellDataDir() {
    return appDataDir() / "data" / "xell-images";
}

std::filesystem::path outputDir() {
    const QString settingsOverride =
        genexis::utils::settings::SettingsController::instance().settings().outputDirPath();
    if (!settingsOverride.trimmed().isEmpty()) {
        return stringToPath(settingsOverride.trimmed());
    }

    return appDataDir() / "output";
}

std::expected<void, std::string> initDirectories() {
    const std::filesystem::path dirs[] = {
        xebuildDataDir(),
        smcDataDir(),
        xellDataDir(),
        outputDir(),
    };

    for (const auto &dir : dirs) {
        if (dir.empty()) {
            continue;
        }
        std::error_code ec;
        std::filesystem::create_directories(dir, ec);
        if (ec) {
            return std::unexpected("Failed to create directory '" + dir.string() +
                                   "': " + ec.message());
        }
    }
    return {};
}

bool isValidVersionDir(const std::filesystem::path &dir) {
    std::error_code ec;
    if (!std::filesystem::is_directory(dir, ec)) {
        return false;
    }

    for (const auto &entry : std::filesystem::directory_iterator(dir, ec)) {
        if (ec) {
            return false;
        }
        std::error_code fileEc;
        if (!entry.is_regular_file(fileEc)) {
            continue;
        }
        if (isBuildIniFilename(entry.path().filename().string())) {
            return true;
        }
    }
    return false;
}

std::vector<std::string> listVersions(const std::filesystem::path &xebuildDir) {
    std::vector<std::string> versions;
    std::error_code ec;
    if (!std::filesystem::is_directory(xebuildDir, ec)) {
        return versions;
    }

    for (const auto &entry : std::filesystem::directory_iterator(xebuildDir, ec)) {
        if (ec) {
            break;
        }
        std::error_code dirEc;
        if (!entry.is_directory(dirEc)) {
            continue;
        }
        if (isValidVersionDir(entry.path())) {
            versions.push_back(entry.path().filename().string());
        }
    }

    std::sort(versions.begin(), versions.end(), [](const std::string &a, const std::string &b) {
        const auto numA = parseUnsigned(a);
        const auto numB = parseUnsigned(b);
        if (numA.has_value() && numB.has_value()) {
            if (*numA != *numB) {
                return *numA > *numB;
            }
            return a > b;
        }
        if (numA.has_value() != numB.has_value()) {
            return numA.has_value();
        }
        return a > b;
    });

    return versions;
}

std::vector<std::string> listImageTypes(const std::filesystem::path &xebuildDir,
                                        std::string_view version) {
    if (version.empty()) {
        return {};
    }

    std::vector<std::string> types;
    const auto versionDir = xebuildDir / std::filesystem::path(version);
    std::error_code ec;
    if (!std::filesystem::is_directory(versionDir, ec)) {
        return types;
    }

    for (const auto &entry : std::filesystem::directory_iterator(versionDir, ec)) {
        if (ec) {
            break;
        }
        std::error_code fileEc;
        if (!entry.is_regular_file(fileEc)) {
            continue;
        }
        const std::string name = entry.path().filename().string();
        if (!isBuildIniFilename(name)) {
            continue;
        }
        // Strip leading '_' and trailing ".ini".
        types.push_back(name.substr(1, name.size() - 5));
    }

    std::sort(types.begin(), types.end());
    return types;
}

std::vector<xebuildini::ConsoleSection>
listConsoleTargets(const std::filesystem::path &xebuildDir, std::string_view version,
                   std::string_view imageType) {
    if (version.empty() || imageType.empty()) {
        return {};
    }

    const auto versionDir = xebuildDir / std::filesystem::path(version);
    const std::string iniName = "_" + std::string(imageType) + ".ini";
    auto iniPath = findChildCaseInsensitive(versionDir, iniName, false);
    if (!iniPath) {
        iniPath = versionDir / iniName;
    }

    auto parsed = xebuildini::XebuildIni::load(*iniPath);
    if (!parsed) {
        return {};
    }

    return parsed->consoleTargets();
}

std::vector<std::string> listConsoles(const std::filesystem::path &xebuildDir,
                                      std::string_view version, std::string_view imageType) {
    const auto targets = listConsoleTargets(xebuildDir, version, imageType);
    std::vector<std::string> consoles;
    consoles.reserve(targets.size());
    for (const auto &target : targets) {
        consoles.push_back(target.stem);
    }
    return consoles;
}

std::vector<std::string> listAddonPatches(const std::filesystem::path &xebuildDir,
                                          std::string_view version) {
    if (version.empty()) {
        return {};
    }

    std::vector<std::string> patches;
    const auto binDir = xebuildDir / std::filesystem::path(version) / "bin";
    std::error_code ec;
    if (!std::filesystem::is_directory(binDir, ec)) {
        return patches;
    }

    for (const auto &entry : std::filesystem::directory_iterator(binDir, ec)) {
        if (ec) {
            break;
        }
        std::error_code fileEc;
        if (!entry.is_regular_file(fileEc)) {
            continue;
        }
        const std::string name = entry.path().filename().string();
        if (!endsWithIgnoreCase(name, ".bin")) {
            continue;
        }
        if (startsWithIgnoreCase(name, "patches_")) {
            continue;
        }
        patches.push_back(name.substr(0, name.size() - 4));
    }

    std::sort(patches.begin(), patches.end());
    return patches;
}

std::vector<std::string> listSmcFiles(const std::filesystem::path &smcDir,
                                      std::string_view consoleModel) {
    std::error_code ec;
    if (!std::filesystem::is_directory(smcDir, ec)) {
        return {};
    }

    const auto usPos = consoleModel.find('_');
    const std::string baseConsole =
        toLowerAscii((usPos != std::string_view::npos) ? consoleModel.substr(0, usPos)
                                                       : consoleModel);

    std::vector<std::string> files;
    for (const auto &entry : std::filesystem::directory_iterator(smcDir, ec)) {
        if (ec) {
            break;
        }
        std::error_code fileEc;
        if (!entry.is_regular_file(fileEc)) {
            continue;
        }
        const std::string name = entry.path().filename().string();
        if (matchesSmcFilename(name, baseConsole)) {
            files.push_back(name);
        }
    }

    std::sort(files.begin(), files.end());
    return files;
}

std::vector<std::string> listSmcFiles(std::span<const std::filesystem::path> searchDirs,
                                      std::string_view consoleModel) {
    std::vector<std::string> files;
    std::unordered_set<std::string> seenLower;

    for (const auto &dir : searchDirs) {
        for (auto &name : listSmcFiles(dir, consoleModel)) {
            if (seenLower.insert(toLowerAscii(name)).second) {
                files.push_back(std::move(name));
            }
        }
    }

    std::sort(files.begin(), files.end());
    return files;
}

std::vector<std::string> listXellHacks(const std::filesystem::path &xellDir) {
    std::vector<std::string> hacks;
    std::error_code ec;
    if (!std::filesystem::is_directory(xellDir, ec)) {
        return hacks;
    }

    for (const auto &entry : std::filesystem::directory_iterator(xellDir, ec)) {
        if (ec) {
            break;
        }
        std::error_code dirEc;
        if (entry.is_directory(dirEc)) {
            hacks.push_back(entry.path().filename().string());
        }
    }

    std::sort(hacks.begin(), hacks.end());
    return hacks;
}

std::vector<std::string> listXellImages(const std::filesystem::path &xellDir,
                                        std::string_view hack) {
    if (hack.empty()) {
        return {};
    }

    const auto hackDir = findChildCaseInsensitive(xellDir, hack, true);
    if (!hackDir) {
        return {};
    }
    static constexpr std::array<std::string_view, 2> kXellExtensions{".bin", ".ecc"};
    return listFilesWithExtensions(*hackDir, kXellExtensions);
}

BuildEnvironmentValidation validateBuildEnvironment(const std::filesystem::path &xebuildDir) {
    BuildEnvironmentValidation result;
    std::error_code ec;

    if (xebuildDir.empty() || !std::filesystem::exists(xebuildDir, ec) ||
        !std::filesystem::is_directory(xebuildDir, ec)) {
        result.errors.push_back("xeBuild data directory does not exist: " + xebuildDir.string());
        return result;
    }

    const auto commonDir = findChildCaseInsensitive(xebuildDir, "common", true);
    if (!commonDir) {
        result.errors.push_back("Missing 'common' directory under: " + xebuildDir.string());
    }

    result.versions = listVersions(xebuildDir);
    if (result.versions.empty()) {
        result.errors.push_back("No valid version folders (containing _*.ini) found under: " +
                                xebuildDir.string());
    }

    result.valid = result.errors.empty();
    return result;
}

std::filesystem::path urlToPath(const QUrl &url) {
    if (url.isEmpty()) {
        return {};
    }
    if (url.isLocalFile()) {
        return qStringToPath(url.toLocalFile());
    }
    return qStringToPath(url.toString());
}

std::filesystem::path stringToPath(const QString &pathOrUrl) {
    if (pathOrUrl.isEmpty()) {
        return {};
    }
    if (pathOrUrl.startsWith(QStringLiteral("file:"), Qt::CaseInsensitive)) {
        return urlToPath(QUrl(pathOrUrl));
    }
    return qStringToPath(pathOrUrl);
}

std::expected<std::vector<uint8_t>, std::string>
readBinaryFile(const std::filesystem::path &path) {
    std::error_code ec;
    if (!std::filesystem::exists(path, ec)) {
        return std::unexpected("File does not exist: " + path.string());
    }
    if (!std::filesystem::is_regular_file(path, ec)) {
        return std::unexpected("Path is not a regular file: " + path.string());
    }

    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
        return std::unexpected("Failed to open file for reading: " + path.string());
    }

    const std::streamsize size = file.tellg();
    if (size < 0) {
        return std::unexpected("Failed to determine file size: " + path.string());
    }

    std::vector<uint8_t> buffer(static_cast<size_t>(size));
    file.seekg(0, std::ios::beg);
    if (size > 0 && !file.read(reinterpret_cast<char *>(buffer.data()), size)) {
        return std::unexpected("Failed while reading file: " + path.string());
    }

    return buffer;
}

std::expected<void, std::string> writeBinaryFile(const std::filesystem::path &path,
                                                 std::span<const uint8_t> data) {
    const auto parent = path.parent_path();
    if (!parent.empty()) {
        std::error_code ec;
        std::filesystem::create_directories(parent, ec);
        if (ec) {
            return std::unexpected("Failed to create parent directory '" + parent.string() +
                                   "': " + ec.message());
        }
    }

    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file) {
        return std::unexpected("Failed to open file for writing: " + path.string());
    }

    if (!data.empty()) {
        file.write(reinterpret_cast<const char *>(data.data()),
                   static_cast<std::streamsize>(data.size()));
        if (!file) {
            return std::unexpected("Failed while writing file: " + path.string());
        }
    }

    return {};
}

std::optional<QString> detectSiblingCpuKey(const std::filesystem::path &nandPath) {
    if (nandPath.empty()) {
        return std::nullopt;
    }

    const auto parent = nandPath.parent_path();
    if (parent.empty()) {
        return std::nullopt;
    }

    const auto keyFile = findChildCaseInsensitive(parent, "cpukey.txt", false);
    if (!keyFile) {
        return std::nullopt;
    }

    const auto bytes = readBinaryFile(*keyFile);
    if (!bytes) {
        return std::nullopt;
    }

    const QString text = QString::fromUtf8(reinterpret_cast<const char *>(bytes->data()),
                                           static_cast<qsizetype>(bytes->size()));
    static const QRegularExpression hexRe(QStringLiteral("[0-9a-fA-F]{32}"));
    const QRegularExpressionMatch match = hexRe.match(text);
    if (!match.hasMatch()) {
        return std::nullopt;
    }

    return match.captured(0).toUpper();
}

} // namespace genexis::utils::files
