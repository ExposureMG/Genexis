#include "backend/adapters/GxBuild3Adapter.hpp"
#include "Args.hpp"
#include "Library.hpp"
#include "cli/BuildArgs.hpp"
#include "cli/BuildInputResolver.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTextStream>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <span>
#include <string_view>
#include <system_error>
#include <unordered_set>
#include <utility>

namespace gxapi::backend {

namespace {

std::string lowercase(std::string_view value) {
  std::string result(value);
  std::transform(result.begin(), result.end(), result.begin(),
                 [](unsigned char character) {
                   return static_cast<char>(std::tolower(character));
                 });
  return result;
}

std::string normalizeConsoleStem(std::string_view value) {
  std::string stem =
      lowercase(std::filesystem::path(value).stem().string());
  const auto marker = stem.find("bl");
  if (marker != std::string::npos && marker > 0 &&
      (marker + 2 == stem.size() || stem[marker + 2] == '_')) {
    const std::string motherboard = stem.substr(0, marker);
    if (kImageTypeMap.contains(motherboard)) {
      return motherboard;
    }
  }
  return kImageTypeMap.contains(stem) ? stem : std::string{};
}

std::string normalizeOptionName(std::string_view value) {
  while (!value.empty() &&
         std::isspace(static_cast<unsigned char>(value.front()))) {
    value.remove_prefix(1);
  }
  while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back()))) {
    value.remove_suffix(1);
  }
  while (!value.empty() && value.front() == '-') {
    value.remove_prefix(1);
  }
  return lowercase(value);
}

bool isRecognizedUiOption(std::string_view name) {
  static const std::unordered_set<std::string> recognized{
      "cygnos",      "demon",       "olddvd",       "nodvd",
      "nomobile",    "nofcrt",      "noremap",      "noecdremap",
      "nandmu",      "nosecurity",  "nosusecurity", "smcnocheck",
      "nochecksmc",  "noblpatch",   "nopatch",      "cbldv",
      "pairing_data", "pairingdata", "pd",          "cfldv",
      "xellbutton",  "xellbutton2", "dualboot",     "cputemp",
      "gputemp",     "edramtemp",   "overcputemp",  "overgputemp",
      "overedramtemp", "cpufan",    "gpufan",       "dvdkey",
      "avregion",    "gameregion",  "dvdregion",    "macid"};
  return recognized.contains(std::string(name));
}

std::string normalizeCpuKey(std::string_view value) {
  std::string compact;
  compact.reserve(value.size());
  for (const unsigned char character : value) {
    if (!std::isspace(character)) {
      compact.push_back(static_cast<char>(std::toupper(character)));
    }
  }
  return compact;
}

std::expected<void, std::string>
ensureDirectory(const std::filesystem::path &directory,
                std::string_view description) {
  std::error_code error;
  std::filesystem::create_directories(directory, error);
  if (error) {
    return std::unexpected("Could not create " + std::string(description) +
                           " '" + directory.string() + "': " +
                           error.message());
  }
  if (!std::filesystem::is_directory(directory, error) || error) {
    return std::unexpected("Expected " + std::string(description) +
                           " to be a directory: " + directory.string());
  }
  return {};
}

std::expected<void, std::string>
copySelectedFile(const std::filesystem::path &source,
                 const std::filesystem::path &destination,
                 std::string_view description) {
  std::error_code error;
  const auto status = std::filesystem::status(source, error);
  if (error || !std::filesystem::is_regular_file(status)) {
    std::string message = "Could not read selected " + std::string(description) +
                          " file '" + source.string() + "'";
    if (error) {
      message += ": " + error.message();
    }
    return std::unexpected(std::move(message));
  }
  const auto directory = ensureDirectory(destination.parent_path(),
                                         "gxbuild3 staging directory");
  if (!directory) {
    return directory;
  }
  std::filesystem::copy_file(source, destination,
                             std::filesystem::copy_options::overwrite_existing,
                             error);
  if (error) {
    return std::unexpected("Could not stage selected " +
                           std::string(description) + " file '" +
                           source.string() + "' as '" + destination.string() +
                           "': " + error.message());
  }
  return {};
}

