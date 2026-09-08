#include "pages/NandBuilderController.hpp"

#include "NandBuildConfigMapper.hpp"
#include "StartupManager.hpp"
#include "backend/BackendManager.hpp"
#include "pages/Nand.hpp"

#include <QDir>
#include <thread>

using BackendManager = gxapi::backend::BackendManager;

NandBuilderController::NandBuilderController(QObject *parent)
    : QObject(parent) {
  refresh();
}

NandBuilderController &NandBuilderController::instance() {
  static NandBuilderController inst;
  return inst;
}

void NandBuilderController::refresh() {
  scanBuildVersions();
  scanXellHacks();
  scanPatches();
  scanSmc();
}

void NandBuilderController::setSelectedVersion(const QString &version) {
  if (m_selectedVersion == version) {
    return;
  }
  m_selectedVersion = version;
  Q_EMIT selectedVersionChanged();
  scanImageTypes();
  scanPatches();
}

void NandBuilderController::setSelectedImageType(const QString &imageType) {
  if (m_selectedImageType == imageType) {
    return;
  }
  m_selectedImageType = imageType;
  Q_EMIT selectedImageTypeChanged();
  scanConsoles();
}

void NandBuilderController::setSelectedConsole(const QString &console) {
  if (m_selectedConsole == console) {
    return;
  }
  m_selectedConsole = console;
  Q_EMIT selectedConsoleChanged();
  scanSmc();
}

void NandBuilderController::setSelectedSmc(const QString &smc) {
  if (m_selectedSmc == smc) {
    return;
  }
  m_selectedSmc = smc;
  Q_EMIT selectedSmcChanged();
}

void NandBuilderController::setSelectedXellHack(const QString &hack) {
  if (m_selectedXellHack == hack) {
    return;
  }
  m_selectedXellHack = hack;
  Q_EMIT selectedXellHackChanged();
  scanXellImages();
}

void NandBuilderController::setSelectedXellImage(const QString &image) {
  if (m_selectedXellImage == image) {
    return;
  }
  m_selectedXellImage = image;
  Q_EMIT selectedXellImageChanged();
}

void NandBuilderController::setSelectedSimpleVersion(const QString &version) {
  if (m_selectedSimpleVersion == version) {
    return;
  }
  m_selectedSimpleVersion = version;
  Q_EMIT selectedSimpleVersionChanged();
  scanSimpleImageTypes();
  scanPatches();
}

void NandBuilderController::setSelectedSimpleImageType(const QString &imageType) {
  if (m_selectedSimpleImageType == imageType) {
    return;
  }
  m_selectedSimpleImageType = imageType;
  Q_EMIT selectedSimpleImageTypeChanged();
  scanSimpleHacks();
}

void NandBuilderController::setSelectedSimpleHack(const QString &hack) {
  if (m_selectedSimpleHack == hack) {
    return;
  }
  m_selectedSimpleHack = hack;
  Q_EMIT selectedSimpleHackChanged();
}

QString NandBuilderController::getResolvedVersion() const {
  if (m_selectedSimpleVersion.isEmpty() ||
      m_selectedSimpleVersion == QStringLiteral("Latest")) {
    return m_buildVersions.isEmpty() ? QString() : m_buildVersions.first();
  }
  return m_selectedSimpleVersion;
}

QString NandBuilderController::getMappedUnderlyingImageType() const {
  std::string mapped = BackendManager::instance().builder().resolveUnderlyingImageType(
      m_selectedSimpleImageType.toStdString(), m_selectedSimpleHack.toStdString());
  return QString::fromStdString(mapped);
}

void NandBuilderController::scanBuildVersions() {
  m_buildVersions.clear();

  auto versions = BackendManager::instance().builder().getAvailableVersions();
  for (const auto &v : versions) {
    m_buildVersions.append(QString::fromStdString(v));
  }

  Q_EMIT buildVersionsChanged();

  if (!m_buildVersions.contains(m_selectedVersion)) {
    m_selectedVersion =
        m_buildVersions.isEmpty() ? QString() : m_buildVersions.first();
    Q_EMIT selectedVersionChanged();
  }

  scanImageTypes();
  scanSimpleVersions();
}

