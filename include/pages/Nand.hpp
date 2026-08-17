#pragma once

#include <QObject>
#include <QString>
#include <qqmlregistration.h>

class Nand : public QObject {
  Q_OBJECT
  QML_ELEMENT
  QML_SINGLETON

  Q_PROPERTY(bool isLoading READ isLoading NOTIFY loadingChanged)
  Q_PROPERTY(bool isNandLoaded READ isNandLoaded NOTIFY nandStateChanged)
  Q_PROPERTY(bool isCpuKeyLoaded READ isCpuKeyLoaded NOTIFY cpuKeyStateChanged)
  Q_PROPERTY(bool isSmcDecrypted READ isSmcDecrypted NOTIFY smcStateChanged)

  Q_PROPERTY(
      QString loadedFilePath READ loadedFilePath NOTIFY loadedFilePathChanged)
  Q_PROPERTY(QString cpuKey READ cpuKey WRITE setCpuKey NOTIFY cpuKeyChanged)

  
  Q_PROPERTY(QString consoleTarget READ consoleTarget NOTIFY metadataChanged)
  Q_PROPERTY(QString buildType READ buildType NOTIFY metadataChanged)
  Q_PROPERTY(QString imageSize READ imageSize NOTIFY metadataChanged)
  Q_PROPERTY(QString headerMagic READ headerMagic NOTIFY metadataChanged)
  Q_PROPERTY(QString headerVersion READ headerVersion NOTIFY metadataChanged)
  Q_PROPERTY(QString patchSlots READ patchSlots NOTIFY metadataChanged)

  
  Q_PROPERTY(QString cbVersion READ cbVersion NOTIFY metadataChanged)
  Q_PROPERTY(QString cbAVersion READ cbAVersion NOTIFY metadataChanged)
  Q_PROPERTY(QString cbBVersion READ cbBVersion NOTIFY metadataChanged)
  Q_PROPERTY(QString cbXVersion READ cbXVersion NOTIFY metadataChanged)
  Q_PROPERTY(QString cbSize READ cbSize NOTIFY metadataChanged)
  Q_PROPERTY(QString cbMagic READ cbMagic NOTIFY metadataChanged)
  Q_PROPERTY(QString scVersion READ scVersion NOTIFY metadataChanged)
  Q_PROPERTY(QString ccVersion READ ccVersion NOTIFY metadataChanged)
  Q_PROPERTY(QString cdVersion READ cdVersion NOTIFY metadataChanged)
  Q_PROPERTY(QString ceVersion READ ceVersion NOTIFY metadataChanged)
  Q_PROPERTY(QString cf0Version READ cf0Version NOTIFY metadataChanged)
  Q_PROPERTY(QString cg0Version READ cg0Version NOTIFY metadataChanged)
  Q_PROPERTY(QString cf1Version READ cf1Version NOTIFY metadataChanged)
  Q_PROPERTY(QString cg1Version READ cg1Version NOTIFY metadataChanged)

  
  Q_PROPERTY(QString cbLdv READ cbLdv NOTIFY metadataChanged)
  Q_PROPERTY(QString cbPairing READ cbPairing NOTIFY metadataChanged)
  Q_PROPERTY(QString cbALdv READ cbALdv NOTIFY metadataChanged)
  Q_PROPERTY(QString cbAPairing READ cbAPairing NOTIFY metadataChanged)
  Q_PROPERTY(QString cf0Ldv READ cf0Ldv NOTIFY metadataChanged)
  Q_PROPERTY(QString cf0Pairing READ cf0Pairing NOTIFY metadataChanged)
  Q_PROPERTY(QString cf1Ldv READ cf1Ldv NOTIFY metadataChanged)
  Q_PROPERTY(QString cf1Pairing READ cf1Pairing NOTIFY metadataChanged)

  
  Q_PROPERTY(QString smcVersion READ smcVersion NOTIFY metadataChanged)
  Q_PROPERTY(QString smcType READ smcType NOTIFY metadataChanged)
  Q_PROPERTY(
      QString smcConfigOffset READ smcConfigOffset NOTIFY metadataChanged)
  Q_PROPERTY(QString smcSize READ smcSize NOTIFY metadataChanged)

  
  Q_PROPERTY(QString serialNumber READ serialNumber NOTIFY metadataChanged)
  Q_PROPERTY(QString consoleId READ consoleId NOTIFY metadataChanged)
  Q_PROPERTY(QString dvdKey READ dvdKey NOTIFY metadataChanged)
  Q_PROPERTY(QString gameRegion READ gameRegion NOTIFY metadataChanged)
  Q_PROPERTY(QString consoleType READ consoleType NOTIFY metadataChanged)
  Q_PROPERTY(QString kvVersion READ kvVersion NOTIFY metadataChanged)
  Q_PROPERTY(QString ldvCount READ ldvCount NOTIFY metadataChanged)

  
  Q_PROPERTY(QVariantList components READ components NOTIFY componentsChanged)

public:
  explicit Nand(QObject *parent = nullptr);
  ~Nand() override = default;

