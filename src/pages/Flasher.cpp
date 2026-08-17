#include "pages/Flasher.hpp"
#include "StartupManager.hpp"
#include "backend/BackendManager.hpp"
#include "pages/Nand.hpp"
#include "pages/Settings.hpp"
#include "vendor/USBDiscovery.hpp"

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QMetaObject>
#include <QUrl>
#include <iomanip>
#include <sstream>
#include <thread>

using USBDiscovery = Genexis::Devices::Vendor::USBDiscovery;
using BackendManager = gxapi::backend::BackendManager;

Flasher::Flasher(QObject *parent) : QObject(parent) {
  connect(&m_usbPollTimer, &QTimer::timeout, this, &Flasher::checkUsbDevices);
  m_usbPollTimer.start(2000);
  checkUsbDevices();
}

Flasher &Flasher::instance() {
  static Flasher inst;
  return inst;
}

bool Flasher::isBusy() const { return m_isBusy; }

void Flasher::setSelectedFlasher(const QString &flasher) {
  if (m_selectedFlasher == flasher) {
    return;
  }
  m_selectedFlasher = flasher;
  Q_EMIT selectedFlasherChanged();

  bool wasUpdClient = m_isUpdClientSelected;
  m_isUpdClientSelected =
      (m_selectedFlasher == QStringLiteral("UpdClient (Network)"));
  if (wasUpdClient != m_isUpdClientSelected) {
    Q_EMIT isUpdClientSelectedChanged();
  }

  if (m_isUpdClientSelected) {
    m_connectedFlasherName = QStringLiteral("UpdClient (Network)");
    m_connectedFlasherImage =
        QStringLiteral("qrc:/qt/qml/org/gxoss/genexis/assets/noflasher.png");
    m_isFlasherConnected = true;
    Q_EMIT flasherNameChanged(m_connectedFlasherName);
    Q_EMIT flasherImageChanged(m_connectedFlasherImage);
    Q_EMIT flasherConnectionChanged(m_isFlasherConnected);
  } else {
    checkUsbDevices();
  }
}

void Flasher::setTargetIp(const QString &ip) {
  if (m_targetIp == ip) {
    return;
  }
  m_targetIp = ip;
  Q_EMIT targetIpChanged();
}

#include "backend/FlasherDeviceRegistry.hpp"

void Flasher::checkUsbDevices() {
  auto devices = USBDiscovery::getAllDevices();
  QString newImage =
      QStringLiteral("qrc:/qt/qml/org/gxoss/genexis/assets/noflasher.png");
  QString newName = QStringLiteral("No Flasher Connected");
  bool isConnected = false;
  QStringList detectedUsb;

  for (const auto &dev : devices) {
    auto profile =
        gxapi::backend::findDeviceByVidPid(dev.vendorId, dev.productId);
    if (profile.has_value()) {
      newImage = QString::fromStdString(profile->imageResource);
      newName = QString::fromStdString(profile->displayName);
      isConnected = true;
      if (!detectedUsb.contains(newName)) {
        detectedUsb.append(newName);
      }
    }
  }

  if (!m_isUpdClientSelected) {
    if (m_connectedFlasherImage != newImage) {
      m_connectedFlasherImage = newImage;
      Q_EMIT flasherImageChanged(m_connectedFlasherImage);
    }
    if (m_connectedFlasherName != newName) {
      m_connectedFlasherName = newName;
      Q_EMIT flasherNameChanged(m_connectedFlasherName);
    }
    if (m_isFlasherConnected != isConnected) {
      m_isFlasherConnected = isConnected;
      Q_EMIT flasherConnectionChanged(m_isFlasherConnected);
    }
  }

  QStringList allOptions = detectedUsb;
  if (allOptions.isEmpty() && !isConnected) {
    allOptions.append(QStringLiteral("None"));
  }
  allOptions.append(QStringLiteral("UpdServ (Network)"));

  if (m_availableFlashers != allOptions) {
    m_availableFlashers = allOptions;
    Q_EMIT availableFlashersChanged();

    if (m_selectedFlasher.isEmpty() ||
        !m_availableFlashers.contains(m_selectedFlasher)) {
      m_selectedFlasher = m_availableFlashers.first();
      Q_EMIT selectedFlasherChanged();
    }
  }
}

void Flasher::searchNetworkDevices() {
  Q_EMIT logOutput(QStringLiteral(
      "[NETWORK] Scanning local subnet for Xbox 360 UpdServer consoles..."));

  std::thread([this]() {
    auto consoles = BackendManager::instance().network().discoverConsoles();
    QStringList devList;
    QString firstIp;

    for (const auto &c : consoles) {
      QString item = QString::fromStdString(c.ipAddress);
      if (!c.consoleType.empty()) {
        item +=
            QStringLiteral(" (%1)").arg(QString::fromStdString(c.consoleType));
      }
      devList.append(item);
      if (firstIp.isEmpty()) {
        firstIp = QString::fromStdString(c.ipAddress);
      }
    }

    QMetaObject::invokeMethod(
        this,
        [this, devList, firstIp]() {
          m_detectedNetworkDevices = devList;
          Q_EMIT detectedNetworkDevicesChanged();

          if (!firstIp.isEmpty() && m_targetIp.isEmpty()) {
            m_targetIp = firstIp;
            Q_EMIT targetIpChanged();
          }

          if (devList.isEmpty()) {
            Q_EMIT logOutput(QStringLiteral(
                "[NETWORK] No UpdServer consoles found on local broadcast."));
          } else {
            Q_EMIT logOutput(
                QStringLiteral("[NETWORK] Discovered %1 console(s).")
                    .arg(devList.size()));
          }
        },
        Qt::QueuedConnection);
  }).detach();
}