void NandBuilderController::scanImageTypes() {
  m_imageTypes.clear();

  if (!m_selectedVersion.isEmpty()) {
    auto types = BackendManager::instance().builder().getAvailableImageTypes(
        m_selectedVersion.toStdString());
    for (const auto &t : types) {
      m_imageTypes.append(QString::fromStdString(t));
    }
  }

  Q_EMIT imageTypesChanged();

  if (!m_imageTypes.contains(m_selectedImageType)) {
    m_selectedImageType =
        m_imageTypes.isEmpty() ? QString() : m_imageTypes.first();
    Q_EMIT selectedImageTypeChanged();
  }

  scanConsoles();
}

void NandBuilderController::scanConsoles() {
  m_consoles.clear();

  if (!m_selectedVersion.isEmpty() && !m_selectedImageType.isEmpty()) {
    auto consoles = BackendManager::instance().builder().getAvailableConsoles(
        m_selectedVersion.toStdString(), m_selectedImageType.toStdString());
    for (const auto &c : consoles) {
      m_consoles.append(QString::fromStdString(c));
    }
  }

  Q_EMIT consolesChanged();

  if (!m_consoles.contains(m_selectedConsole)) {
    m_selectedConsole = m_consoles.isEmpty() ? QString() : m_consoles.first();
    Q_EMIT selectedConsoleChanged();
  }

  scanSmc();
}

void NandBuilderController::scanXellHacks() {
  m_xellHacks.clear();

  auto hacks = BackendManager::instance().builder().getXellHacks();
  for (const auto &h : hacks) {
    m_xellHacks.append(QString::fromStdString(h));
  }

  Q_EMIT xellHacksChanged();

  if (!m_xellHacks.contains(m_selectedXellHack)) {
    m_selectedXellHack =
        m_xellHacks.isEmpty() ? QString() : m_xellHacks.first();
    Q_EMIT selectedXellHackChanged();
  }

  scanXellImages();
}

void NandBuilderController::scanXellImages() {
  m_xellImages.clear();

  if (!m_selectedXellHack.isEmpty()) {
    auto images = BackendManager::instance().builder().getXellImages(
        m_selectedXellHack.toStdString());
    for (const auto &img : images) {
      m_xellImages.append(QString::fromStdString(img));
    }
  }

  Q_EMIT xellImagesChanged();

  if (!m_xellImages.contains(m_selectedXellImage)) {
    m_selectedXellImage =
        m_xellImages.isEmpty() ? QString() : m_xellImages.first();
    Q_EMIT selectedXellImageChanged();
  }
}

void NandBuilderController::scanSimpleVersions() {
  m_simpleVersions.clear();

  auto versions = BackendManager::instance().builder().getSimpleVersions();
  for (const auto &ver : versions) {
    m_simpleVersions.append(QString::fromStdString(ver));
  }

  Q_EMIT simpleVersionsChanged();

  if (!m_simpleVersions.contains(m_selectedSimpleVersion)) {
    m_selectedSimpleVersion = QStringLiteral("Latest");
    Q_EMIT selectedSimpleVersionChanged();
  }

  scanSimpleImageTypes();
}

void NandBuilderController::scanSimpleImageTypes() {
  m_simpleImageTypes.clear();

  auto types = BackendManager::instance().builder().getSimpleImageTypes(
      m_selectedSimpleVersion.toStdString());
  for (const auto &t : types) {
    m_simpleImageTypes.append(QString::fromStdString(t));
  }

  Q_EMIT simpleImageTypesChanged();

  if (!m_simpleImageTypes.contains(m_selectedSimpleImageType)) {
    m_selectedSimpleImageType =
        m_simpleImageTypes.contains(QStringLiteral("FreeBoot"))
            ? QStringLiteral("FreeBoot")
            : (m_simpleImageTypes.isEmpty() ? QString()
                                             : m_simpleImageTypes.first());
    Q_EMIT selectedSimpleImageTypeChanged();
  }

  scanSimpleHacks();
}

