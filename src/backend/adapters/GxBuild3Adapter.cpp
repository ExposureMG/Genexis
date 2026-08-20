#include "backend/adapters/GxBuild3Adapter.hpp"
#include "Library.hpp"
#include "Args.hpp"
#include "utils/FileManager.hpp"
#include "ini/IniParser.hpp"
#include "nand/objects/Keyvault.hpp"
#include "utils/Utils.hpp"
#include "StartupManager.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <algorithm>
#include <fstream>
#include <sstream>

namespace gxapi::backend {

std::filesystem::path GxBuild3Adapter::getXeBuildDataPath() const {
  QString appData = StartupManager::instance().appDataPath();
  return std::filesystem::path(
      QDir(appData)
          .filePath(QStringLiteral("data/nand/xebuild"))
          .toStdString());
}

std::filesystem::path GxBuild3Adapter::getXellDataPath() const {
  QString appData = StartupManager::instance().appDataPath();
  return std::filesystem::path(
      QDir(appData).filePath(QStringLiteral("data/xell-images")).toStdString());
}

std::filesystem::path GxBuild3Adapter::getSmcDataPath() const {
  QString appData = StartupManager::instance().appDataPath();
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
          std::string secStr = section.toStdString();
          if (std::find(consoles.begin(), consoles.end(), secStr) ==
              consoles.end()) {
            consoles.push_back(secStr);
          }
        }
      }
    }
    iniFile.close();
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