void Flasher::detectHardware(const QString &filePath) {
  if (m_isUpdClientSelected) {
    m_detectedHardwareInfo =
        QStringLiteral("Network UpdClient active (Detection N/A)");
    Q_EMIT detectedHardwareInfoChanged();
    return;
  }

  m_isBusy = true;
  Q_EMIT busyStateChanged(m_isBusy);
  Q_EMIT logOutput(
      QStringLiteral("[DETECT] Querying hardware configuration..."));

  std::thread([this, filePath]() {
    QString ext = QFileInfo(filePath).suffix().toLower();
    QString resultStr;
    std::string hwTarget =
        m_isUpdClientSelected ? "UpdClient" : m_selectedFlasher.toStdString();
    auto devProfile = gxapi::backend::findDeviceByName(hwTarget);

    if (ext == QStringLiteral("svf") || ext == QStringLiteral("xsvf")) {
      if (devProfile.has_value() && devProfile->jtagBackendName.empty()) {
        resultStr = QStringLiteral("JTAG not supported by ") +
                    QString::fromStdString(devProfile->displayName);
      } else {
        
        gxapi::backend::JtagDeviceConfig jConfig;
        auto scanRes =
            BackendManager::instance().jtagForHardware(hwTarget).scanChain(
                jConfig);
        if (scanRes.has_value()) {
          std::ostringstream ss;
          ss << "JTAG Chain: " << scanRes->size() << " device(s)";
          for (size_t i = 0; i < scanRes->size(); ++i) {
            uint32_t idcode = (*scanRes)[i];
            std::string partName =
                (idcode == 0x06e5e093 || idcode == 0x06e5c093) ? "XC2C64A"
                                                               : "Unknown JTAG";
            ss << " | Device " << i << ": " << partName << " (IDCODE: 0x"
               << std::hex << std::setw(8) << std::setfill('0') << idcode
               << ")";
          }
          resultStr = QString::fromStdString(ss.str());
        } else {
          resultStr = QStringLiteral("JTAG Scan Failed: ") +
                      QString::fromStdString(scanRes.error());
        }
      }
    } else {
      if (devProfile.has_value() && devProfile->flashBackendName.empty()) {
        resultStr = QStringLiteral("Flash not supported by ") +
                    QString::fromStdString(devProfile->displayName);
      } else {
        
        
        gxapi::backend::FlashDeviceConfig fConfig;
        if (m_isUpdClientSelected) {
          fConfig.ipAddress = m_targetIp.toStdString();
        }
        auto flashRes =
            BackendManager::instance().flashForHardware(hwTarget).getFlashInfo(
                fConfig);
        if (flashRes.has_value()) {
          std::ostringstream ss;
          ss << "Config: 0x" << std::hex << std::setw(8) << std::setfill('0')
             << flashRes->configWord << " | Size: " << std::dec
             << (flashRes->totalBytes / (1024 * 1024)) << "MB"
             << " | " << flashRes->flashType;
          resultStr = QString::fromStdString(ss.str());
        } else {
          resultStr = QStringLiteral("Flash Detect Failed: ") +
                      QString::fromStdString(flashRes.error());
        }
      }
    }

    QMetaObject::invokeMethod(
        this,
        [this, resultStr]() {
          m_isBusy = false;
          Q_EMIT busyStateChanged(m_isBusy);

          m_detectedHardwareInfo = resultStr;
          Q_EMIT detectedHardwareInfoChanged();
          Q_EMIT logOutput(QStringLiteral("[DETECT] ") + resultStr);
        },
        Qt::QueuedConnection);
  }).detach();
}

