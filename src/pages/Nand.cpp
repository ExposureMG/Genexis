#include "pages/Nand.hpp"

#include "Library.hpp"
#include "nand/objects/Keyvault.hpp"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QThreadPool>
#include <QUrl>
#include <fstream>

Nand::Nand(QObject *parent) : QObject(parent) {}

Nand &Nand::instance() {
  static Nand inst;
  return inst;
}

void Nand::clear() {
  if (m_isLoading) {
    m_isLoading = false;
    Q_EMIT loadingChanged();
  }

  m_isNandLoaded = false;
  m_isCpuKeyLoaded = false;
  m_isSmcDecrypted = false;

  m_loadedFilePath.clear();
  m_cpuKey.clear();

  m_consoleTarget.clear();
  m_buildType.clear();
  m_blockType.clear();
  m_imageSize.clear();
  m_headerMagic.clear();
  m_headerVersion.clear();
  m_patchSlots.clear();

  m_cbVersion.clear();
  m_cbAVersion.clear();
  m_cbBVersion.clear();
  m_cbXVersion.clear();
  m_cbSize.clear();
  m_cbMagic.clear();
  m_scVersion.clear();
  m_ccVersion.clear();
  m_cdVersion.clear();
  m_ceVersion.clear();
  m_cf0Version.clear();
  m_cg0Version.clear();
  m_cf1Version.clear();
  m_cg1Version.clear();

  m_cbLdv.clear();
  m_cbPairing.clear();
  m_cbALdv.clear();
  m_cbAPairing.clear();
  m_cf0Ldv.clear();
  m_cf0Pairing.clear();
  m_cf1Ldv.clear();
  m_cf1Pairing.clear();

  m_smcVersion.clear();
  m_smcType.clear();
  m_smcConfigOffset.clear();
  m_smcSize.clear();

  m_serialNumber.clear();
  m_consoleId.clear();
  m_dvdKey.clear();
  m_gameRegion.clear();
  m_consoleType.clear();
  m_kvVersion.clear();
  m_ldvCount.clear();

  m_rawNandData.clear();
  m_components.clear();

  Q_EMIT nandStateChanged();
  Q_EMIT cpuKeyStateChanged();
  Q_EMIT smcStateChanged();
  Q_EMIT loadedFilePathChanged();
  Q_EMIT cpuKeyChanged();
  Q_EMIT metadataChanged();
  Q_EMIT componentsChanged();
}

void Nand::openFile(const QString &filePath, const QString &cpuKey) {
  clear();

  QString cleanPath = filePath;
  if (cleanPath.startsWith(QStringLiteral("file://"))) {
    cleanPath = QUrl(filePath).toLocalFile();
  }

  if (cleanPath.isEmpty()) {
    return;
  }

  QFileInfo fileInfo(cleanPath);
  QString ext = fileInfo.suffix().toLower();

  if (ext == QStringLiteral("svf") || ext == QStringLiteral("xsvf")) {
    m_loadedFilePath = cleanPath;
    Q_EMIT loadedFilePathChanged();
    
    qDebug() << "[Nand] Opened timing file (.svf/.xsvf) -> keeping NAND "
                "metadata empty/greyed.";
    return;
  }

  if (ext != QStringLiteral("bin") && ext != QStringLiteral("ecc")) {
    qDebug() << "[Nand] Unsupported file extension for NAND info:" << ext;
    return;
  }

  m_isLoading = true;
  Q_EMIT loadingChanged();

  QString keyParam = cpuKey;

  QThreadPool::globalInstance()->start([this, cleanPath, keyParam]() {
    std::ifstream file(cleanPath.toStdString(), std::ios::binary);
    if (!file) {
      qDebug() << "[Nand] Failed to open NAND file:" << cleanPath;
      QMetaObject::invokeMethod(this, [this]() {
        m_isLoading = false;
        Q_EMIT loadingChanged();
      });
      return;
    }

    std::vector<uint8_t> rawData((std::istreambuf_iterator<char>(file)),
                                 std::istreambuf_iterator<char>());
    if (rawData.empty()) {
      qDebug() << "[Nand] NAND file is empty:" << cleanPath;
      QMetaObject::invokeMethod(this, [this]() {
        m_isLoading = false;
        Q_EMIT loadingChanged();
      });
      return;
    }

    QString keyToUse = keyParam.trimmed();
    if (keyToUse.isEmpty()) {
      keyToUse = detectCpuKey(cleanPath);
    }

    QMetaObject::invokeMethod(
        this, [this, cleanPath, rawData = std::move(rawData), keyToUse]() mutable {
          m_rawNandData = std::move(rawData);
          m_loadedFilePath = cleanPath;
          Q_EMIT loadedFilePathChanged();

          parseNandData(m_rawNandData);

          if (!keyToUse.isEmpty()) {
            setCpuKey(keyToUse);
          }

          m_isLoading = false;
          Q_EMIT loadingChanged();
        });
  });
}

