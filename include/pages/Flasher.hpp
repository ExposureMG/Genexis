#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QVariantMap>
#include <qqmlregistration.h>

class Flasher : public QObject {
  Q_OBJECT
  QML_ELEMENT
  QML_SINGLETON

  Q_PROPERTY(bool isBusy READ isBusy NOTIFY busyStateChanged)
  Q_PROPERTY(QString connectedFlasherImage READ connectedFlasherImage NOTIFY
                 flasherImageChanged)
  Q_PROPERTY(QString connectedFlasherName READ connectedFlasherName NOTIFY
                 flasherNameChanged)
  Q_PROPERTY(bool isFlasherConnected READ isFlasherConnected NOTIFY
                 flasherConnectionChanged)

  Q_PROPERTY(QStringList availableFlashers READ availableFlashers NOTIFY
                 availableFlashersChanged)
  Q_PROPERTY(QString selectedFlasher READ selectedFlasher WRITE
                 setSelectedFlasher NOTIFY selectedFlasherChanged)
  Q_PROPERTY(bool isUpdClientSelected READ isUpdClientSelected NOTIFY
                 isUpdClientSelectedChanged)

  Q_PROPERTY(
      QString targetIp READ targetIp WRITE setTargetIp NOTIFY targetIpChanged)
  Q_PROPERTY(QStringList detectedNetworkDevices READ detectedNetworkDevices
                 NOTIFY detectedNetworkDevicesChanged)
  Q_PROPERTY(QString detectedHardwareInfo READ detectedHardwareInfo NOTIFY
                 detectedHardwareInfoChanged)

public:
  explicit Flasher(QObject *parent = nullptr);
  ~Flasher() override = default;

  static Flasher &instance();

  bool isBusy() const;
  QString connectedFlasherImage() const { return m_connectedFlasherImage; }
  QString connectedFlasherName() const { return m_connectedFlasherName; }
  bool isFlasherConnected() const { return m_isFlasherConnected; }

  QStringList availableFlashers() const { return m_availableFlashers; }
  QString selectedFlasher() const { return m_selectedFlasher; }
  Q_INVOKABLE void setSelectedFlasher(const QString &flasher);

  bool isUpdClientSelected() const { return m_isUpdClientSelected; }

  QString targetIp() const { return m_targetIp; }
  Q_INVOKABLE void setTargetIp(const QString &ip);

  QStringList detectedNetworkDevices() const {
    return m_detectedNetworkDevices;
  }
  QString detectedHardwareInfo() const { return m_detectedHardwareInfo; }

  /// Search for active UpdServer consoles over the local network via UDP
  Q_INVOKABLE void searchNetworkDevices();

  /// Run hardware detection: JTAG chain scan (if .svf/.xsvf) or Flash Config
  /// read (if SPI/eMMC)
  Q_INVOKABLE void detectHardware(const QString &filePath = {});

  /// Perform NAND read/write/erase or CPLD timing flash operation
  Q_INVOKABLE void performOperation(const QString &filePath,
                                    const QString &operation,
                                    const QVariantMap &options = {});

  /// Manually trigger a USB flasher device scan
  Q_INVOKABLE void checkUsbDevices();

Q_SIGNALS:
  void busyStateChanged(bool busy);
  void flasherImageChanged(const QString &imagePath);
  void flasherNameChanged(const QString &name);
  void flasherConnectionChanged(bool connected);
  void availableFlashersChanged();
  void selectedFlasherChanged();
  void isUpdClientSelectedChanged();
  void targetIpChanged();
  void detectedNetworkDevicesChanged();
  void detectedHardwareInfoChanged();

  void logOutput(const QString &message);
  void progressUpdated(qreal progress, const QString &status);
  void operationFinished(bool success, const QString &message);

private:
  void updateFlasherList();

  bool m_isBusy{false};
  QString m_connectedFlasherImage{
      QStringLiteral("qrc:/qt/qml/org/gxoss/genexis/assets/noflasher.png")};
  QString m_connectedFlasherName{QStringLiteral("No Flasher Connected")};
  bool m_isFlasherConnected{false};

  QStringList m_availableFlashers;
  QString m_selectedFlasher;
  bool m_isUpdClientSelected{false};

  QString m_targetIp;
  QStringList m_detectedNetworkDevices;
  QString m_detectedHardwareInfo{QStringLiteral("000000")};

  QTimer m_usbPollTimer;
};
