#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>

#include <cstdint>
#include <vector>

#include "pages/NandInfoMapper.hpp"

class Nand : public QObject {
  Q_OBJECT

  Q_PROPERTY(bool isLoading READ isLoading NOTIFY loadingChanged)
  Q_PROPERTY(bool isNandLoaded READ isNandLoaded NOTIFY nandStateChanged)
  Q_PROPERTY(bool isCpuKeyLoaded READ isCpuKeyLoaded NOTIFY cpuKeyStateChanged)
  Q_PROPERTY(bool isSmcDecrypted READ isSmcDecrypted NOTIFY smcStateChanged)

  Q_PROPERTY(
      QString loadedFilePath READ loadedFilePath NOTIFY loadedFilePathChanged)
  Q_PROPERTY(QString cpuKey READ cpuKey WRITE setCpuKey NOTIFY cpuKeyChanged)

  // Per-image metadata from the last applied NandInfoSnapshot, grouped by
  // concern; NandInfoToVariantMap() documents the layout.
  Q_PROPERTY(QVariantMap metadata READ metadata NOTIFY metadataChanged)

public:
  explicit Nand(QObject *parent = nullptr);
  ~Nand() override = default;

  static Nand &instance();

  bool isLoading() const { return m_isLoading; }
  bool isNandLoaded() const { return m_isNandLoaded; }
  bool isCpuKeyLoaded() const { return m_isCpuKeyLoaded; }
  bool isSmcDecrypted() const { return m_info.smcDecrypted; }

  QString loadedFilePath() const { return m_loadedFilePath; }
  QString cpuKey() const { return m_cpuKey; }

  QVariantMap metadata() const { return NandInfoToVariantMap(m_info); }

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

protected:
  // Replaces all metadata and marks the image loaded. Protected so tests can
  // drive the model without a real NAND image.
  void applySnapshot(const NandInfoSnapshot &snapshot, bool decrypted);

private:
  void parseNandData(const std::vector<uint8_t> &data);

  bool m_isLoading{false};
  // Bumped by clear(); a load whose generation is stale drops its result.
  quint64 m_loadGeneration{0};
  bool m_isNandLoaded{false};
  bool m_isCpuKeyLoaded{false};

  QString m_loadedFilePath;
  QString m_cpuKey;

  NandInfoSnapshot m_info;
  std::vector<uint8_t> m_rawNandData;
};
