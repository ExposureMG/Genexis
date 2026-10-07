#include "pages/NandInfoMapper.hpp"

#include "NandSnapshotFixture.hpp"

#include <QSet>
#include <QtTest/QTest>

namespace {

struct MappedField {
  const char *group;
  const char *key;
  QString NandInfoSnapshot::*field;
};

// The full layout NandInfoToVariantMap must produce, besides "components".
constexpr MappedField kMappedFields[] = {
    {"header", "imageSize", &NandInfoSnapshot::imageSize},
    {"header", "blockType", &NandInfoSnapshot::blockType},
    {"header", "consoleTarget", &NandInfoSnapshot::consoleTarget},
    {"header", "buildType", &NandInfoSnapshot::buildType},
    {"header", "magic", &NandInfoSnapshot::headerMagic},
    {"header", "version", &NandInfoSnapshot::headerVersion},
    {"header", "patchSlots", &NandInfoSnapshot::patchSlots},
    {"smc", "version", &NandInfoSnapshot::smcVersion},
    {"smc", "type", &NandInfoSnapshot::smcType},
    {"smc", "size", &NandInfoSnapshot::smcSize},
    {"smc", "configOffset", &NandInfoSnapshot::smcConfigOffset},
    {"bootloaders", "cbVersion", &NandInfoSnapshot::cbVersion},
    {"bootloaders", "cbAVersion", &NandInfoSnapshot::cbAVersion},
    {"bootloaders", "cbBVersion", &NandInfoSnapshot::cbBVersion},
    {"bootloaders", "cbXVersion", &NandInfoSnapshot::cbXVersion},
    {"bootloaders", "cbSize", &NandInfoSnapshot::cbSize},
    {"bootloaders", "cbMagic", &NandInfoSnapshot::cbMagic},
    {"bootloaders", "scVersion", &NandInfoSnapshot::scVersion},
    {"bootloaders", "ccVersion", &NandInfoSnapshot::ccVersion},
    {"bootloaders", "cdVersion", &NandInfoSnapshot::cdVersion},
    {"bootloaders", "ceVersion", &NandInfoSnapshot::ceVersion},
    {"bootloaders", "cf0Version", &NandInfoSnapshot::cf0Version},
    {"bootloaders", "cg0Version", &NandInfoSnapshot::cg0Version},
    {"bootloaders", "cf1Version", &NandInfoSnapshot::cf1Version},
    {"bootloaders", "cg1Version", &NandInfoSnapshot::cg1Version},
    {"bootloaders", "cbLdv", &NandInfoSnapshot::cbLdv},
    {"bootloaders", "cbPairing", &NandInfoSnapshot::cbPairing},
    {"bootloaders", "cbALdv", &NandInfoSnapshot::cbALdv},
    {"bootloaders", "cbAPairing", &NandInfoSnapshot::cbAPairing},
    {"bootloaders", "cf0Ldv", &NandInfoSnapshot::cf0Ldv},
    {"bootloaders", "cf0Pairing", &NandInfoSnapshot::cf0Pairing},
    {"bootloaders", "cf1Ldv", &NandInfoSnapshot::cf1Ldv},
    {"bootloaders", "cf1Pairing", &NandInfoSnapshot::cf1Pairing},
    {"bootloaders", "ldvCount", &NandInfoSnapshot::ldvCount},
    {"keyvault", "serialNumber", &NandInfoSnapshot::serialNumber},
    {"keyvault", "consoleId", &NandInfoSnapshot::consoleId},
    {"keyvault", "dvdKey", &NandInfoSnapshot::dvdKey},
    {"keyvault", "gameRegion", &NandInfoSnapshot::gameRegion},
    {"keyvault", "consoleType", &NandInfoSnapshot::consoleType},
    {"keyvault", "version", &NandInfoSnapshot::kvVersion},
};

QSet<QString> keysOf(const QVariantMap &map) {
  const QStringList keys = map.keys();
  return {keys.cbegin(), keys.cend()};
}

// Every group with exactly the keys from kMappedFields, plus kernelVerOrType.
void verifyLayout(const QVariantMap &map) {
  QCOMPARE(
      keysOf(map),
      (QSet<QString>{QStringLiteral("header"), QStringLiteral("smc"),
                     QStringLiteral("bootloaders"), QStringLiteral("keyvault"),
                     QStringLiteral("components")}));
  QHash<QString, QSet<QString>> expected;
  expected[QStringLiteral("header")].insert(QStringLiteral("kernelVerOrType"));
  for (const auto &mapped : kMappedFields) {
    expected[QLatin1StringView(mapped.group)].insert(
        QLatin1StringView(mapped.key));
  }
  for (auto it = expected.cbegin(); it != expected.cend(); ++it) {
    const QVariant group = map.value(it.key());
    QCOMPARE(group.typeId(), QMetaType::QVariantMap);
    QCOMPARE(keysOf(group.toMap()), it.value());
  }
  QCOMPARE(map.value(QStringLiteral("components")).typeId(),
           QMetaType::QVariantList);
}

QVariant mappedValue(const QVariantMap &map, const char *group,
                     const char *key) {
  return map.value(QLatin1StringView(group))
      .toMap()
      .value(QLatin1StringView(key));
}

} // namespace