void NandBuilderController::scanSimpleHacks() {
  m_simpleHacks.clear();

  auto hacks = BackendManager::instance().builder().getSimpleHacks(
      m_selectedSimpleVersion.toStdString(), m_selectedSimpleImageType.toStdString());
  for (const auto &h : hacks) {
    m_simpleHacks.append(QString::fromStdString(h));
  }

  Q_EMIT simpleHacksChanged();

  if (!m_simpleHacks.contains(m_selectedSimpleHack)) {
    m_selectedSimpleHack =
        m_simpleHacks.isEmpty() ? QString() : m_simpleHacks.first();
    Q_EMIT selectedSimpleHackChanged();
  }
}

void NandBuilderController::scanPatches() {
  m_availablePatches.clear();

  QString verToScan = getResolvedVersion();
  if (verToScan.isEmpty()) {
    verToScan = m_selectedVersion;
  }

  if (!verToScan.isEmpty()) {
    auto patches = BackendManager::instance().builder().getAvailablePatches(
        verToScan.toStdString());
    for (const auto &p : patches) {
      m_availablePatches.append(QString::fromStdString(p));
    }
  }

  Q_EMIT availablePatchesChanged();
}

void NandBuilderController::scanSmc() {
  m_smcFiles.clear();

  if (!m_selectedConsole.isEmpty()) {
    auto smcs = BackendManager::instance().builder().getAvailableSmcFiles(
        m_selectedConsole.toStdString());
    for (const auto &s : smcs) {
      m_smcFiles.append(QString::fromStdString(s));
    }
  }

  Q_EMIT smcFilesChanged();

  if (!m_smcFiles.contains(m_selectedSmc)) {
    m_selectedSmc = m_smcFiles.isEmpty() ? QString() : m_smcFiles.first();
    Q_EMIT selectedSmcChanged();
  }
}

void NandBuilderController::buildImage(const QVariantMap &config) {
  Q_EMIT buildStarted();
  Q_EMIT buildProgress(5, QStringLiteral("Validating build parameters..."));

  const QString appDataPath = StartupManager::instance().appDataPath();
  const QString outputPath =
      QDir(appDataPath).filePath(QStringLiteral("output/updflash.bin"));
  const gxapi::pages::detail::NandBuildConfigDefaults defaults{
      .cpuKeyHex = Nand::instance().cpuKey().trimmed(),
      .version = getResolvedVersion(),
      .imageType = getMappedUnderlyingImageType(),
      .consoleModel = m_selectedConsole,
      .outputPath = outputPath,
      .smcSelection = m_selectedSmc,
      .smcDataRoot =
          QDir(appDataPath).filePath(QStringLiteral("data/nand/smc"))};
  const gxapi::backend::NandBuildConfig bConfig =
      gxapi::pages::detail::normalizeNandBuildConfig(config, defaults);

  if (!bConfig.xellOnly && bConfig.cpuKeyHex.empty()) {
    QString err = QStringLiteral("CPU key must be provided.");
    Q_EMIT buildProgress(100, err);
    Q_EMIT buildFinished(false, QString(), err);
    return;
  }

  std::thread worker([this, bConfig]() {
    auto progressCb = [this](const gxapi::backend::BuilderProgressInfo &info) {
      Q_EMIT buildProgress(info.percentage, QString::fromStdString(info.statusMessage));
    };

    auto res = BackendManager::instance().builder().buildImage(bConfig, progressCb);
    if (res.has_value()) {
      Q_EMIT buildProgress(100, QStringLiteral("NAND image assembled successfully!"));
      Q_EMIT buildFinished(true, QString::fromStdString(res->outputPath),
                           QString::fromStdString(res->logOutput));
    } else {
      QString err = QStringLiteral("Build failed: ") + QString::fromStdString(res.error());
      Q_EMIT buildProgress(100, err);
      Q_EMIT buildFinished(false, QString(), err);
    }
  });
  worker.detach();
}
