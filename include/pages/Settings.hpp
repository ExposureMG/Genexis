#pragma once

#include <QObject>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <qqmlregistration.h>

class Settings : public QObject {
  Q_OBJECT
  QML_ELEMENT
  QML_SINGLETON

  Q_PROPERTY(QString buildBackend READ buildBackend WRITE setBuildBackend NOTIFY
                 buildBackendChanged)
  Q_PROPERTY(QString flashBackend READ flashBackend WRITE setFlashBackend NOTIFY
                 flashBackendChanged)
  Q_PROPERTY(QString timingFlashBackend READ timingFlashBackend WRITE
                 setTimingFlashBackend NOTIFY timingFlashBackendChanged)
  Q_PROPERTY(QString wirelessBackend READ wirelessBackend WRITE
                 setWirelessBackend NOTIFY wirelessBackendChanged)

  Q_PROPERTY(
      QStringList availableBuildBackends READ availableBuildBackends CONSTANT)
  Q_PROPERTY(
      QStringList availableFlashBackends READ availableFlashBackends CONSTANT)
  Q_PROPERTY(QStringList availableTimingFlashBackends READ
                 availableTimingFlashBackends CONSTANT)
  Q_PROPERTY(QStringList availableWirelessBackends READ
                 availableWirelessBackends CONSTANT)

public:
  explicit Settings(QObject *parent = nullptr);
  ~Settings() override = default;

  static Settings &instance();

  QString buildBackend() const;
  void setBuildBackend(const QString &value);

  QString flashBackend() const;
  void setFlashBackend(const QString &value);

  QString timingFlashBackend() const;
  void setTimingFlashBackend(const QString &value);

  QString wirelessBackend() const;
  void setWirelessBackend(const QString &value);

  QStringList availableBuildBackends() const;
  QStringList availableFlashBackends() const;
  QStringList availableTimingFlashBackends() const;
  QStringList availableWirelessBackends() const;

  Q_INVOKABLE void loadSettings();
  Q_INVOKABLE void saveSettings();

  static QString getSetting(const QString &key,
                            const QString &defaultValue = QString());
  static void setSetting(const QString &key, const QString &value);

Q_SIGNALS:
  void buildBackendChanged();
  void flashBackendChanged();
  void timingFlashBackendChanged();
  void wirelessBackendChanged();
  void settingsLoaded();
  void settingsSaved();

private:
  static QString getIniPath();

  QString m_buildBackend;
  QString m_flashBackend;
  QString m_timingFlashBackend;
  QString m_wirelessBackend;
};
