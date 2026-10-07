#include "pages/NandInfoMapper.hpp"

#include <array>
#include <optional>

namespace {

QString versionString(const std::optional<BootloaderEntryInfo> &entry) {
  return entry && entry->present ? QString::number(entry->version) : QString();
}

QString pairingString(const std::array<uint8_t, 3> &pairing) {
  return QStringLiteral("0x") +
         QStringLiteral("%1%2%3")
             .arg(pairing[0], 2, 16, QLatin1Char('0'))
             .arg(pairing[1], 2, 16, QLatin1Char('0'))
             .arg(pairing[2], 2, 16, QLatin1Char('0'))
             .toUpper();
}

void appendCard(QVariantList &components, const QString &cardType,
                const QString &title, const QString &version,
                uint32_t size = 0) {
  QVariantMap card;
  card[QStringLiteral("cardType")] = cardType;
  card[QStringLiteral("title")] = title;
  card[QStringLiteral("versionStr")] = version;
  card[QStringLiteral("sizeStr")] =
      size == 0 ? QString() : QStringLiteral("%1 bytes").arg(size);
  components.append(card);
}

void appendBootloaderCard(QVariantList &components,
                          const std::optional<BootloaderEntryInfo> &entry,
                          const QString &cardType, const QString &title) {
  if (entry && entry->present) {
    appendCard(components, cardType, title, QString::number(entry->version),
               entry->size);
  }
}

void replaceOrAppendKeyvaultCard(QVariantList &components,
                                 const QString &version) {
  for (QVariant &component : components) {
    QVariantMap card = component.toMap();
    if (card.value(QStringLiteral("cardType")) == QStringLiteral("keyvault")) {
      card[QStringLiteral("versionStr")] = version;
      component = card;
      return;
    }
  }
  appendCard(components, QStringLiteral("keyvault"), QStringLiteral("Keyvault"),
             version);
}

void mapLockdownValues(const BootloaderEntryInfo &entry, QString &ldv,
                       QString &pairing) {
  if (entry.ldv) {
    ldv = QString::number(*entry.ldv);
  }
  if (entry.pairing_data) {
    pairing = pairingString(*entry.pairing_data);
  }
}

} // namespace