void Flasher::performOperation(const QString &filePath,
                               const QString &operation,
                               const QVariantMap &options) {
  if (m_isBusy) {
    Q_EMIT logOutput(
        QStringLiteral("[WARN] Flasher operation already in progress."));
    return;
  }

  if (operation == QStringLiteral("Detect")) {
    detectHardware(filePath);
    return;
  }

  QString cleanPath = filePath.trimmed();
  if (cleanPath.startsWith(QStringLiteral("file://"))) {
    cleanPath = QUrl(cleanPath).toLocalFile();
  }

  m_isBusy = true;
  Q_EMIT busyStateChanged(m_isBusy);

  QString op = operation.trimmed();
  Q_EMIT logOutput(QStringLiteral("[INFO] Starting %1 operation...").arg(op));

  std::thread([this, cleanPath, op, options]() {
    QString ext = QFileInfo(cleanPath).suffix().toLower();
    bool isJtag =
        (ext == QStringLiteral("svf") || ext == QStringLiteral("xsvf"));
    std::string hwTarget =
        m_isUpdClientSelected ? "UpdClient" : m_selectedFlasher.toStdString();
    auto devProfile = gxapi::backend::findDeviceByName(hwTarget);

    if (isJtag && devProfile.has_value() &&
        devProfile->jtagBackendName.empty()) {
      QString err = QStringLiteral("JTAG operations are not supported by ") +
                    QString::fromStdString(devProfile->displayName);
      QMetaObject::invokeMethod(
          this,
          [this, err]() {
            m_isBusy = false;
            Q_EMIT busyStateChanged(m_isBusy);
            Q_EMIT logOutput(QStringLiteral("[ERROR] ") + err);
            Q_EMIT operationFinished(false, err);
          },
          Qt::QueuedConnection);
      return;
    }

    if (!isJtag && devProfile.has_value() &&
        devProfile->flashBackendName.empty() && !m_isUpdClientSelected) {
      QString err =
          QStringLiteral("NAND flash operations are not supported by ") +
          QString::fromStdString(devProfile->displayName);
      QMetaObject::invokeMethod(
          this,
          [this, err]() {
            m_isBusy = false;
            Q_EMIT busyStateChanged(m_isBusy);
            Q_EMIT logOutput(QStringLiteral("[ERROR] ") + err);
            Q_EMIT operationFinished(false, err);
          },
          Qt::QueuedConnection);
      return;
    }

    auto progressCb = [this](const gxapi::backend::FlashProgressInfo &p) {
      QMetaObject::invokeMethod(
          this,
          [this, p]() {
            Q_EMIT progressUpdated(p.percentage / 100.0,
                                   QString::fromStdString(p.statusMessage));
          },
          Qt::QueuedConnection);
    };

    gxapi::backend::FlashDeviceConfig fConfig;
    if (m_isUpdClientSelected) {
      fConfig.ipAddress = m_targetIp.toStdString();
    }

    bool success = false;
    QString finishMsg;

    if (isJtag) {
      gxapi::backend::JtagDeviceConfig jConfig;
      auto progCbJtag = [this](const gxapi::backend::JtagProgressInfo &p) {
        QMetaObject::invokeMethod(
            this,
            [this, p]() {
              Q_EMIT progressUpdated(p.percentage / 100.0,
                                     QString::fromStdString(p.statusMessage));
            },
            Qt::QueuedConnection);
      };

      auto res = BackendManager::instance().jtagForHardware(hwTarget).flashCpld(
          cleanPath.toStdString(), jConfig, progCbJtag);
      success = res.has_value();
      finishMsg = success ? QStringLiteral("CPLD timing flashed successfully!")
                          : QString::fromStdString(res.error());
    } else if (op == QStringLiteral("Read")) {
      QString outPath = cleanPath;
      if (outPath.isEmpty()) {
        QString appData = StartupManager::instance().appDataPath();
        QString dumpDir = QDir(appData).filePath(QStringLiteral("output"));
        QDir().mkpath(dumpDir);
        outPath = QDir(dumpDir).filePath(QStringLiteral("nanddump.bin"));
      }
      auto res = BackendManager::instance().flashForHardware(hwTarget).readNand(
          outPath.toStdString(), 0, 0, fConfig, progressCb);
      success = res.has_value();
      finishMsg = success ? QStringLiteral("NAND read complete: ") + outPath
                          : QString::fromStdString(res.error());
    } else if (op == QStringLiteral("Write")) {
      if (cleanPath.isEmpty() || !QFileInfo::exists(cleanPath)) {
        success = false;
        finishMsg = QStringLiteral("Target file not found: ") + cleanPath;
      } else {
        bool verify = options.value(QStringLiteral("verify"), true).toBool();
        auto res =
            BackendManager::instance().flashForHardware(hwTarget).writeNand(
                cleanPath.toStdString(), 0, verify, false, fConfig, progressCb);
        success = res.has_value();
        finishMsg = success ? QStringLiteral("NAND write complete!")
                            : QString::fromStdString(res.error());
      }
    } else if (op == QStringLiteral("Erase")) {
      auto res =
          BackendManager::instance().flashForHardware(hwTarget).eraseNand(
              0, 0, fConfig);
      success = res.has_value();
      finishMsg = success ? QStringLiteral("NAND erased successfully!")
                          : QString::fromStdString(res.error());
    }

    QMetaObject::invokeMethod(
        this,
        [this, success, finishMsg]() {
          m_isBusy = false;
          Q_EMIT busyStateChanged(m_isBusy);
          Q_EMIT logOutput(QStringLiteral("[%1] %2").arg(
              success ? QStringLiteral("SUCCESS") : QStringLiteral("ERROR"),
              finishMsg));
          Q_EMIT operationFinished(success, finishMsg);
        },
        Qt::QueuedConnection);
  }).detach();
}
