#include "pages/Settings.hpp"
#include "StartupManager.hpp"
#include "backend/BackendManager.hpp"

#include <QDebug>
#include <QDir>
#include <QSettings>
#include <QStandardPaths>

using BackendManager = gxapi::backend::BackendManager;

namespace {
QStringList toStringList(const std::vector<std::string> &values) {
  QStringList list;
  list.reserve(static_cast<qsizetype>(values.size()));
  for (const auto &value : values) {
    list.append(QString::fromStdString(value));
  }
  return list;
}
} // namespace

Settings::Settings(QObject *parent)
    : QObject(parent), m_buildBackend(QStringLiteral("gxbuild3")),
      m_flashBackend(QStringLiteral("NandProMax")),
      m_timingFlashBackend(QStringLiteral("NandProMax")),
      m_wirelessBackend(QStringLiteral("UpdClient")) {
  loadSettings();
}

Settings &Settings::instance() {
  static Settings inst;
  return inst;
}

QString Settings::getIniPath() {
  QString appData = StartupManager::instance().appDataPath();
  if (appData.isEmpty()) {
    appData =
        QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
  }
  return QDir(appData).filePath(QStringLiteral("genexis.ini"));
}

void Settings::loadSettings() {
  QSettings ini(getIniPath(), QSettings::IniFormat);
  ini.beginGroup(QStringLiteral("Backends"));

  setBuildBackend(
      ini.value(QStringLiteral("buildBackend"), QStringLiteral("gxbuild3"))
          .toString());
  setFlashBackend(
      ini.value(QStringLiteral("flashBackend"), QStringLiteral("NandProMax"))
          .toString());
  setTimingFlashBackend(ini.value(QStringLiteral("timingFlashBackend"),
                                  QStringLiteral("NandProMax"))
                            .toString());
  setWirelessBackend(
      ini.value(QStringLiteral("wirelessBackend"), QStringLiteral("UpdClient"))
          .toString());

  ini.endGroup();

  Q_EMIT settingsLoaded();
  qDebug() << "[Settings] Loaded settings from" << getIniPath()
           << "flashBackend:" << m_flashBackend
           << "timingFlashBackend:" << m_timingFlashBackend;
}

void Settings::saveSettings() {
  QSettings ini(getIniPath(), QSettings::IniFormat);
  ini.beginGroup(QStringLiteral("Backends"));

  ini.setValue(QStringLiteral("buildBackend"), m_buildBackend);
  ini.setValue(QStringLiteral("flashBackend"), m_flashBackend);
  ini.setValue(QStringLiteral("timingFlashBackend"), m_timingFlashBackend);
  ini.setValue(QStringLiteral("wirelessBackend"), m_wirelessBackend);

  ini.endGroup();
  ini.sync();

  Q_EMIT settingsSaved();
  qDebug() << "[Settings] Saved settings to" << getIniPath();
}

QString Settings::buildBackend() const { return m_buildBackend; }
void Settings::setBuildBackend(const QString &value) {
  if (m_buildBackend != value) {
    m_buildBackend = value;
    Q_EMIT buildBackendChanged();
  }
}

QString Settings::flashBackend() const { return m_flashBackend; }
void Settings::setFlashBackend(const QString &value) {
  if (m_flashBackend != value) {
    m_flashBackend = value;
    Q_EMIT flashBackendChanged();
  }
}

QString Settings::timingFlashBackend() const { return m_timingFlashBackend; }
void Settings::setTimingFlashBackend(const QString &value) {
  if (m_timingFlashBackend != value) {
    m_timingFlashBackend = value;
    Q_EMIT timingFlashBackendChanged();
  }
}

QString Settings::wirelessBackend() const { return m_wirelessBackend; }
void Settings::setWirelessBackend(const QString &value) {
  if (m_wirelessBackend != value) {
    m_wirelessBackend = value;
    Q_EMIT wirelessBackendChanged();
  }
}

QStringList Settings::availableBuildBackends() const {
  return toStringList(BackendManager::instance().getAvailableBuilderBackends());
}

QStringList Settings::availableFlashBackends() const {
  return toStringList(BackendManager::instance().getAvailableFlashBackends());
}

QStringList Settings::availableTimingFlashBackends() const {
  return toStringList(BackendManager::instance().getAvailableJtagBackends());
}

QStringList Settings::availableWirelessBackends() const {
  return toStringList(BackendManager::instance().getAvailableNetworkBackends());
}