NandInfoSnapshot MapPublicNandInfo(const AllNandInfo &info) {
  NandInfoSnapshot snapshot;

  if (info.block_type) {
    switch (*info.block_type) {
    case ImageType::SmallBlock:
      snapshot.imageSize = QStringLiteral("16MB");
      snapshot.blockType = QStringLiteral("Small Block");
      break;
    case ImageType::NewSmallBlock:
      snapshot.imageSize = QStringLiteral("16MB");
      snapshot.blockType = QStringLiteral("New Small Block");
      break;
    case ImageType::BigBlock:
      snapshot.imageSize = QStringLiteral("64MB");
      snapshot.blockType = QStringLiteral("Big Block");
      break;
    case ImageType::Emmc:
      snapshot.imageSize = QStringLiteral("48MB");
      snapshot.blockType = QStringLiteral("eMMC");
      break;
    }
  }

  snapshot.headerMagic = QStringLiteral("0x") +
                         QString::number(info.header_magic, 16).toUpper();
  snapshot.headerVersion = QString::number(info.header_version);

  if (info.smc.present) {
    snapshot.smcDecrypted = true;
    snapshot.smcVersion = QString::fromStdString(info.smc.version);
    snapshot.smcType = QString::fromStdString(info.smc.type_name);
    snapshot.consoleTarget = QString::fromStdString(info.smc.motherboard_name);
    snapshot.smcSize = QStringLiteral("%1 bytes").arg(info.smc.size);
    appendCard(snapshot.components, QStringLiteral("smc"),
               QStringLiteral("SMC Firmware"),
               snapshot.smcVersion.isEmpty() ? QStringLiteral("Clean")
                                             : snapshot.smcVersion,
               info.smc.size);
  }

  snapshot.cbAVersion = versionString(info.bootloaders.cb_a);
  snapshot.cbBVersion = versionString(info.bootloaders.cb_b);
  snapshot.cbXVersion = versionString(info.bootloaders.cb_x);
  snapshot.scVersion = versionString(info.bootloaders.sc);
  snapshot.cdVersion = versionString(info.bootloaders.cd);
  snapshot.ceVersion = versionString(info.bootloaders.ce);
  snapshot.cf0Version = versionString(info.bootloaders.cf_0);
  snapshot.cg0Version = versionString(info.bootloaders.cg_0);
  snapshot.cf1Version = versionString(info.bootloaders.cf_1);
  snapshot.cg1Version = versionString(info.bootloaders.cg_1);

  if (info.bootloaders.cb_a && info.bootloaders.cb_a->present) {
    snapshot.cbSize = QString::number(info.bootloaders.cb_a->size);
  }
  if (!snapshot.cbBVersion.isEmpty()) {
    snapshot.cbVersion = snapshot.cbAVersion + QStringLiteral(" / ") +
                         snapshot.cbBVersion;
  } else {
    snapshot.cbVersion = snapshot.cbAVersion;
  }

  appendBootloaderCard(snapshot.components, info.bootloaders.cb_a,
                       QStringLiteral("cb_a"), QStringLiteral("CB_A (2BL)"));
  appendBootloaderCard(snapshot.components, info.bootloaders.cb_b,
                       QStringLiteral("cb_b"), QStringLiteral("CB_B (2BL)"));
  appendBootloaderCard(snapshot.components, info.bootloaders.cb_x,
                       QStringLiteral("cb_x"), QStringLiteral("CB_X (RGH3)"));
  appendBootloaderCard(snapshot.components, info.bootloaders.sc,
                       QStringLiteral("sc"), QStringLiteral("SC (3BL)"));
  appendBootloaderCard(snapshot.components, info.bootloaders.cd,
                       QStringLiteral("cd"), QStringLiteral("CD (4BL)"));
  appendBootloaderCard(snapshot.components, info.bootloaders.ce,
                       QStringLiteral("ce"), QStringLiteral("CE (Kernel)"));
  appendBootloaderCard(snapshot.components, info.bootloaders.cf_0,
                       QStringLiteral("patch0"), QStringLiteral("Patchslot 0"));
  appendBootloaderCard(snapshot.components, info.bootloaders.cg_0,
                       QStringLiteral("cg_0"), QStringLiteral("CG 0"));
  appendBootloaderCard(snapshot.components, info.bootloaders.cf_1,
                       QStringLiteral("patch1"), QStringLiteral("Patchslot 1"));
  appendBootloaderCard(snapshot.components, info.bootloaders.cg_1,
                       QStringLiteral("cg_1"), QStringLiteral("CG 1"));

  if (info.keyvault.present) {
    replaceOrAppendKeyvaultCard(snapshot.components, QStringLiteral("Encrypted"));
  }

  return snapshot;
}

NandInfoSnapshot MapDecryptedNandInfo(const AllNandInfo &info,
                                      NandInfoSnapshot current) {
  if (info.bootloaders.cb_a && info.bootloaders.cb_a->present) {
    mapLockdownValues(*info.bootloaders.cb_a, current.cbALdv,
                      current.cbAPairing);
    current.cbLdv = current.cbALdv;
    current.cbPairing = current.cbAPairing;
  }
  if (info.bootloaders.cb_b && info.bootloaders.cb_b->present) {
    mapLockdownValues(*info.bootloaders.cb_b, current.cbLdv,
                      current.cbPairing);
  }
  if (info.bootloaders.cf_0 && info.bootloaders.cf_0->present) {
    mapLockdownValues(*info.bootloaders.cf_0, current.cf0Ldv,
                      current.cf0Pairing);
  }
  if (info.bootloaders.cf_1 && info.bootloaders.cf_1->present) {
    mapLockdownValues(*info.bootloaders.cf_1, current.cf1Ldv,
                      current.cf1Pairing);
  }
  current.ldvCount = current.cbLdv;

  if (info.keyvault.present && info.keyvault.decrypted) {
    current.serialNumber = QString::fromStdString(info.keyvault.serial_number);
    current.consoleId = QString::fromStdString(info.keyvault.console_id_raw);
    current.dvdKey = QString::fromStdString(info.keyvault.dvd_key);
    current.gameRegion =
        QStringLiteral("0x") +
        QStringLiteral("%1")
            .arg(info.keyvault.region_raw, 4, 16, QLatin1Char('0'))
            .toUpper();
    current.kvVersion = QString::number(info.keyvault.kv_type);
    replaceOrAppendKeyvaultCard(current.components, current.serialNumber);
  }

  return current;
}