QString Nand::detectCpuKey(const QString &filePath) {
  QString cleanPath = filePath.trimmed();
  if (cleanPath.startsWith(QStringLiteral("file://"))) {
    cleanPath = QUrl(cleanPath).toLocalFile();
  }

  if (cleanPath.isEmpty()) {
    return QString();
  }

  QFileInfo fileInfo(cleanPath);
  QString ext = fileInfo.suffix().toLower();
  if (ext != QStringLiteral("bin") && ext != QStringLiteral("ecc")) {
    return QString();
  }

  QDir dir = fileInfo.dir();
  QFileInfoList entries = dir.entryInfoList(QDir::Files);
  for (const auto &entry : entries) {
    if (entry.fileName().compare(QStringLiteral("cpukey.txt"),
                                 Qt::CaseInsensitive) == 0) {
      QFile keyFile(entry.absoluteFilePath());
      if (keyFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString content = QString::fromUtf8(keyFile.readAll()).trimmed();
        keyFile.close();

        static const QRegularExpression hex32Reg(
            QStringLiteral("[0-9a-fA-F]{32}"));
        QRegularExpressionMatch match = hex32Reg.match(content);
        if (match.hasMatch()) {
          QString key = match.captured(0).toUpper();
          qDebug() << "[Nand] Automatically detected CPU key from"
                   << entry.fileName() << ":" << key;
          return key;
        }
      }
    }
  }

  return QString();
}

void Nand::parseNandData(const std::vector<uint8_t> &data) {
  const auto info = GxBuild::ExtractSomeInfo(data);
  if (!info) {
    qDebug() << "[Nand] Invalid NAND image format or header.";
    return;
  }
  applySnapshot(MapPublicNandInfo(*info), false, info->smc.decrypted);
}

void Nand::setCpuKey(const QString &cpuKey) {
  m_cpuKey = cpuKey.trimmed().toUpper();
  Q_EMIT cpuKeyChanged();

  static const QRegularExpression cpuKeyPattern(
      QStringLiteral("^[0-9A-F]{32}$"));
  const auto restorePublicSnapshot = [this]() {
    const auto publicInfo = GxBuild::ExtractSomeInfo(m_rawNandData);
    if (publicInfo) {
      applySnapshot(MapPublicNandInfo(*publicInfo), false,
                    publicInfo->smc.decrypted);
      return;
    }
    m_isCpuKeyLoaded = false;
    Q_EMIT cpuKeyStateChanged();
  };
  if (!cpuKeyPattern.match(m_cpuKey).hasMatch()) {
    restorePublicSnapshot();
    return;
  }

  const auto validation = validate_cpu_key_hex(m_cpuKey.toStdString());
  if (validation.status != CpuKeyStatus::Valid) {
    qDebug() << "[Nand] Invalid CPU key checksum.";
    restorePublicSnapshot();
    return;
  }

  if (m_rawNandData.empty()) {
    restorePublicSnapshot();
    return;
  }

  const auto info = GxBuild::ExtractAllInfo(m_rawNandData, validation.key);
  if (!info) {
    qDebug() << "[Nand] Keyvault decryption failed.";
    restorePublicSnapshot();
    return;
  }

  applySnapshot(MapDecryptedNandInfo(*info, currentSnapshot()), true,
                info->smc.decrypted);
}

