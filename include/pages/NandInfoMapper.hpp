#pragma once

#ifdef slots
#pragma push_macro("slots")
#undef slots
#define GENEXIS_RESTORE_QT_SLOTS
#endif
#include "Args.hpp"
#ifdef GENEXIS_RESTORE_QT_SLOTS
#pragma pop_macro("slots")
#undef GENEXIS_RESTORE_QT_SLOTS
#endif

#include <QString>
#include <QVariantList>

struct NandInfoSnapshot {
  QString imageSize;
  QString blockType;
  QString consoleTarget;
  QString buildType;
  QString headerMagic;
  QString headerVersion;
  QString patchSlots;
  QString smcVersion;
  QString smcType;
  QString smcSize;
  QString smcConfigOffset;
  bool smcDecrypted{false};
  QString cbVersion;
  QString cbAVersion;
  QString cbBVersion;
  QString cbXVersion;
  QString cbSize;
  QString cbMagic;
  QString scVersion;
  QString ccVersion;
  QString cdVersion;
  QString ceVersion;
  QString cf0Version;
  QString cg0Version;
  QString cf1Version;
  QString cg1Version;
  QString cbLdv;
  QString cbPairing;
  QString cbALdv;
  QString cbAPairing;
  QString cf0Ldv;
  QString cf0Pairing;
  QString cf1Ldv;
  QString cf1Pairing;
  QString serialNumber;
  QString consoleId;
  QString dvdKey;
  QString gameRegion;
  QString consoleType;
  QString kvVersion;
  QString ldvCount;
  QVariantList components;
};

NandInfoSnapshot MapPublicNandInfo(const AllNandInfo &info);
NandInfoSnapshot MapDecryptedNandInfo(const AllNandInfo &info,
                                      NandInfoSnapshot current);