std::vector<std::string>
availableVersionsAt(const std::filesystem::path &xeBuildDataPath) {
  std::vector<std::string> versions;
  std::error_code error;
  for (std::filesystem::directory_iterator it(xeBuildDataPath, error), end;
       !error && it != end; it.increment(error)) {
    if (!it->is_directory(error) || error) {
      continue;
    }
    bool hasBuildIni = false;
    std::error_code childError;
    for (std::filesystem::directory_iterator child(it->path(), childError),
         childEnd;
         !childError && child != childEnd; child.increment(childError)) {
      const auto filename = child->path().filename().string();
      if (child->is_regular_file(childError) && !childError &&
          filename.starts_with('_') && child->path().extension() == ".ini") {
        hasBuildIni = true;
        break;
      }
    }
    if (hasBuildIni) {
      versions.push_back(it->path().filename().string());
    }
  }
  std::sort(versions.begin(), versions.end(),
            [](const std::string &left, const std::string &right) {
              try {
                return std::stoll(left) > std::stoll(right);
              } catch (...) {
                return left > right;
              }
            });
  return versions;
}

std::string describeResolutionError(
    const gxbuild3::cli::ResolutionError &error) {
  std::string message = error.message;
  if (!error.path.empty()) {
    message += " [path: " + error.path.string() + "]";
  }
  if (!error.item.empty()) {
    message += " [item: " + error.item + "]";
  }
  return message;
}

std::expected<void, std::string>
writeOutput(const std::filesystem::path &path, std::span<const uint8_t> bytes) {
  if (path.has_parent_path()) {
    const auto directory = ensureDirectory(path.parent_path(),
                                           "NAND output directory");
    if (!directory) {
      return directory;
    }
  }
  std::ofstream output(path, std::ios::binary | std::ios::trunc);
  if (!output) {
    return std::unexpected("Could not open output NAND for writing: " +
                           path.string());
  }
  output.write(reinterpret_cast<const char *>(bytes.data()),
               static_cast<std::streamsize>(bytes.size()));
  if (!output.good()) {
    return std::unexpected("Could not write output NAND: " + path.string());
  }
  return {};
}

std::filesystem::path defaultAppDataPath() {
  return QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)
      .toStdString();
}

} // namespace

GxBuild3Adapter::GxBuild3Adapter(std::filesystem::path xeBuildDataPath)
    : m_xeBuildDataPath(std::move(xeBuildDataPath)) {}

std::filesystem::path GxBuild3Adapter::getXeBuildDataPath() const {
  if (!m_xeBuildDataPath.empty()) {
    return m_xeBuildDataPath;
  }
  const QString appData = QString::fromStdString(defaultAppDataPath().string());
  return std::filesystem::path(
      QDir(appData)
          .filePath(QStringLiteral("data/nand/xebuild"))
          .toStdString());
}

std::filesystem::path GxBuild3Adapter::getXellDataPath() const {
  const QString appData = QString::fromStdString(defaultAppDataPath().string());
  return std::filesystem::path(
      QDir(appData).filePath(QStringLiteral("data/xell-images")).toStdString());
}

std::filesystem::path GxBuild3Adapter::getSmcDataPath() const {
  const QString appData = QString::fromStdString(defaultAppDataPath().string());
  return std::filesystem::path(
      QDir(appData).filePath(QStringLiteral("data/nand/smc")).toStdString());
}

std::vector<std::string> GxBuild3Adapter::getAvailableVersions() {
  std::vector<std::string> versions;
  QDir xeDir(QString::fromStdString(getXeBuildDataPath().string()));
  if (xeDir.exists()) {
    QFileInfoList entries =
        xeDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const auto &entry : entries) {
      QDir verDir(entry.absoluteFilePath());
      QFileInfoList iniEntries = verDir.entryInfoList(
          QStringList{QStringLiteral("_*.ini")}, QDir::Files);
      if (!iniEntries.isEmpty()) {
        std::string name = entry.fileName().toStdString();
        if (std::find(versions.begin(), versions.end(), name) ==
            versions.end()) {
          versions.push_back(name);
        }
      }
    }
  }

  std::sort(versions.begin(), versions.end(),
            [](const std::string &a, const std::string &b) {
              try {
                int numA = std::stoi(a);
                int numB = std::stoi(b);
                return numA > numB;
              } catch (...) {
                return a > b;
              }
            });

  return versions;
}