  static Nand &instance();

  bool isLoading() const { return m_isLoading; }
  bool isNandLoaded() const { return m_isNandLoaded; }
  bool isCpuKeyLoaded() const { return m_isCpuKeyLoaded; }
  bool isSmcDecrypted() const { return m_isSmcDecrypted; }

  QString loadedFilePath() const { return m_loadedFilePath; }
  QString cpuKey() const { return m_cpuKey; }

  QString consoleTarget() const { return m_consoleTarget; }
  QString buildType() const { return m_buildType; }
  QString imageSize() const { return m_imageSize; }
  QString headerMagic() const { return m_headerMagic; }
  QString headerVersion() const { return m_headerVersion; }
  QString patchSlots() const { return m_patchSlots; }

  QString cbVersion() const { return m_cbVersion; }
  QString cbAVersion() const { return m_cbAVersion; }
  QString cbBVersion() const { return m_cbBVersion; }
  QString cbXVersion() const { return m_cbXVersion; }
  QString cbSize() const { return m_cbSize; }
  QString cbMagic() const { return m_cbMagic; }
  QString scVersion() const { return m_scVersion; }
  QString ccVersion() const { return m_ccVersion; }
  QString cdVersion() const { return m_cdVersion; }
  QString ceVersion() const { return m_ceVersion; }
  QString cf0Version() const { return m_cf0Version; }
  QString cg0Version() const { return m_cg0Version; }
  QString cf1Version() const { return m_cf1Version; }
  QString cg1Version() const { return m_cg1Version; }

  QString cbLdv() const { return m_cbLdv; }
  QString cbPairing() const { return m_cbPairing; }
  QString cbALdv() const { return m_cbALdv; }
  QString cbAPairing() const { return m_cbAPairing; }
  QString cf0Ldv() const { return m_cf0Ldv; }
  QString cf0Pairing() const { return m_cf0Pairing; }
  QString cf1Ldv() const { return m_cf1Ldv; }
  QString cf1Pairing() const { return m_cf1Pairing; }

  QString smcVersion() const { return m_smcVersion; }
  QString smcType() const { return m_smcType; }
  QString smcConfigOffset() const { return m_smcConfigOffset; }
  QString smcSize() const { return m_smcSize; }

  QString serialNumber() const { return m_serialNumber; }
  QString consoleId() const { return m_consoleId; }
  QString dvdKey() const { return m_dvdKey; }
  QString gameRegion() const { return m_gameRegion; }
  QString consoleType() const { return m_consoleType; }
  QString kvVersion() const { return m_kvVersion; }
  QString ldvCount() const { return m_ldvCount; }

  QVariantList components() const { return m_components; }

  Q_INVOKABLE void openFile(const QString &filePath,
                            const QString &cpuKey = QString());
  Q_INVOKABLE void setCpuKey(const QString &cpuKey);
  Q_INVOKABLE QString detectCpuKey(const QString &filePath);
  Q_INVOKABLE void clear();

Q_SIGNALS:
  void loadingChanged();
  void nandStateChanged();
  void cpuKeyStateChanged();
  void smcStateChanged();
  void loadedFilePathChanged();
  void cpuKeyChanged();
  void metadataChanged();
  void componentsChanged();

private:
  void parseNandData(const std::vector<uint8_t> &data);
  void parseKeyvault(const std::vector<uint8_t> &cpuKeyBytes,
                     const std::vector<uint8_t> &rawKv);

  bool m_isLoading{false};
  bool m_isNandLoaded{false};
  bool m_isCpuKeyLoaded{false};
  bool m_isSmcDecrypted{false};

  QString m_loadedFilePath;
  QString m_cpuKey;

  QString m_consoleTarget;
  QString m_buildType;
  QString m_imageSize;
  QString m_headerMagic;
  QString m_headerVersion;
  QString m_patchSlots;

  QString m_cbVersion;
  QString m_cbAVersion;
  QString m_cbBVersion;
  QString m_cbXVersion;
  QString m_cbSize;
  QString m_cbMagic;
  QString m_scVersion;
  QString m_ccVersion;
  QString m_cdVersion;
  QString m_ceVersion;
  QString m_cf0Version;
  QString m_cg0Version;
  QString m_cf1Version;
  QString m_cg1Version;

  QString m_cbLdv;
  QString m_cbPairing;
  QString m_cbALdv;
  QString m_cbAPairing;
  QString m_cf0Ldv;
  QString m_cf0Pairing;
  QString m_cf1Ldv;
  QString m_cf1Pairing;

  QString m_smcVersion;
  QString m_smcType;
  QString m_smcConfigOffset;
  QString m_smcSize;

  QString m_serialNumber;
  QString m_consoleId;
  QString m_dvdKey;
  QString m_gameRegion;
  QString m_consoleType;
  QString m_kvVersion;
  QString m_ldvCount;

  QVariantList m_components;

  std::vector<uint8_t> m_rawNandData;
};