std::expected<BuildResult, std::string>
GxBuild3Adapter::buildImage(const NandBuildConfig &config,
                            BuilderProgressCallback progressCb) {
  if (progressCb) {
    progressCb(BuilderProgressInfo{
        .percentage = 10,
        .statusMessage = "Preparing gxbuild3 native engine parameters..."});
  }

  std::string resolvedVersion = config.version;
  if (resolvedVersion.empty() || resolvedVersion == "Latest") {
    auto avail = getAvailableVersions();
    resolvedVersion = avail.empty() ? "17559" : avail.front();
  }

  std::filesystem::path xeDataPath = getXeBuildDataPath();
  std::filesystem::path fwDir = xeDataPath / "data";

  std::vector<uint8_t> cpuKeyBytes;
  if (!config.cpuKeyHex.empty()) {
    auto keyRes = gxbuild3::NAND::validate_cpu_key_hex(config.cpuKeyHex);
    if (keyRes.status == gxbuild3::NAND::CpuKeyStatus::Valid ||
        keyRes.status == gxbuild3::NAND::CpuKeyStatus::Corrected) {
      cpuKeyBytes = std::move(keyRes.key);
    } else {
      for (size_t i = 0; i + 1 < config.cpuKeyHex.length(); i += 2) {
        try {
          uint8_t byteVal = static_cast<uint8_t>(
              std::stoul(config.cpuKeyHex.substr(i, 2), nullptr, 16));
          cpuKeyBytes.push_back(byteVal);
        } catch (...) {
        }
      }
    }
  }

  OptionsManager optionsMgr;
  for (const auto &[key, val] : config.rawOptions) {
    if (val.empty()) {
      optionsMgr.set_bool(key, true);
    } else {
      optionsMgr.set(key, val);
    }
  }

  InputMetadata metadata{};
  if (config.sourceNandPath && std::filesystem::exists(*config.sourceNandPath)) {
    if (progressCb) {
      progressCb(BuilderProgressInfo{
          .percentage = 20,
          .statusMessage = "Extracting donor NAND metadata..."});
    }
    auto nandData = Utils::read_file(*config.sourceNandPath);
    if (nandData) {
      auto extracted = GxBuild::ExtractMetadata(*nandData, cpuKeyBytes);
      if (extracted) {
        metadata = std::move(*extracted);
      }
    }
  }

  if (metadata.cpu_key.empty() && !cpuKeyBytes.empty()) {
    metadata.cpu_key = cpuKeyBytes;
  }

  if (auto cbldvOpt = optionsMgr.get_string("cbldv")) {
    metadata.cb_ldv =
        static_cast<uint8_t>(std::strtoul(cbldvOpt->c_str(), nullptr, 0));
  }
  if (auto cfldvOpt = optionsMgr.get_string("cfldv")) {
    metadata.cf_ldv =
        static_cast<uint8_t>(std::strtoul(cfldvOpt->c_str(), nullptr, 0));
  }
  if (auto pdOpt = optionsMgr.get_string("pairing_data")) {
    auto pdBytes = Utils::hex_string_to_bytes(*pdOpt);
    if (pdBytes.size() >= 3) {
      std::copy_n(pdBytes.begin(), 3, metadata.pairing_data);
    }
  }

  if (config.customKvPath && std::filesystem::exists(*config.customKvPath)) {
    if (auto kvData = Utils::read_file(*config.customKvPath)) {
      metadata.keyvault = std::move(*kvData);
    }
  }

  if (progressCb) {
    progressCb(BuilderProgressInfo{
        .percentage = 35,
        .statusMessage = "Resolving INI configuration & bootloaders..."});
  }

  struct CurrentPathGuard {
    std::filesystem::path prevPath;
    CurrentPathGuard(const std::filesystem::path &newPath)
        : prevPath(std::filesystem::current_path()) {
      std::error_code ec;
      std::filesystem::current_path(newPath, ec);
    }
    ~CurrentPathGuard() {
      std::error_code ec;
      std::filesystem::current_path(prevPath, ec);
    }
  } pathGuard(xeDataPath);

  auto iniFiles = FileManager::ReadIniFiles(
      resolvedVersion, config.imageType, config.consoleModel, fwDir);
  if (!iniFiles) {
    return std::unexpected(
        "Failed to locate or parse INI configuration for '_" +
        config.imageType + ".ini' (section '[" + config.consoleModel + "]')");
  }

  Input input{};
  input.metadata = std::move(metadata);
  input.bootloaders = std::move(iniFiles->bootloaders);
  input.flashfs_sec = std::move(iniFiles->flashfs_sec);

  if (config.customSmcPath && std::filesystem::exists(*config.customSmcPath)) {
    if (auto smcData = Utils::read_file(*config.customSmcPath)) {
      if (input.flashfs_sec) {
        bool replaced = false;
        for (auto &entry : *input.flashfs_sec) {
          if (entry.first == "smc.bin") {
            entry.second = *smcData;
            replaced = true;
            break;
          }
        }
        if (!replaced) {
          input.flashfs_sec->push_back({"smc.bin", *smcData});
        }
      }
    }
  }

  std::string consoleLower = config.consoleModel;
  std::transform(consoleLower.begin(), consoleLower.end(), consoleLower.begin(),
                 [](unsigned char c) {
                   return static_cast<char>(std::tolower(c));
                 });

  if (consoleLower.find("corona") != std::string::npos &&
      (consoleLower.find("4g") != std::string::npos ||
       optionsMgr.get_bool("nandmu").value_or(false))) {
    input.overrides.ksb_image = true;
  } else if (consoleLower.find("jasper") != std::string::npos ||
             consoleLower.find("trinity") != std::string::npos) {
    input.overrides.psb_image = true;
  } else {
    input.overrides.xsb_image = true;
  }

  if (progressCb) {
    progressCb(BuilderProgressInfo{
        .percentage = 60,
        .statusMessage = "Assembling NAND image in-process with gxbuild3..."});
  }

  auto builtImage = GxBuild::RunBuild(input);
  if (!builtImage) {
    return std::unexpected(
        "gxbuild3 assembly error: failed to assemble NAND image.");
  }

  std::string outPath = config.outputPath;
  if (outPath.empty()) {
    QString outDir = QDir(StartupManager::instance().appDataPath())
                         .filePath(QStringLiteral("output"));
    QDir().mkpath(outDir);
    outPath =
        QDir(outDir).filePath(QStringLiteral("updflash.bin")).toStdString();
  }

  std::filesystem::path outFsPath(outPath);
  if (outFsPath.has_parent_path()) {
    std::filesystem::create_directories(outFsPath.parent_path());
  }

  if (!Utils::write_file(outFsPath, *builtImage)) {
    return std::unexpected("Failed to open output file for writing: " + outPath);
  }

  if (progressCb) {
    progressCb(BuilderProgressInfo{
        .percentage = 100,
        .statusMessage = "NAND image assembled natively with gxbuild3!"});
  }

  BuildResult bResult;
  bResult.success = true;
  bResult.outputPath = outPath;
  bResult.logOutput = "Successfully built NAND image: " + outPath + " (" +
                      std::to_string(builtImage->size()) + " bytes)";
  return bResult;
}

} 
