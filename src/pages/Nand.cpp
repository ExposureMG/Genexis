#include "pages/Nand.hpp"
#include "FlashImage.hpp"
#include "bootloaders/2bl.hpp"
#include "bootloaders/6bl.hpp"
#include "bootloaders/Keyvault.hpp"
#include "bootloaders/SMC.hpp"

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QUrl>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace {

static std::vector<uint8_t> hexToBytes(const QString &hexStr)
{
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

static QString bytesToHex(const uint8_t *data, size_t size)
{
    std::ostringstream ss;
    for (size_t i = 0; i < size; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]);
    }
    return QString::fromStdString(ss.str()).toUpper();
}

} // namespace

Nand::Nand(QObject *parent)
    : QObject(parent)
{
}

Nand &Nand::instance()
{
    static Nand inst;
    return inst;
}

void Nand::clear()
{
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
    m_payloadIndicator.clear();
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

void Nand::openFile(const QString &filePath, const QString &cpuKey)
{
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
        // Timing files keep all NAND metadata greyed out
        qDebug() << "[Nand] Opened timing file (.svf/.xsvf) -> keeping NAND metadata empty/greyed.";
        return;
    }

    if (ext != QStringLiteral("bin") && ext != QStringLiteral("ecc")) {
        qDebug() << "[Nand] Unsupported file extension for NAND info:" << ext;
        return;
    }

    std::ifstream file(cleanPath.toStdString(), std::ios::binary);
    if (!file) {
        qDebug() << "[Nand] Failed to open NAND file:" << cleanPath;
        return;
    }

    m_rawNandData = std::vector<uint8_t>((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    if (m_rawNandData.empty()) {
        qDebug() << "[Nand] NAND file is empty:" << cleanPath;
        return;
    }

    m_loadedFilePath = cleanPath;
    Q_EMIT loadedFilePathChanged();

    parseNandData(m_rawNandData);

    QString keyToUse = cpuKey.trimmed();
    if (keyToUse.isEmpty()) {
        keyToUse = detectCpuKey(cleanPath);
    }

    if (!keyToUse.isEmpty()) {
        setCpuKey(keyToUse);
    }
}

QString Nand::detectCpuKey(const QString &filePath)
{
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
        if (entry.fileName().compare(QStringLiteral("cpukey.txt"), Qt::CaseInsensitive) == 0) {
            QFile keyFile(entry.absoluteFilePath());
            if (keyFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QString content = QString::fromUtf8(keyFile.readAll()).trimmed();
                keyFile.close();

                static const QRegularExpression hex32Reg(QStringLiteral("[0-9a-fA-F]{32}"));
                QRegularExpressionMatch match = hex32Reg.match(content);
                if (match.hasMatch()) {
                    QString key = match.captured(0).toUpper();
                    qDebug() << "[Nand] Automatically detected CPU key from" << entry.fileName() << ":" << key;
                    return key;
                }
            }
        }
    }

    return QString();
}

void Nand::parseNandData(const std::vector<uint8_t> &data)
{
    nand_results_t results = read(data);
    if (!results.valid) {
        qDebug() << "[Nand] Invalid NAND image format or header.";
        return;
    }

    m_isNandLoaded = true;
    Q_EMIT nandStateChanged();

    // Image size formatting
    double sizeMb = static_cast<double>(data.size()) / (1024.0 * 1024.0);
    m_imageSize = QString::number(sizeMb, 'f', 0) + QStringLiteral(" MB");

    // Header & Bootloaders
    if (results.cbb) {
        m_cbAVersion = QString::number(results.cb_or_a.version);
        m_cbBVersion = QString::number(results.cbb->version);
        m_cbVersion = QStringLiteral("%1 / %2").arg(m_cbAVersion, m_cbBVersion);
        if (results.cbx) {
            m_cbXVersion = QString::number(results.cbx->version);
        }
    } else if (results.cb_or_a.version != 0) {
        m_cbVersion = QString::number(results.cb_or_a.version);
        m_cbAVersion = m_cbVersion;
    }

    m_cbSize = QString::number(results.cb_or_a.size);
    m_cbMagic = QStringLiteral("0x") + QString::number(results.cb_or_a.magic, 16).toUpper();

    if (results.sc)  m_scVersion = QString::number(results.sc->version);
    if (results.cd)  m_cdVersion = QString::number(results.cd->version);
    if (results.ce)  m_ceVersion = QString::number(results.ce->version);

    if (results.cf0) m_cf0Version = QString::number(results.cf0->version);
    if (results.cg0) m_cg0Version = QString::number(results.cg0->version);
    if (results.cf1) m_cf1Version = QString::number(results.cf1->version);
    if (results.cg1) m_cg1Version = QString::number(results.cg1->version);

    // Extract LDV & Pairing Data for 2BL / CB_A
    if (results.cb_or_a.offset != 0 && results.cb_or_a.offset + results.cb_or_a.size <= data.size()) {
        std::vector<uint8_t> cbBytes(data.begin() + results.cb_or_a.offset, data.begin() + results.cb_or_a.offset + results.cb_or_a.size);
        try {
            BootloaderCb cbA = BootloaderCb::parse(cbBytes);
            cbA.decrypt(key_1bl);
            cbA.populate_metadata();
            if (cbA.metadata && cbA.metadata->lockdown_value) {
                m_cbALdv = QString::number(*cbA.metadata->lockdown_value);
                m_cbLdv = m_cbALdv;
            }
            if (cbA.metadata && cbA.metadata->pairing_data) {
                auto p = *cbA.metadata->pairing_data;
                m_cbAPairing = QStringLiteral("0x") + bytesToHex(p.data(), 3).toUpper();
                m_cbPairing = m_cbAPairing;
            }
        } catch (...) {}
    }

    // Extract LDV & Pairing Data for 6BL 0 (CF0) and 6BL 1 (CF1)
    if (results.cf0 && results.cf0->offset + results.cf0->size <= data.size()) {
        std::vector<uint8_t> cfBytes(data.begin() + results.cf0->offset, data.begin() + results.cf0->offset + results.cf0->size);
        try {
            BootloaderCf cf0 = BootloaderCf::parse(cfBytes);
            m_cf0Ldv = QString::number(cf0.header.lockdown_value);
            m_cf0Pairing = QStringLiteral("0x") + bytesToHex(cf0.header.pairing_data, 3).toUpper();
        } catch (...) {}
    }

    if (results.cf1 && results.cf1->offset + results.cf1->size <= data.size()) {
        std::vector<uint8_t> cfBytes(data.begin() + results.cf1->offset, data.begin() + results.cf1->offset + results.cf1->size);
        try {
            BootloaderCf cf1 = BootloaderCf::parse(cfBytes);
            m_cf1Ldv = QString::number(cf1.header.lockdown_value);
            m_cf1Pairing = QStringLiteral("0x") + bytesToHex(cf1.header.pairing_data, 3).toUpper();
        } catch (...) {}
    }

    m_payloadIndicator = QStringLiteral("0x") + QString::number(results.payload_indicator, 16).toUpper();
    m_patchSlots = QString::number(results.patch_slots);
    m_smcConfigOffset = QStringLiteral("0x") + QString::number(results.smc_config_offset, 16).toUpper();
    m_smcSize = QString::number(results.smc_size) + QStringLiteral(" bytes");

    // Decrypt SMC Firmware
    if (results.smc_offset != 0 && results.smc_size != 0 && (results.smc_offset + results.smc_size) <= data.size()) {
        std::vector<uint8_t> rawSmc(data.begin() + results.smc_offset, data.begin() + results.smc_offset + results.smc_size);
        std::vector<uint8_t> decSmc = rawSmc;
        if (smc_is_encrypted(rawSmc)) {
            decSmc = smc_decrypt(rawSmc);
        }

        SmcType type = smc_get_type(decSmc);
        m_smcType = QString::fromStdString(std::string(smc_type_name(type)));

        if (decSmc.size() >= 0x10) {
            uint8_t vMajor = decSmc[0x01];
            uint8_t vMinor = decSmc[0x02];
            m_smcVersion = QStringLiteral("%1.%2").arg(vMajor).arg(vMinor);
        }

        m_isSmcDecrypted = true;
        Q_EMIT smcStateChanged();
    }

    Q_EMIT metadataChanged();
    qDebug() << "[Nand] NAND metadata successfully loaded. CB Version:" << m_cbVersion << "SMC Type:" << m_smcType;

    // Dynamically discover and populate components list
    m_components.clear();

    if (results.smc_offset != 0) {
        QVariantMap smcMap;
        smcMap[QStringLiteral("cardType")] = QStringLiteral("smc");
        smcMap[QStringLiteral("title")] = QStringLiteral("SMC Firmware");
        smcMap[QStringLiteral("versionStr")] = m_smcVersion.isEmpty() ? QStringLiteral("Clean") : m_smcVersion;
        smcMap[QStringLiteral("sizeStr")] = m_smcSize;
        m_components.append(smcMap);
    }

    if (results.cbb) {
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

        if (results.cbx) {
            QVariantMap cbXMap;
            cbXMap[QStringLiteral("cardType")] = QStringLiteral("cb_x");
            cbXMap[QStringLiteral("title")] = QStringLiteral("CB_X (RGH3)");
            cbXMap[QStringLiteral("versionStr")] = m_cbXVersion;
            m_components.append(cbXMap);
        }
    } else if (results.cb_or_a.version != 0) {
        QVariantMap cbMap;
        cbMap[QStringLiteral("cardType")] = QStringLiteral("cb");
        cbMap[QStringLiteral("title")] = QStringLiteral("CB (2BL)");
        cbMap[QStringLiteral("versionStr")] = m_cbVersion;
        cbMap[QStringLiteral("sizeStr")] = QString(m_cbSize + QStringLiteral(" bytes"));
        m_components.append(cbMap);
    }

    if (results.cd) {
        QVariantMap cdMap;
        cdMap[QStringLiteral("cardType")] = QStringLiteral("cd");
        cdMap[QStringLiteral("title")] = QStringLiteral("CD (4BL)");
        cdMap[QStringLiteral("versionStr")] = m_cdVersion;
        m_components.append(cdMap);
    }

    if (results.ce) {
        QVariantMap ceMap;
        ceMap[QStringLiteral("cardType")] = QStringLiteral("ce");
        ceMap[QStringLiteral("title")] = QStringLiteral("CE (Kernel)");
        ceMap[QStringLiteral("versionStr")] = m_ceVersion;
        m_components.append(ceMap);
    }

    if (results.payload_indicator != 0) {
        QVariantMap xellMap;
        xellMap[QStringLiteral("cardType")] = QStringLiteral("xell");
        xellMap[QStringLiteral("title")] = QStringLiteral("XeLL Payload");
        xellMap[QStringLiteral("versionStr")] = QStringLiteral("0.99");
        m_components.append(xellMap);
    }

    if (results.cf0) {
        QVariantMap patch0Map;
        patch0Map[QStringLiteral("cardType")] = QStringLiteral("patch0");
        patch0Map[QStringLiteral("title")] = QStringLiteral("Patchslot 0");
        patch0Map[QStringLiteral("versionStr")] = m_cf0Version;
        m_components.append(patch0Map);
    }

    if (results.cf1) {
        QVariantMap patch1Map;
        patch1Map[QStringLiteral("cardType")] = QStringLiteral("patch1");
        patch1Map[QStringLiteral("title")] = QStringLiteral("Patchslot 1");
        patch1Map[QStringLiteral("versionStr")] = m_cf1Version;
        m_components.append(patch1Map);
    }

    if (results.kv_offset != 0) {
        QVariantMap kvMap;
        kvMap[QStringLiteral("cardType")] = QStringLiteral("keyvault");
        kvMap[QStringLiteral("title")] = QStringLiteral("Keyvault");
        kvMap[QStringLiteral("versionStr")] = m_serialNumber.isEmpty() ? QStringLiteral("Encrypted") : m_serialNumber;
        m_components.append(kvMap);
    }

    Q_EMIT componentsChanged();
}

void Nand::setCpuKey(const QString &cpuKey)
{
    m_cpuKey = cpuKey.trimmed();
    Q_EMIT cpuKeyChanged();

    if (m_cpuKey.length() != 32) {
        m_isCpuKeyLoaded = false;
        Q_EMIT cpuKeyStateChanged();
        return;
    }

    std::vector<uint8_t> keyBytes = hexToBytes(m_cpuKey);
    if (!cpukey_valid(keyBytes)) {
        qDebug() << "[Nand] Invalid CPU key checksum.";
        m_isCpuKeyLoaded = false;
        Q_EMIT cpuKeyStateChanged();
        return;
    }

    if (m_rawNandData.empty()) {
        return;
    }

    FlashImage image = FlashImage::parse(m_rawNandData);
    if (image.keyvault) {
        parseKeyvault(keyBytes, *image.keyvault);
    }
}

void Nand::parseKeyvault(const std::vector<uint8_t> &cpuKeyBytes, const std::vector<uint8_t> &rawKv)
{
    std::vector<uint8_t> decKv = keyvault_decrypt(cpuKeyBytes, rawKv);
    if (decKv.size() < sizeof(XE_KEYVAULT_DATA)) {
        qDebug() << "[Nand] Keyvault decryption failed (buffer size mismatch).";
        m_isCpuKeyLoaded = false;
        Q_EMIT cpuKeyStateChanged();
        return;
    }

    const auto *kv = reinterpret_cast<const XE_KEYVAULT_DATA *>(decKv.data());

    char serialBuf[13] = {0};
    std::memcpy(serialBuf, kv->sz14ConsoleSerialNumber, 12);
    m_serialNumber = QString::fromLatin1(serialBuf);

    m_consoleId = bytesToHex(kv->b36ConsoleCertificate.ConsoleId, 5);
    m_dvdKey = bytesToHex(kv->b1ADvdKey, 16);
    m_gameRegion = QStringLiteral("0x") + QString::number(kv->w16GameRegion, 16).rightJustified(4, u'0').toUpper();
    m_consoleType = QString::number(kv->b36ConsoleCertificate.ConsoleType);

    m_isCpuKeyLoaded = true;
    Q_EMIT cpuKeyStateChanged();
    Q_EMIT metadataChanged();

    for (auto &compVal : m_components) {
        QVariantMap compMap = compVal.toMap();
        if (compMap[QStringLiteral("cardType")].toString() == QStringLiteral("keyvault")) {
            compMap[QStringLiteral("versionStr")] = m_serialNumber;
            compVal = compMap;
            break;
        }
    }
    Q_EMIT componentsChanged();

    qDebug() << "[Nand] Keyvault decrypted successfully. Serial:" << m_serialNumber << "DVD Key:" << m_dvdKey;
}
