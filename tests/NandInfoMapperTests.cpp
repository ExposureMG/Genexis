#include "pages/NandInfoMapper.hpp"

#include <QtTest/QTest>

class NandInfoMapperTests final : public QObject {
  Q_OBJECT

private slots:
  void publicInfoMapsGeometryAndBootloaders();
  void decryptedInfoEnrichesKeyvaultAndLockdownValues();
  void decryptedInfoUsesCbBLockdownValuesForDualCbImages();
  void decryptedInfoReplacesEncryptedKeyvaultCard();
};

void NandInfoMapperTests::publicInfoMapsGeometryAndBootloaders() {
  AllNandInfo info{};
  info.block_type = ImageType::NewSmallBlock;
  info.smc = {.version = "1.02",
              .motherboard_name = "Trinity",
              .type_name = "Retail",
              .size = 12288,
              .present = true};
  info.bootloaders.cb_a = BootloaderEntryInfo{.name = "CB_A",
                                              .version = 13121,
                                              .size = 0x4000,
                                              .present = true};
  info.bootloaders.cd = BootloaderEntryInfo{.name = "CD",
                                            .version = 17559,
                                            .size = 0x8000,
                                            .present = true};

  const auto snapshot = MapPublicNandInfo(info);

  QCOMPARE(snapshot.imageSize, QStringLiteral("16MB"));
  QCOMPARE(snapshot.blockType, QStringLiteral("New Small Block"));
  QCOMPARE(snapshot.consoleTarget, QStringLiteral("Trinity"));
  QCOMPARE(snapshot.cbAVersion, QStringLiteral("13121"));
  QCOMPARE(snapshot.cdVersion, QStringLiteral("17559"));
  QCOMPARE(snapshot.components.size(), 3);
  QVERIFY(snapshot.serialNumber.isEmpty());
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
  QCOMPARE(snapshot.consoleType, QStringLiteral("2"));
  QCOMPARE(snapshot.cbLdv, QStringLiteral("5"));
  QCOMPARE(snapshot.cbALdv, QStringLiteral("5"));
  QCOMPARE(snapshot.cbPairing, QStringLiteral("0x123456"));
  QCOMPARE(snapshot.cbAPairing, QStringLiteral("0x123456"));
  QCOMPARE(snapshot.components.size(), 1);
  QCOMPARE(snapshot.components.first().toMap().value("versionStr").toString(),
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
  QCOMPARE(snapshot.components.first().toMap().value("versionStr").toString(),
           QStringLiteral("Encrypted"));

  AllNandInfo decryptedInfo{};
  decryptedInfo.keyvault = {.serial_number = "123456789012",
                            .present = true,
                            .decrypted = true};
  snapshot = MapDecryptedNandInfo(decryptedInfo, std::move(snapshot));

  QCOMPARE(snapshot.components.size(), 1);
  QCOMPARE(snapshot.components.first().toMap().value("versionStr").toString(),
           QStringLiteral("123456789012"));
}

QTEST_MAIN(NandInfoMapperTests)
#include "NandInfoMapperTests.moc"