class NandInfoMapperTests final : public QObject {
  Q_OBJECT

private Q_SLOTS:
  void publicInfoMapsGeometryAndBootloaders();
  void presentSmcKeepsLegacyDetailStateAvailable();
  void unavailableHeaderAndCertificateFactsStayEmpty();
  void decryptedInfoEnrichesKeyvaultAndLockdownValues();
  void decryptedInfoUsesCbBLockdownValuesForDualCbImages();
  void decryptedInfoReplacesEncryptedKeyvaultCard();
  void variantMapGroupsEverySnapshotField();
  void variantMapOfEmptySnapshotHasEveryKey();
  void kernelVerOrTypePrefersCeVersionOverBuildType();
};

void NandInfoMapperTests::publicInfoMapsGeometryAndBootloaders() {
  AllNandInfo info{};
  info.block_type = ImageType::NewSmallBlock;
  info.smc = {.version = "1.02",
              .motherboard_name = "Trinity",
              .type_name = "Retail",
              .size = 12288,
              .present = true,
              .decrypted = true};
  info.bootloaders.cb_a = BootloaderEntryInfo{.name = "CB_A",
                                              .version = 13121,
                                              .size = 0x4000,
                                              .present = true};
  info.bootloaders.cd = BootloaderEntryInfo{.name = "CD",
                                            .version = 17559,
                                            .size = 0x8000,
                                            .present = true};

  const auto snapshot = MapPublicNandInfo(info);

  QVERIFY(snapshot.smcDecrypted);
  QCOMPARE(snapshot.imageSize, QStringLiteral("16MB"));
  QCOMPARE(snapshot.blockType, QStringLiteral("New Small Block"));
  QCOMPARE(snapshot.consoleTarget, QStringLiteral("Trinity"));
  QCOMPARE(snapshot.cbAVersion, QStringLiteral("13121"));
  QCOMPARE(snapshot.cdVersion, QStringLiteral("17559"));
  QCOMPARE(snapshot.components.size(), 3);
  QVERIFY(snapshot.serialNumber.isEmpty());
}

void NandInfoMapperTests::presentSmcKeepsLegacyDetailStateAvailable() {
  AllNandInfo info{};
  info.smc = {.version = "1.02",
              .motherboard_name = "Trinity",
              .type_name = "Retail",
              .size = 12288,
              .present = true,
              .decrypted = false};

  const auto snapshot = MapPublicNandInfo(info);

  QVERIFY(snapshot.smcDecrypted);
  QCOMPARE(snapshot.smcVersion, QStringLiteral("1.02"));
}

void NandInfoMapperTests::unavailableHeaderAndCertificateFactsStayEmpty() {
  AllNandInfo info{};
  info.header_flags = 7;
  info.keyvault = {.kv_type = 2, .present = true, .decrypted = true};

  auto snapshot = MapPublicNandInfo(info);
  snapshot = MapDecryptedNandInfo(info, std::move(snapshot));

  QVERIFY(snapshot.patchSlots.isEmpty());
  QVERIFY(snapshot.smcConfigOffset.isEmpty());
  QVERIFY(snapshot.cbMagic.isEmpty());
  QVERIFY(snapshot.consoleType.isEmpty());
}

void NandInfoMapperTests::decryptedInfoEnrichesKeyvaultAndLockdownValues() {
  AllNandInfo info{};
  info.keyvault = {.serial_number = "123456789012",
                   .dvd_key = "00112233445566778899AABBCCDDEEFF",
                   .console_id_raw = "A1B2C3D4E5",
                   .region_name = "PAL/EU",
                   .region_raw = 0x02FE,
                   .kv_type = 2,
                   .present = true,
                   .decrypted = true};
  info.bootloaders.cb_a = BootloaderEntryInfo{
      .name = "CB_A",
      .version = 13121,
      .present = true,
      .ldv = 5,
      .pairing_data = std::array<uint8_t, 3>{0x12, 0x34, 0x56}};

  const auto snapshot = MapDecryptedNandInfo(info, {});

  QCOMPARE(snapshot.serialNumber, QStringLiteral("123456789012"));
  QCOMPARE(snapshot.dvdKey, QStringLiteral("00112233445566778899AABBCCDDEEFF"));
  QCOMPARE(snapshot.consoleId, QStringLiteral("A1B2C3D4E5"));
  QCOMPARE(snapshot.gameRegion, QStringLiteral("0x02FE"));
  QVERIFY(snapshot.consoleType.isEmpty());
  QCOMPARE(snapshot.kvVersion, QStringLiteral("2"));
  QCOMPARE(snapshot.cbLdv, QStringLiteral("5"));
  QCOMPARE(snapshot.cbALdv, QStringLiteral("5"));
  QCOMPARE(snapshot.cbPairing, QStringLiteral("0x123456"));
  QCOMPARE(snapshot.cbAPairing, QStringLiteral("0x123456"));
  QCOMPARE(snapshot.components.size(), 1);
  QCOMPARE(snapshot.components.first().toMap().value(QStringLiteral("versionStr")).toString(),
           QStringLiteral("123456789012"));
}