QVariantMap NandInfoToVariantMap(const NandInfoSnapshot &snapshot) {
  const QVariantMap header{
      {QStringLiteral("imageSize"), snapshot.imageSize},
      {QStringLiteral("blockType"), snapshot.blockType},
      {QStringLiteral("consoleTarget"), snapshot.consoleTarget},
      {QStringLiteral("buildType"), snapshot.buildType},
      {QStringLiteral("kernelVerOrType"),
       snapshot.ceVersion.isEmpty() ? snapshot.buildType : snapshot.ceVersion},
      {QStringLiteral("magic"), snapshot.headerMagic},
      {QStringLiteral("version"), snapshot.headerVersion},
      {QStringLiteral("patchSlots"), snapshot.patchSlots},
  };
  const QVariantMap smc{
      {QStringLiteral("version"), snapshot.smcVersion},
      {QStringLiteral("type"), snapshot.smcType},
      {QStringLiteral("size"), snapshot.smcSize},
      {QStringLiteral("configOffset"), snapshot.smcConfigOffset},
  };
  const QVariantMap bootloaders{
      {QStringLiteral("cbVersion"), snapshot.cbVersion},
      {QStringLiteral("cbAVersion"), snapshot.cbAVersion},
      {QStringLiteral("cbBVersion"), snapshot.cbBVersion},
      {QStringLiteral("cbXVersion"), snapshot.cbXVersion},
      {QStringLiteral("cbSize"), snapshot.cbSize},
      {QStringLiteral("cbMagic"), snapshot.cbMagic},
      {QStringLiteral("scVersion"), snapshot.scVersion},
      {QStringLiteral("ccVersion"), snapshot.ccVersion},
      {QStringLiteral("cdVersion"), snapshot.cdVersion},
      {QStringLiteral("ceVersion"), snapshot.ceVersion},
      {QStringLiteral("cf0Version"), snapshot.cf0Version},
      {QStringLiteral("cg0Version"), snapshot.cg0Version},
      {QStringLiteral("cf1Version"), snapshot.cf1Version},
      {QStringLiteral("cg1Version"), snapshot.cg1Version},
      {QStringLiteral("cbLdv"), snapshot.cbLdv},
      {QStringLiteral("cbPairing"), snapshot.cbPairing},
      {QStringLiteral("cbALdv"), snapshot.cbALdv},
      {QStringLiteral("cbAPairing"), snapshot.cbAPairing},
      {QStringLiteral("cf0Ldv"), snapshot.cf0Ldv},
      {QStringLiteral("cf0Pairing"), snapshot.cf0Pairing},
      {QStringLiteral("cf1Ldv"), snapshot.cf1Ldv},
      {QStringLiteral("cf1Pairing"), snapshot.cf1Pairing},
      {QStringLiteral("ldvCount"), snapshot.ldvCount},
  };
  const QVariantMap keyvault{
      {QStringLiteral("serialNumber"), snapshot.serialNumber},
      {QStringLiteral("consoleId"), snapshot.consoleId},
      {QStringLiteral("dvdKey"), snapshot.dvdKey},
      {QStringLiteral("gameRegion"), snapshot.gameRegion},
      {QStringLiteral("consoleType"), snapshot.consoleType},
      {QStringLiteral("version"), snapshot.kvVersion},
  };
  return {
      {QStringLiteral("header"), header},
      {QStringLiteral("smc"), smc},
      {QStringLiteral("bootloaders"), bootloaders},
      {QStringLiteral("keyvault"), keyvault},
      {QStringLiteral("components"), snapshot.components},
  };
}