std::vector<std::string>
GxBuild3Adapter::getAvailableImageTypes(const std::string &version) {
  std::vector<std::string> types;
  if (version.empty())
    return types;

  QString verPath = QDir(QString::fromStdString(getXeBuildDataPath().string()))
                        .filePath(QString::fromStdString(version));
  QDir dir(verPath);
  if (dir.exists()) {
    QFileInfoList entries =
        dir.entryInfoList(QStringList{QStringLiteral("_*.ini")}, QDir::Files);
    for (const auto &entry : entries) {
      QString name = entry.fileName();
      if (name.startsWith(u'_') &&
          name.endsWith(QStringLiteral(".ini"), Qt::CaseInsensitive)) {
        std::string typeName = name.mid(1, name.length() - 5).toStdString();
        if (std::find(types.begin(), types.end(), typeName) == types.end()) {
          types.push_back(typeName);
        }
      }
    }
  }

  std::sort(types.begin(), types.end());
  return types;
}

std::vector<std::string>
GxBuild3Adapter::getAvailableConsoles(const std::string &version,
                                      const std::string &imageType) {
  std::vector<std::string> consoles;
  std::vector<std::string> sections;
  if (version.empty() || imageType.empty())
    return consoles;

  QString iniName = QStringLiteral("_") + QString::fromStdString(imageType) +
                    QStringLiteral(".ini");
  QString iniPath = QDir(QString::fromStdString(getXeBuildDataPath().string()))
                        .filePath(QString::fromStdString(version) +
                                  QStringLiteral("/") + iniName);

  QFile iniFile(iniPath);
  if (iniFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QTextStream stream(&iniFile);
    while (!stream.atEnd()) {
      QString line = stream.readLine().trimmed();
      if (line.startsWith(u'[') && line.endsWith(u']')) {
        QString section = line.mid(1, line.length() - 2).trimmed();
        QString secLower = section.toLower();
        if (secLower != QStringLiteral("version") &&
            secLower != QStringLiteral("security") &&
            secLower != QStringLiteral("rawpatch") &&
            secLower != QStringLiteral("flashfs")) {
          sections.push_back(secLower.toStdString());
        }
      }
    }
    iniFile.close();
  }

  const std::unordered_set<std::string> availableSections(sections.begin(),
                                                          sections.end());
  for (const auto &section : sections) {
    const std::string console = normalizeConsoleStem(section);
    if (console.empty() ||
        !availableSections.contains(console + "bl") ||
        std::find(consoles.begin(), consoles.end(), console) !=
            consoles.end()) {
      continue;
    }
    consoles.push_back(console);
  }

  return consoles;
}

std::vector<std::string>
GxBuild3Adapter::getAvailablePatches(const std::string &version) {
  std::vector<std::string> patches;
  if (version.empty())
    return patches;

  QString binPath =
      QDir(QString::fromStdString(getXeBuildDataPath().string()))
          .filePath(QString::fromStdString(version) + QStringLiteral("/bin"));
  QDir binDir(binPath);
  if (binDir.exists()) {
    QFileInfoList entries =
        binDir.entryInfoList(QStringList{QStringLiteral("*.bin")}, QDir::Files);
    for (const auto &entry : entries) {
      QString fname = entry.fileName();
      if (!fname.startsWith(QStringLiteral("patches_"), Qt::CaseInsensitive)) {
        std::string pName = entry.completeBaseName().toStdString();
        if (std::find(patches.begin(), patches.end(), pName) == patches.end()) {
          patches.push_back(pName);
        }
      }
    }
  }

  std::sort(patches.begin(), patches.end());
  return patches;
}

std::vector<std::string>
GxBuild3Adapter::getAvailableSmcFiles(const std::string &consoleModel) {
  std::vector<std::string> smcFiles;
  if (consoleModel.empty())
    return smcFiles;

  QDir baseDir(QString::fromStdString(getSmcDataPath().string()));
  if (baseDir.exists()) {
    QString targetConsoleDir = QString::fromStdString(consoleModel);
    if (!baseDir.exists(targetConsoleDir)) {
      QFileInfoList consoleDirs =
          baseDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
      for (const auto &cDir : consoleDirs) {
        if (cDir.fileName().compare(QString::fromStdString(consoleModel),
                                    Qt::CaseInsensitive) == 0) {
          targetConsoleDir = cDir.fileName();
          break;
        }
      }
    }

    QDir smcDir(baseDir.filePath(targetConsoleDir));
    if (smcDir.exists()) {
      QFileInfoList entries = smcDir.entryInfoList(
          QStringList{QStringLiteral("*.bin")}, QDir::Files);
      for (const auto &entry : entries) {
        std::string fname = entry.fileName().toStdString();
        if (std::find(smcFiles.begin(), smcFiles.end(), fname) ==
            smcFiles.end()) {
          smcFiles.push_back(fname);
        }
      }
    }
  }

  std::sort(smcFiles.begin(), smcFiles.end());
  return smcFiles;
}

