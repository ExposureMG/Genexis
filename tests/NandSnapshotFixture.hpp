#pragma once

#include "pages/NandInfoMapper.hpp"

#include <QVariantMap>

// A snapshot whose string fields all hold distinct values, so a test can tell
// which field ended up where.
inline NandInfoSnapshot makeDistinctSnapshot() {
  return {
      .imageSize = QStringLiteral("64MB"),
      .blockType = QStringLiteral("Big Block"),
      .consoleTarget = QStringLiteral("Jasper"),
      .buildType = QStringLiteral("Retail"),
      .headerMagic = QStringLiteral("0xFF4F"),
      .headerVersion = QStringLiteral("1888"),
      .patchSlots = QStringLiteral("2"),
      .smcVersion = QStringLiteral("2.03"),
      .smcType = QStringLiteral("Retail SMC"),
      .smcSize = QStringLiteral("12288 bytes"),
      .smcConfigOffset = QStringLiteral("0xF7C000"),
      .smcDecrypted = true,
      .cbVersion = QStringLiteral("6750 / 6751"),
      .cbAVersion = QStringLiteral("6750"),
      .cbBVersion = QStringLiteral("6751"),
      .cbXVersion = QStringLiteral("5772"),
      .cbSize = QStringLiteral("16384"),
      .cbMagic = QStringLiteral("0x4342"),
      .scVersion = QStringLiteral("1"),
      .ccVersion = QStringLiteral("2"),
      .cdVersion = QStringLiteral("6752"),
      .ceVersion = QStringLiteral("17489"),
      .cf0Version = QStringLiteral("17559"),
      .cg0Version = QStringLiteral("17560"),
      .cf1Version = QStringLiteral("17561"),
      .cg1Version = QStringLiteral("17562"),
      .cbLdv = QStringLiteral("12"),
      .cbPairing = QStringLiteral("0xABCDEF"),
      .cbALdv = QStringLiteral("11"),
      .cbAPairing = QStringLiteral("0x123456"),
      .cf0Ldv = QStringLiteral("13"),
      .cf0Pairing = QStringLiteral("0x654321"),
      .cf1Ldv = QStringLiteral("14"),
      .cf1Pairing = QStringLiteral("0xFEDCBA"),
      .serialNumber = QStringLiteral("123456789012"),
      .consoleId = QStringLiteral("A1B2C3D4E5"),
      .dvdKey = QStringLiteral("00112233445566778899AABBCCDDEEFF"),
      .gameRegion = QStringLiteral("0x02FE"),
      .consoleType = QStringLiteral("Jasper 256MB"),
      .kvVersion = QStringLiteral("2"),
      .ldvCount = QStringLiteral("15"),
      .components = {QVariantMap{
          {QStringLiteral("cardType"), QStringLiteral("smc")},
          {QStringLiteral("title"), QStringLiteral("SMC Firmware")},
          {QStringLiteral("versionStr"), QStringLiteral("2.03")},
          {QStringLiteral("sizeStr"), QStringLiteral("12288 bytes")},
      }},
  };
}
