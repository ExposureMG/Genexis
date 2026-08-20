#include "pages/Nand.hpp"
#include "nand/FlashImage.hpp"
#include "StartupManager.hpp"
#include "nand/bootloaders/2bl.hpp"
#include "nand/bootloaders/6bl.hpp"
#include "nand/objects/Keyvault.hpp"
#include "nand/objects/SMC.hpp"

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QThreadPool>
#include <QUrl>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace {

static std::vector<uint8_t> hexToBytes(const QString &hexStr) {
  QString clean = hexStr.trimmed();
  std::vector<uint8_t> bytes;
  for (int i = 0; i < clean.length(); i += 2) {
    bool ok = false;
    uint8_t byte = static_cast<uint8_t>(clean.mid(i, 2).toUInt(&ok, 16));
    if (ok) {
      bytes.push_back(byte);
    }
  }
  return bytes;
}

static QString bytesToHex(const uint8_t *data, size_t size) {
  std::ostringstream ss;
  for (size_t i = 0; i < size; ++i) {
    ss << std::hex << std::setw(2) << std::setfill('0')
       << static_cast<int>(data[i]);
  }
  return QString::fromStdString(ss.str()).toUpper();
}

} 

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
  auto imageOpt = gxbuild3::NAND::FlashImage::read(data);
  if (!imageOpt || !imageOpt->parse()) {
    qDebug() << "[Nand] Invalid NAND image format or header.";
    return;
  }
  auto &image = *imageOpt;

  m_isNandLoaded = true;
  Q_EMIT nandStateChanged();

  double sizeMb = static_cast<double>(data.size()) / (1024.0 * 1024.0);
  m_imageSize = QString::number(sizeMb, 'f', 0) + QStringLiteral(" MB");

  m_headerMagic = QStringLiteral("0x") +
                  QString::number(image.header.magic, 16).toUpper();
  m_headerVersion = QString::number(image.header.version);
  m_patchSlots = QString::number(image.header.patch_slots);
  m_smcConfigOffset = QStringLiteral("0x") +
                      QString::number(image.header.smc_config_offset, 16).toUpper();

  if (image.cb_section.cb_B.has_value() && !image.cb_section.cb_B->data.empty()) {
    m_cbAVersion = QString::number(image.cb_section.cb_or_A.header.header.version);
    m_cbBVersion = QString::number(image.cb_section.cb_B->header.header.version);
    m_cbVersion = QStringLiteral("%1 / %2").arg(m_cbAVersion, m_cbBVersion);
    if (image.cb_section.cb_x.has_value()) {
      m_cbXVersion = QString::number(image.cb_section.cb_x->header.header.version);
    }
  } else if (image.cb_section.cb_or_A.header.header.version != 0) {
    m_cbVersion = QString::number(image.cb_section.cb_or_A.header.header.version);
    m_cbAVersion = m_cbVersion;
  }

  m_cbSize = QString::number(image.cb_section.cb_or_A.header.header.size);
  m_cbMagic = QStringLiteral("0x") +
              QString::number(image.cb_section.cb_or_A.header.header.magic, 16).toUpper();

  if (image.cb_section.sc.has_value())
    m_scVersion = QString::number(image.cb_section.sc->header.header.version);
  if (!image.kernel_section.cd.data.empty())
    m_cdVersion = QString::number(image.kernel_section.cd.header.header.version);
  if (image.kernel_section.ce.has_value())
    m_ceVersion = QString::number(image.kernel_section.ce->header.header.version);

  if (image.system_update_0.cf.has_value())
    m_cf0Version = QString::number(image.system_update_0.cf->header.header.version);
  if (image.system_update_0.cg.has_value())
    m_cg0Version = QString::number(image.system_update_0.cg->header.header.version);
  if (image.system_update_1.cf.has_value())
    m_cf1Version = QString::number(image.system_update_1.cf->header.header.version);
  if (image.system_update_1.cg.has_value())
    m_cg1Version = QString::number(image.system_update_1.cg->header.header.version);

  if (!image.cb_section.cb_or_A.data.empty()) {
    try {
      BootloaderCb cbA = image.cb_section.cb_or_A;
      if (!cbA.is_decrypted()) {
        cbA.decrypt(key_1bl);
      }
      cbA.populate_metadata();
      if (cbA.perbox.has_value()) {
        m_cbALdv = QString::number(cbA.perbox->lockdown_value);
        m_cbLdv = m_cbALdv;
        m_cbAPairing = QStringLiteral("0x") + bytesToHex(cbA.perbox->pairing_data, 3).toUpper();
        m_cbPairing = m_cbAPairing;
      }
    } catch (...) {
    }
  }

  if (image.system_update_0.cf.has_value()) {
    try {
      BootloaderCf cf0 = *image.system_update_0.cf;
      if (!cf0.is_decrypted()) {
        cf0.decrypt(key_1bl);
      }
      if (cf0.perbox.has_value()) {
        m_cf0Ldv = QString::number(cf0.perbox->lockdown_value);
        m_cf0Pairing = QStringLiteral("0x") +
                       bytesToHex(cf0.perbox->pairing_data, 3).toUpper();
      }
    } catch (...) {
    }
  }

  if (image.system_update_1.cf.has_value()) {
    try {
      BootloaderCf cf1 = *image.system_update_1.cf;
      if (!cf1.is_decrypted()) {
        cf1.decrypt(key_1bl);
      }
      if (cf1.perbox.has_value()) {
        m_cf1Ldv = QString::number(cf1.perbox->lockdown_value);
        m_cf1Pairing = QStringLiteral("0x") +
                       bytesToHex(cf1.perbox->pairing_data, 3).toUpper();
      }
    } catch (...) {
    }
  }

  if (image.smc.has_value() && !image.smc->data.empty()) {
    m_smcSize = QString::number(image.smc->data.size()) + QStringLiteral(" bytes");

    if (!image.smc->version.empty()) {
      m_smcVersion = QString::fromStdString(image.smc->version);
    } else {
      std::vector<uint8_t> decSmc = image.smc->data;
      if (gxbuild3::NAND::smc_is_encrypted(decSmc)) {
        decSmc = gxbuild3::NAND::smc_decrypt(decSmc);
      }
      if (decSmc.size() >= 0x102) {
        uint8_t b0 = decSmc[0x100];
        uint8_t b1 = decSmc[0x101];
        uint8_t major = (b0 >> 4) & 0xF;
        uint8_t minor = b0 & 0xF;
        m_smcVersion = QStringLiteral("%1.%2").arg(major).arg(minor);
        if (b1 != 0) {
          m_smcVersion += QStringLiteral(".%1").arg(b1);
        }
      }
    }

    auto variantName = gxbuild3::NAND::smc_type_name(image.smc->variant);
    auto mbName = gxbuild3::NAND::smc_motherboard_name(image.smc->motherboard);
    if (variantName != "Unknown" && mbName != "Unknown") {
      m_smcType = QStringLiteral("%1 (%2)")
                      .arg(QString::fromStdString(std::string(variantName)),
                           QString::fromStdString(std::string(mbName)));
    } else if (variantName != "Unknown") {
      m_smcType = QString::fromStdString(std::string(variantName));
    } else if (mbName != "Unknown") {
      m_smcType = QString::fromStdString(std::string(mbName));
    } else {
      std::vector<uint8_t> decSmc = image.smc->data;
      if (gxbuild3::NAND::smc_is_encrypted(decSmc)) {
        decSmc = gxbuild3::NAND::smc_decrypt(decSmc);
      }
      gxbuild3::NAND::SmcType type = gxbuild3::NAND::smc_get_type(decSmc);
      m_smcType = QString::fromStdString(std::string(gxbuild3::NAND::smc_type_name(type)));
    }

    if (image.smc->motherboard != gxbuild3::NAND::SmcMotherboard::Unknown) {
      m_consoleTarget = QString::fromStdString(
          std::string(gxbuild3::NAND::smc_motherboard_name(image.smc->motherboard)));
    }

    m_isSmcDecrypted = true;
    Q_EMIT smcStateChanged();
  }

  Q_EMIT metadataChanged();
  qDebug() << "[Nand] NAND metadata successfully loaded. CB Version:"
           << m_cbVersion << "SMC Type:" << m_smcType;

  m_components.clear();

  if (image.smc.has_value()) {
    QVariantMap smcMap;
    smcMap[QStringLiteral("cardType")] = QStringLiteral("smc");
    smcMap[QStringLiteral("title")] = QStringLiteral("SMC Firmware");
    smcMap[QStringLiteral("versionStr")] =
        m_smcVersion.isEmpty() ? QStringLiteral("Clean") : m_smcVersion;
    smcMap[QStringLiteral("sizeStr")] = m_smcSize;
    m_components.append(smcMap);
  }

  if (image.cb_section.cb_B.has_value() && !image.cb_section.cb_B->data.empty()) {
    QVariantMap cbAMap;
    cbAMap[QStringLiteral("cardType")] = QStringLiteral("cb_a");
    cbAMap[QStringLiteral("title")] = QStringLiteral("CB_A (2BL)");
    cbAMap[QStringLiteral("versionStr")] = m_cbAVersion;
    m_components.append(cbAMap);

    QVariantMap cbBMap;
    cbBMap[QStringLiteral("cardType")] = QStringLiteral("cb_b");
    cbBMap[QStringLiteral("title")] = QStringLiteral("CB_B (2BL)");
    cbBMap[QStringLiteral("versionStr")] = m_cbBVersion;
    m_components.append(cbBMap);

    if (image.cb_section.cb_x.has_value()) {
      QVariantMap cbXMap;
      cbXMap[QStringLiteral("cardType")] = QStringLiteral("cb_x");
      cbXMap[QStringLiteral("title")] = QStringLiteral("CB_X (RGH3)");
      cbXMap[QStringLiteral("versionStr")] = m_cbXVersion;
      m_components.append(cbXMap);
    }
  } else if (image.cb_section.cb_or_A.header.header.version != 0) {
    QVariantMap cbMap;
    cbMap[QStringLiteral("cardType")] = QStringLiteral("cb");
    cbMap[QStringLiteral("title")] = QStringLiteral("CB (2BL)");
    cbMap[QStringLiteral("versionStr")] = m_cbVersion;
    cbMap[QStringLiteral("sizeStr")] =
        QString(m_cbSize + QStringLiteral(" bytes"));
    m_components.append(cbMap);
  }

  if (!image.kernel_section.cd.data.empty()) {
    QVariantMap cdMap;
    cdMap[QStringLiteral("cardType")] = QStringLiteral("cd");
    cdMap[QStringLiteral("title")] = QStringLiteral("CD (4BL)");
    cdMap[QStringLiteral("versionStr")] = m_cdVersion;
    m_components.append(cdMap);
  }

  if (image.kernel_section.ce.has_value()) {
    QVariantMap ceMap;
    ceMap[QStringLiteral("cardType")] = QStringLiteral("ce");
    ceMap[QStringLiteral("title")] = QStringLiteral("CE (Kernel)");
    ceMap[QStringLiteral("versionStr")] = m_ceVersion;
    m_components.append(ceMap);
  }

  if (image.payloads.xell.has_value()) {
    QVariantMap xellMap;
    xellMap[QStringLiteral("cardType")] = QStringLiteral("xell");
    xellMap[QStringLiteral("title")] = QStringLiteral("XeLL Payload");
    xellMap[QStringLiteral("versionStr")] =
        QString::fromStdString(image.payloads.xell->metadata.version.empty()
                                   ? "0.99"
                                   : image.payloads.xell->metadata.version);
    m_components.append(xellMap);
  }

  if (image.system_update_0.cf.has_value()) {
    QVariantMap patch0Map;
    patch0Map[QStringLiteral("cardType")] = QStringLiteral("patch0");
    patch0Map[QStringLiteral("title")] = QStringLiteral("Patchslot 0");
    patch0Map[QStringLiteral("versionStr")] = m_cf0Version;
    m_components.append(patch0Map);
  }

  if (image.system_update_1.cf.has_value()) {
    QVariantMap patch1Map;
    patch1Map[QStringLiteral("cardType")] = QStringLiteral("patch1");
    patch1Map[QStringLiteral("title")] = QStringLiteral("Patchslot 1");
    patch1Map[QStringLiteral("versionStr")] = m_cf1Version;
    m_components.append(patch1Map);
  }

  if (image.keyvault.has_value()) {
    QVariantMap kvMap;
    kvMap[QStringLiteral("cardType")] = QStringLiteral("keyvault");
    kvMap[QStringLiteral("title")] = QStringLiteral("Keyvault");
    kvMap[QStringLiteral("versionStr")] =
        m_serialNumber.isEmpty() ? QStringLiteral("Encrypted") : m_serialNumber;
    m_components.append(kvMap);
  }

  Q_EMIT componentsChanged();
}