std::vector<std::string> GxBuild3Adapter::getXellHacks() {
  std::vector<std::string> hacks;
  QDir dir(QString::fromStdString(getXellDataPath().string()));
  if (dir.exists()) {
    QFileInfoList entries =
        dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const auto &entry : entries) {
      hacks.push_back(entry.fileName().toStdString());
    }
    std::sort(hacks.begin(), hacks.end());
  }
  return hacks;
}

std::vector<std::string>
GxBuild3Adapter::getXellImages(const std::string &hack) {
  std::vector<std::string> images;
  if (hack.empty())
    return images;

  QString hackPath = QDir(QString::fromStdString(getXellDataPath().string()))
                         .filePath(QString::fromStdString(hack));
  QDir dir(hackPath);
  if (dir.exists()) {
    QFileInfoList entries = dir.entryInfoList(QDir::Files);
    for (const auto &entry : entries) {
      std::string name = entry.completeBaseName().toStdString();
      if (std::find(images.begin(), images.end(), name) == images.end()) {
        images.push_back(name);
      }
    }
    std::sort(images.begin(), images.end());
  }
  return images;
}

std::vector<std::string> GxBuild3Adapter::getSimpleVersions() {
  std::vector<std::string> versions;
  versions.push_back("Latest");
  auto avail = getAvailableVersions();
  for (const auto &v : avail) {
    versions.push_back(v);
  }
  return versions;
}

std::vector<std::string>
GxBuild3Adapter::getSimpleImageTypes(const std::string &version) {
  std::vector<std::string> types;
  std::string resVer = version;
  if (resVer.empty() || resVer == "Latest") {
    auto avail = getAvailableVersions();
    resVer = avail.empty() ? "" : avail.front();
  }

  if (resVer.empty()) {
    return {"Retail", "FreeBoot", "Devkit"};
  }

  QString verPath = QDir(QString::fromStdString(getXeBuildDataPath().string()))
                        .filePath(QString::fromStdString(resVer));
  QDir dir(verPath);
  if (dir.exists()) {
    if (QFileInfo::exists(dir.filePath(QStringLiteral("_retail.ini")))) {
      types.push_back("Retail");
    }
    QFileInfoList entries = dir.entryInfoList(
        QStringList{QStringLiteral("_glitch*.ini")}, QDir::Files);
    if (!entries.isEmpty()) {
      types.push_back("FreeBoot");
    }
    if (QFileInfo::exists(dir.filePath(QStringLiteral("_devkit.ini"))) ||
        QFileInfo::exists(dir.filePath(QStringLiteral("_devgl.ini"))) ||
        QFileInfo::exists(dir.filePath(QStringLiteral("_devrgl.ini")))) {
      types.push_back("Devkit");
    }
  }

  if (types.empty()) {
    types = {"Retail", "FreeBoot", "Devkit"};
  }

  return types;
}

