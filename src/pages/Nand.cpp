#include "pages/Nand.hpp"

#include "Async.hpp"
#include "FilePaths.hpp"
#include "Library.hpp"

#include <QByteArray>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>

#include <fstream>
#include <iterator>
#include <optional>

namespace {

// Looks for a cpukey.txt next to a .bin/.ecc NAND image. It runs on load
// workers too, so it must not touch Nand state.
QString findCpuKeyNextTo(const QString &filePath) {
  const QString cleanPath = gxapi::toLocalPath(filePath.trimmed());
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

struct LoadedNand {
  std::vector<uint8_t> data;
  QString cpuKey;
};

// Empty when the file cannot be read or is empty.
std::optional<LoadedNand> readNandFile(const QString &path,
                                       const QString &cpuKey) {
  std::ifstream file(path.toStdString(), std::ios::binary);
  if (!file) {
    qDebug() << "[Nand] Failed to open NAND file:" << path;
    return std::nullopt;
  }

  LoadedNand loaded;
  loaded.data.assign(std::istreambuf_iterator<char>(file),
                     std::istreambuf_iterator<char>());
  if (loaded.data.empty()) {
    qDebug() << "[Nand] NAND file is empty:" << path;
    return std::nullopt;
  }

  loaded.cpuKey = cpuKey.trimmed();
  if (loaded.cpuKey.isEmpty()) {
    loaded.cpuKey = findCpuKeyNextTo(path);
  }
  return loaded;
}

} // namespace

Nand::Nand(QObject *parent) : QObject(parent) {}

Nand &Nand::instance() {
  static Nand inst;
  return inst;
}

void Nand::clear() {
  ++m_loadGeneration;
  if (m_isLoading) {
    m_isLoading = false;
    Q_EMIT loadingChanged();
  }

  m_isNandLoaded = false;
  m_isCpuKeyLoaded = false;

  m_loadedFilePath.clear();
  m_cpuKey.clear();

  m_info = {};
  m_rawNandData.clear();

  Q_EMIT nandStateChanged();
  Q_EMIT cpuKeyStateChanged();
  Q_EMIT smcStateChanged();
  Q_EMIT loadedFilePathChanged();
  Q_EMIT cpuKeyChanged();
  Q_EMIT metadataChanged();
}

void Nand::openFile(const QString &filePath, const QString &cpuKey) {
  clear();

  const QString cleanPath = gxapi::toLocalPath(filePath);
  if (cleanPath.isEmpty()) {
    return;
  }

  if (gxapi::isJtagTimingFile(cleanPath)) {
    m_loadedFilePath = cleanPath;
    Q_EMIT loadedFilePathChanged();
    qDebug() << "[Nand] Opened timing file (.svf/.xsvf) -> keeping NAND "
                "metadata empty/greyed.";
    return;
  }

  const QString ext = QFileInfo(cleanPath).suffix().toLower();
  if (ext != QStringLiteral("bin") && ext != QStringLiteral("ecc")) {
    qDebug() << "[Nand] Unsupported file extension for NAND info:" << ext;
    return;
  }

  m_isLoading = true;
  Q_EMIT loadingChanged();

  const quint64 generation = m_loadGeneration;
  gxapi::runAsync(
      this, [cleanPath, cpuKey]() { return readNandFile(cleanPath, cpuKey); },
      [this, generation, cleanPath](std::optional<LoadedNand> loaded) {
        if (generation != m_loadGeneration) {
          return;
        }
        if (loaded) {
          m_rawNandData = std::move(loaded->data);
          m_loadedFilePath = cleanPath;
          Q_EMIT loadedFilePathChanged();

          parseNandData(m_rawNandData);

          if (!loaded->cpuKey.isEmpty()) {
            setCpuKey(loaded->cpuKey);
          }
        }
        m_isLoading = false;
        Q_EMIT loadingChanged();
      });
}

QString Nand::detectCpuKey(const QString &filePath) {
  return findCpuKeyNextTo(filePath);
}

void Nand::parseNandData(const std::vector<uint8_t> &data) {
  const auto info = GxBuild::ExtractSomeInfo(data);
  if (!info) {
    qDebug() << "[Nand] Invalid NAND image format or header.";
    return;
  }
  applySnapshot(MapPublicNandInfo(*info), false);
}

void Nand::setCpuKey(const QString &cpuKey) {
  m_cpuKey = cpuKey.trimmed().toUpper();
  Q_EMIT cpuKeyChanged();

  static const QRegularExpression cpuKeyPattern(
      QStringLiteral("^[0-9A-F]{32}$"));
  const auto restorePublicSnapshot = [this]() {
    const auto publicInfo = GxBuild::ExtractSomeInfo(m_rawNandData);
    if (publicInfo) {
      applySnapshot(MapPublicNandInfo(*publicInfo), false);
      return;
    }
    m_isCpuKeyLoaded = false;
    Q_EMIT cpuKeyStateChanged();
  };
  if (!cpuKeyPattern.match(m_cpuKey).hasMatch()) {
    restorePublicSnapshot();
    return;
  }

  if (m_rawNandData.empty()) {
    restorePublicSnapshot();
    return;
  }

  const QByteArray decodedKey = QByteArray::fromHex(m_cpuKey.toLatin1());
  const std::vector<uint8_t> keyBytes(decodedKey.cbegin(), decodedKey.cend());
  const auto info = GxBuild::ExtractAllInfo(m_rawNandData, keyBytes);
  if (!info) {
    qDebug() << "[Nand] Keyvault decryption failed.";
    restorePublicSnapshot();
    return;
  }

  applySnapshot(MapDecryptedNandInfo(*info, m_info), true);
}

void Nand::applySnapshot(const NandInfoSnapshot &snapshot, bool decrypted) {
  m_info = snapshot;
  m_isNandLoaded = true;
  m_isCpuKeyLoaded = decrypted;

  Q_EMIT nandStateChanged();
  Q_EMIT cpuKeyStateChanged();
  Q_EMIT smcStateChanged();
  Q_EMIT metadataChanged();
}