void Nand::applySnapshot(const NandInfoSnapshot &snapshot, bool decrypted,
                         bool smcDecrypted) {
  m_imageSize = snapshot.imageSize;
  m_blockType = snapshot.blockType;
  m_consoleTarget = snapshot.consoleTarget;
  m_buildType = snapshot.buildType;
  m_headerMagic = snapshot.headerMagic;
  m_headerVersion = snapshot.headerVersion;
  m_patchSlots = snapshot.patchSlots;
  m_smcVersion = snapshot.smcVersion;
  m_smcType = snapshot.smcType;
  m_smcSize = snapshot.smcSize;
  m_smcConfigOffset = snapshot.smcConfigOffset;
  m_cbVersion = snapshot.cbVersion;
  m_cbAVersion = snapshot.cbAVersion;
  m_cbBVersion = snapshot.cbBVersion;
  m_cbXVersion = snapshot.cbXVersion;
  m_cbSize = snapshot.cbSize;
  m_cbMagic = snapshot.cbMagic;
  m_scVersion = snapshot.scVersion;
  m_ccVersion = snapshot.ccVersion;
  m_cdVersion = snapshot.cdVersion;
  m_ceVersion = snapshot.ceVersion;
  m_cf0Version = snapshot.cf0Version;
  m_cg0Version = snapshot.cg0Version;
  m_cf1Version = snapshot.cf1Version;
  m_cg1Version = snapshot.cg1Version;
  m_cbLdv = snapshot.cbLdv;
  m_cbPairing = snapshot.cbPairing;
  m_cbALdv = snapshot.cbALdv;
  m_cbAPairing = snapshot.cbAPairing;
  m_cf0Ldv = snapshot.cf0Ldv;
  m_cf0Pairing = snapshot.cf0Pairing;
  m_cf1Ldv = snapshot.cf1Ldv;
  m_cf1Pairing = snapshot.cf1Pairing;
  m_serialNumber = snapshot.serialNumber;
  m_consoleId = snapshot.consoleId;
  m_dvdKey = snapshot.dvdKey;
  m_gameRegion = snapshot.gameRegion;
  m_consoleType = snapshot.consoleType;
  m_kvVersion = snapshot.kvVersion;
  m_ldvCount = snapshot.ldvCount;
  m_components = snapshot.components;

  m_isNandLoaded = true;
  m_isCpuKeyLoaded = decrypted;
  m_isSmcDecrypted = smcDecrypted;

  Q_EMIT nandStateChanged();
  Q_EMIT cpuKeyStateChanged();
  Q_EMIT smcStateChanged();
  Q_EMIT metadataChanged();
  Q_EMIT componentsChanged();
}

NandInfoSnapshot Nand::currentSnapshot() const {
  return {
      .imageSize = m_imageSize,
      .blockType = m_blockType,
      .consoleTarget = m_consoleTarget,
      .buildType = m_buildType,
      .headerMagic = m_headerMagic,
      .headerVersion = m_headerVersion,
      .patchSlots = m_patchSlots,
      .smcVersion = m_smcVersion,
      .smcType = m_smcType,
      .smcSize = m_smcSize,
      .smcConfigOffset = m_smcConfigOffset,
      .smcDecrypted = m_isSmcDecrypted,
      .cbVersion = m_cbVersion,
      .cbAVersion = m_cbAVersion,
      .cbBVersion = m_cbBVersion,
      .cbXVersion = m_cbXVersion,
      .cbSize = m_cbSize,
      .cbMagic = m_cbMagic,
      .scVersion = m_scVersion,
      .ccVersion = m_ccVersion,
      .cdVersion = m_cdVersion,
      .ceVersion = m_ceVersion,
      .cf0Version = m_cf0Version,
      .cg0Version = m_cg0Version,
      .cf1Version = m_cf1Version,
      .cg1Version = m_cg1Version,
      .cbLdv = m_cbLdv,
      .cbPairing = m_cbPairing,
      .cbALdv = m_cbALdv,
      .cbAPairing = m_cbAPairing,
      .cf0Ldv = m_cf0Ldv,
      .cf0Pairing = m_cf0Pairing,
      .cf1Ldv = m_cf1Ldv,
      .cf1Pairing = m_cf1Pairing,
      .serialNumber = m_serialNumber,
      .consoleId = m_consoleId,
      .dvdKey = m_dvdKey,
      .gameRegion = m_gameRegion,
      .consoleType = m_consoleType,
      .kvVersion = m_kvVersion,
      .ldvCount = m_ldvCount,
      .components = m_components,
  };
}