std::vector<std::string>
GxBuild3Adapter::getSimpleHacks(const std::string &version,
                                const std::string &simpleType) {
  std::vector<std::string> hacks;
  std::string resVer = version;
  if (resVer.empty() || resVer == "Latest") {
    auto avail = getAvailableVersions();
    resVer = avail.empty() ? "" : avail.front();
  }

  if (simpleType == "Retail") {
    return {"None"};
  }

  if (simpleType == "FreeBoot") {
    if (resVer.empty()) {
      return {"RGH 1", "RGH 1.2", "RGH 2",  "S-RGH",
              "RGH 3", "RGH 1.3", "EXT_CLK"};
    }
    QString verPath =
        QDir(QString::fromStdString(getXeBuildDataPath().string()))
            .filePath(QString::fromStdString(resVer));
    QDir dir(verPath);
    if (dir.exists()) {
      if (QFileInfo::exists(dir.filePath(QStringLiteral("_glitch.ini")))) {
        hacks.push_back("RGH 1");
      }
      if (QFileInfo::exists(dir.filePath(QStringLiteral("_glitch2.ini"))) ||
          QFileInfo::exists(dir.filePath(QStringLiteral("_glitch2m.ini")))) {
        hacks.push_back("RGH 1.2");
        hacks.push_back("RGH 2");
        hacks.push_back("S-RGH");
      }
      if (QFileInfo::exists(dir.filePath(QStringLiteral("_glitch3.ini")))) {
        hacks.push_back("RGH 3");
        hacks.push_back("RGH 1.3");
        hacks.push_back("EXT_CLK.3");
      }
      if (QFileInfo::exists(dir.filePath(QStringLiteral("_jtag.ini")))) {
        hacks.push_back("Argon Data");
        hacks.push_back("AUD_CLAMP");
        hacks.push_back("R-JTAG");
      }
    }
    if (hacks.empty()) {
      return {"RGH 1", "RGH 1.2", "RGH 2",  "S-RGH",
              "RGH 3", "RGH 1.3", "EXT_CLK"};
    }
    return hacks;
  }

  if (simpleType == "Devkit") {
    if (resVer.empty()) {
      return {"None", "DevGL", "DevRGL"};
    }
    QString verPath =
        QDir(QString::fromStdString(getXeBuildDataPath().string()))
            .filePath(QString::fromStdString(resVer));
    QDir dir(verPath);
    if (dir.exists()) {
      if (QFileInfo::exists(dir.filePath(QStringLiteral("_devkit.ini")))) {
        hacks.push_back("None");
      }
      if (QFileInfo::exists(dir.filePath(QStringLiteral("_devgl.ini")))) {
        hacks.push_back("DevGL");
      }
      if (QFileInfo::exists(dir.filePath(QStringLiteral("_devrgl.ini")))) {
        hacks.push_back("DevRGL");
      }
    }
    if (hacks.empty()) {
      return {"None", "DevGL", "DevRGL"};
    }
    return hacks;
  }

  return {"None"};
}

std::string
GxBuild3Adapter::resolveUnderlyingImageType(const std::string &simpleType,
                                            const std::string &simpleHack) {
  if (simpleType == "Retail") {
    return "retail";
  }
  if (simpleType == "FreeBoot") {
    if (simpleHack == "RGH 1")
      return "glitch";
    if (simpleHack == "RGH 3" || simpleHack == "RGH 1.3" ||
        simpleHack == "EXT_CLK.3")
      return "glitch3";
    if (simpleHack == "Argon Data" || simpleHack == "AUD_CLAMP" ||
        simpleHack == "R-JTAG")
      return "jtag";
    return "glitch2";
  }
  if (simpleType == "Devkit") {
    if (simpleHack == "DevGL")
      return "devgl";
    if (simpleHack == "DevRGL")
      return "devrgl";
    return "devkit";
  }
  return "retail";
}