void Nand::setCpuKey(const QString &cpuKey) {
  m_cpuKey = cpuKey.trimmed();
  Q_EMIT cpuKeyChanged();

  if (m_cpuKey.length() != 32) {
    m_isCpuKeyLoaded = false;
    Q_EMIT cpuKeyStateChanged();
    return;
  }

  std::vector<uint8_t> keyBytes = hexToBytes(m_cpuKey);
  if (!gxbuild3::NAND::cpukey_valid(keyBytes)) {
    qDebug() << "[Nand] Invalid CPU key checksum.";
    m_isCpuKeyLoaded = false;
    Q_EMIT cpuKeyStateChanged();
    return;
  }

  if (m_rawNandData.empty()) {
    return;
  }

  auto imageOpt = gxbuild3::NAND::FlashImage::read(m_rawNandData);
  if (imageOpt && imageOpt->parse() && imageOpt->keyvault.has_value()) {
    const auto &rawKv = imageOpt->keyvault->raw_data.empty()
                            ? imageOpt->keyvault->serialize()
                            : imageOpt->keyvault->raw_data;
    parseKeyvault(keyBytes, rawKv);
  }
}

void Nand::parseKeyvault(const std::vector<uint8_t> &cpuKeyBytes,
                         const std::vector<uint8_t> &rawKv) {
  std::vector<uint8_t> decKv = gxbuild3::NAND::keyvault_decrypt(cpuKeyBytes, rawKv);
  if (decKv.size() < sizeof(gxbuild3::NAND::XE_KEYVAULT_DATA)) {
    qDebug() << "[Nand] Keyvault decryption failed (buffer size mismatch).";
    m_isCpuKeyLoaded = false;
    Q_EMIT cpuKeyStateChanged();
    return;
  }

  const auto *kv =
      reinterpret_cast<const gxbuild3::NAND::XE_KEYVAULT_DATA *>(decKv.data());

  char serialBuf[13] = {0};
  std::memcpy(serialBuf, kv->sz14ConsoleSerialNumber, 12);
  m_serialNumber = QString::fromLatin1(serialBuf);

  m_consoleId = bytesToHex(kv->b36ConsoleCertificate.ConsoleId, 5);
  m_dvdKey = bytesToHex(kv->b1ADvdKey, 16);
  m_gameRegion =
      QStringLiteral("0x") +
      QString::number(kv->w16GameRegion, 16).rightJustified(4, u'0').toUpper();
  m_consoleType = QString::number(kv->b36ConsoleCertificate.ConsoleType);

  m_isCpuKeyLoaded = true;
  Q_EMIT cpuKeyStateChanged();
  Q_EMIT metadataChanged();

  for (auto &compVal : m_components) {
    QVariantMap compMap = compVal.toMap();
    if (compMap[QStringLiteral("cardType")].toString() ==
        QStringLiteral("keyvault")) {
      compMap[QStringLiteral("versionStr")] = m_serialNumber;
      compVal = compMap;
      break;
    }
  }
  Q_EMIT componentsChanged();

  qDebug() << "[Nand] Keyvault decrypted successfully. Serial:"
           << m_serialNumber << "DVD Key:" << m_dvdKey;

  if (!m_rawNandData.empty()) {
    QString xeDataDir = QDir(StartupManager::instance().appDataPath())
                            .filePath(QStringLiteral("data/nand/xebuild/data"));
    QDir().mkpath(xeDataDir);
    QString dumpPath = QDir(xeDataDir).filePath(QStringLiteral("nanddump.bin"));

    QFile dumpFile(dumpPath);
    if (dumpFile.open(QIODevice::WriteOnly)) {
      dumpFile.write(reinterpret_cast<const char *>(m_rawNandData.data()),
                     m_rawNandData.size());
      dumpFile.close();
      m_loadedFilePath = dumpPath;
      Q_EMIT loadedFilePathChanged();
      qDebug() << "[Nand] Copied raw NAND dump to:" << dumpPath;
    } else {
      qDebug() << "[Nand] Failed to write nanddump.bin to:" << dumpPath;
    }
  }
}