void NandInfoMapperTests::decryptedInfoUsesCbBLockdownValuesForDualCbImages() {
  AllNandInfo info{};
  info.bootloaders.cb_a = BootloaderEntryInfo{
      .name = "CB_A",
      .present = true,
      .ldv = 5,
      .pairing_data = std::array<uint8_t, 3>{0x12, 0x34, 0x56}};
  info.bootloaders.cb_b = BootloaderEntryInfo{
      .name = "CB_B",
      .present = true,
      .ldv = 6,
      .pairing_data = std::array<uint8_t, 3>{0xab, 0xcd, 0xef}};

  const auto snapshot = MapDecryptedNandInfo(info, {});

  QCOMPARE(snapshot.cbALdv, QStringLiteral("5"));
  QCOMPARE(snapshot.cbAPairing, QStringLiteral("0x123456"));
  QCOMPARE(snapshot.cbLdv, QStringLiteral("6"));
  QCOMPARE(snapshot.cbPairing, QStringLiteral("0xABCDEF"));
}

void NandInfoMapperTests::decryptedInfoReplacesEncryptedKeyvaultCard() {
  AllNandInfo publicInfo{};
  publicInfo.keyvault = {.present = true, .decrypted = false};
  auto snapshot = MapPublicNandInfo(publicInfo);
  QCOMPARE(snapshot.components.size(), 1);
  QCOMPARE(snapshot.components.first().toMap().value(QStringLiteral("versionStr")).toString(),
           QStringLiteral("Encrypted"));

  AllNandInfo decryptedInfo{};
  decryptedInfo.keyvault = {.serial_number = "123456789012",
                            .present = true,
                            .decrypted = true};
  snapshot = MapDecryptedNandInfo(decryptedInfo, std::move(snapshot));

  QCOMPARE(snapshot.components.size(), 1);
  QCOMPARE(snapshot.components.first().toMap().value(QStringLiteral("versionStr")).toString(),
           QStringLiteral("123456789012"));
}

void NandInfoMapperTests::variantMapGroupsEverySnapshotField() {
  const NandInfoSnapshot snapshot = makeDistinctSnapshot();
  const QVariantMap map = NandInfoToVariantMap(snapshot);

  verifyLayout(map);
  if (QTest::currentTestFailed()) {
    return;
  }
  for (const auto &mapped : kMappedFields) {
    const QVariant value = mappedValue(map, mapped.group, mapped.key);
    QVERIFY2(value.typeId() == QMetaType::QString, mapped.key);
    QCOMPARE(value.toString(), snapshot.*mapped.field);
  }
  QCOMPARE(mappedValue(map, "header", "kernelVerOrType").toString(),
           snapshot.ceVersion);
  QCOMPARE(map.value(QStringLiteral("components")).toList(),
           snapshot.components);
}

void NandInfoMapperTests::variantMapOfEmptySnapshotHasEveryKey() {
  const QVariantMap map = NandInfoToVariantMap({});

  verifyLayout(map);
  if (QTest::currentTestFailed()) {
    return;
  }
  for (const auto &mapped : kMappedFields) {
    const QVariant value = mappedValue(map, mapped.group, mapped.key);
    QVERIFY2(value.typeId() == QMetaType::QString, mapped.key);
    QVERIFY2(value.toString().isEmpty(), mapped.key);
  }
  QVERIFY(mappedValue(map, "header", "kernelVerOrType").toString().isEmpty());
  QVERIFY(map.value(QStringLiteral("components")).toList().isEmpty());
}

void NandInfoMapperTests::kernelVerOrTypePrefersCeVersionOverBuildType() {
  NandInfoSnapshot snapshot;
  snapshot.buildType = QStringLiteral("Retail");
  QCOMPARE(
      mappedValue(NandInfoToVariantMap(snapshot), "header", "kernelVerOrType")
          .toString(),
      QStringLiteral("Retail"));

  snapshot.ceVersion = QStringLiteral("17559");
  QCOMPARE(
      mappedValue(NandInfoToVariantMap(snapshot), "header", "kernelVerOrType")
          .toString(),
      QStringLiteral("17559"));
}

QTEST_MAIN(NandInfoMapperTests)
#include "NandInfoMapperTests.moc"