std::expected<gxbuild3::cli::BuildRequest, std::string>
GxBuild3Adapter::resolveBuildRequest(
    const NandBuildConfig &config,
    const std::filesystem::path &stagingRoot) const {
  if (config.xellOnly) {
    return std::unexpected(
        "XeLL-only images are not supported by the gxbuild3 NAND resolver");
  }

  std::string version = config.version;
  if (version == "Latest") {
    const auto versions = availableVersionsAt(getXeBuildDataPath());
    if (versions.empty()) {
      return std::unexpected(
          "Could not resolve Latest because no gxbuild3 versions are available");
    }
    version = versions.front();
  }
  if (version.empty()) {
    return std::unexpected("A gxbuild3 version is required");
  }
  if (config.imageType.empty()) {
    return std::unexpected("A gxbuild3 image type is required");
  }
  if (config.consoleModel.empty()) {
    return std::unexpected("A gxbuild3 console model is required");
  }

  const std::string imageType = lowercase(config.imageType);
  const auto buildType = kBuildTypeMap.find(imageType);
  if (buildType == kBuildTypeMap.end()) {
    return std::unexpected("Unsupported gxbuild3 image type: " +
                           config.imageType);
  }

  const std::string consoleStem = normalizeConsoleStem(config.consoleModel);
  const auto imageGeometry = kImageTypeMap.find(consoleStem);
  if (imageGeometry == kImageTypeMap.end()) {
    return std::unexpected("Unsupported gxbuild3 console model: " +
                           config.consoleModel);
  }

  const auto staging = ensureDirectory(stagingRoot, "gxbuild3 staging root");
  if (!staging) {
    return std::unexpected(staging.error());
  }
  if (config.customKvPath) {
    const auto copied = copySelectedFile(*config.customKvPath,
                                         stagingRoot / "kv.bin", "keyvault");
    if (!copied) {
      return std::unexpected(copied.error());
    }
  }
  if (config.customSmcPath) {
    const auto copied = copySelectedFile(*config.customSmcPath,
                                         stagingRoot / "smc.bin", "SMC");
    if (!copied) {
      return std::unexpected(copied.error());
    }
  }

  gxbuild3::cli::BuildArgs args{};
  args.build_ini = std::filesystem::path(version) /
                   ("_" + imageType + ".ini");
  args.section = consoleStem;
  if (const auto console = kConsoleTypeMap.find(consoleStem);
      console != kConsoleTypeMap.end()) {
    args.console = console->second;
  }
  args.build_type = buildType->second;
  args.image_type = imageGeometry->second;
  args.source_dirs = {stagingRoot, getXeBuildDataPath() / "data",
                      getXeBuildDataPath() / version,
                      getXeBuildDataPath() / "common"};
  args.input_path = config.sourceNandPath;
  args.output_path = config.outputPath.empty()
                         ? defaultAppDataPath() / "output" / "updflash.bin"
                         : std::filesystem::path(config.outputPath);

  if (!config.cpuKeyHex.empty()) {
    args.cpu_key = normalizeCpuKey(config.cpuKeyHex);
  }

  args.config.reserve(config.rawOptions.size());
  for (const auto &[rawName, value] : config.rawOptions) {
    const std::string name = normalizeOptionName(rawName);
    if (!isRecognizedUiOption(name)) {
      continue;
    }
    args.config.push_back(name + "=" + (value.empty() ? "true" : value));
  }

  args.addons.reserve(config.patches.size());
  for (const auto &selected : config.patches) {
    const std::filesystem::path selectedPath(selected);
    const std::string addon = selectedPath.stem().string();
    if (addon.empty()) {
      return std::unexpected("Selected gxbuild3 add-on has no logical name: " +
                             selected);
    }
    if (selectedPath.is_absolute() || selectedPath.has_parent_path()) {
      const auto copied = copySelectedFile(
          selectedPath, stagingRoot / "bin" / (addon + ".bin"), "add-on");
      if (!copied) {
        return std::unexpected(copied.error());
      }
    }
    args.addons.push_back(addon);
  }

  gxbuild3::cli::BuildInputResolver resolver{getXeBuildDataPath()};
  auto request = resolver.Resolve(args);
  if (!request) {
    return std::unexpected(describeResolutionError(request.error()));
  }
  return std::move(*request);
}

std::expected<BuildResult, std::string>
GxBuild3Adapter::buildImage(const NandBuildConfig &config,
                            BuilderProgressCallback progressCb) {
  if (progressCb) {
    progressCb(BuilderProgressInfo{
        .percentage = 10,
        .statusMessage = "Resolving gxbuild3 NAND inputs..."});
  }

  QTemporaryDir staging(QDir::temp().filePath(
      QStringLiteral("genexis-gxbuild3-XXXXXX")));
  if (!staging.isValid()) {
    return std::unexpected("Could not create isolated gxbuild3 staging root");
  }
  const auto request = resolveBuildRequest(
      config, std::filesystem::path(staging.path().toStdString()));
  if (!request) {
    return std::unexpected(request.error());
  }
  if (progressCb) {
    progressCb(BuilderProgressInfo{
        .percentage = 60,
        .statusMessage = "Assembling NAND image in-process with gxbuild3..."});
  }

  const auto builtImage = GxBuild::RunBuild(request->input);
  if (!builtImage) {
    return std::unexpected("gxbuild3 assembly error: " +
                           builtImage.error().message);
  }

  if (progressCb) {
    progressCb(BuilderProgressInfo{.percentage = 85,
                                   .statusMessage = "Writing NAND image..."});
  }
  const auto written = writeOutput(request->output_path, *builtImage);
  if (!written) {
    return std::unexpected(written.error());
  }

  if (progressCb) {
    progressCb(BuilderProgressInfo{
        .percentage = 100,
        .statusMessage = "NAND image assembled natively with gxbuild3!"});
  }

  return BuildResult{.success = true,
                     .outputPath = request->output_path.string(),
                     .logOutput = "Successfully built NAND image: " +
                                  request->output_path.string() + " (" +
                                  std::to_string(builtImage->size()) +
                                  " bytes)"};
}

} // namespace gxapi::backend
