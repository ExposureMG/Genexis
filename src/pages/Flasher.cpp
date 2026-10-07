#include "pages/Flasher.hpp"

#include "Async.hpp"
#include "FilePaths.hpp"
#include "FlasherOperations.hpp"
#include "StartupManager.hpp"
#include "backend/BackendManager.hpp"
#include "backend/FlasherDeviceRegistry.hpp"
#include "backend/FlasherDiscovery.hpp"

#include <QDir>
#include <QFileInfo>

#include <string>

using BackendManager = gxapi::backend::BackendManager;
using FlasherDiscovery = gxapi::backend::FlasherDiscovery;
namespace backend = gxapi::backend;
namespace detail = gxapi::pages::detail;

namespace {

// The flasher-list entry for the UpdServer network flasher. QML/pages/
// Flasher.qml repeats it in its fallback model.
constexpr QLatin1StringView kNetworkFlasherName{"UpdClient (Network)"};
constexpr QLatin1StringView kNoFlasherImage{
    "qrc:/qt/qml/org/gxoss/genexis/assets/noflasher.png"};

QString detectJtagChain(const detail::FlasherTarget &target) {
  if (const auto device = detail::unsupportedBy(target, true)) {
    return QStringLiteral("JTAG not supported by ") + *device;
  }
  const auto scan = BackendManager::instance()
                        .jtagForHardware(target.hardwareName)
                        .scanChain(detail::jtagConfig(target));
  if (!scan) {
    return QStringLiteral("JTAG Scan Failed: ") +
           QString::fromStdString(scan.error());
  }
  return detail::formatJtagChain(*scan);
}

QString detectFlashInfo(const detail::FlasherTarget &target) {
  if (const auto device = detail::unsupportedBy(target, false)) {
    return QStringLiteral("Flash not supported by ") + *device;
  }
  const auto info = BackendManager::instance()
                        .flashForHardware(target.hardwareName)
                        .getFlashInfo(detail::flashConfig(target));
  if (!info) {
    return QStringLiteral("Flash Detect Failed: ") +
           QString::fromStdString(info.error());
  }
  return detail::formatFlashInfo(*info);
}

detail::OperationResult
programCpld(const detail::FlasherTarget &target, const QString &path,
            const backend::JtagProgressCallback &progress) {
  return detail::toResult(
      BackendManager::instance()
          .jtagForHardware(target.hardwareName)
          .flashCpld(path.toStdString(), detail::jtagConfig(target), progress),
      QStringLiteral("CPLD timing flashed successfully!"));
}

// An unknown operation fails with an empty message.
detail::OperationResult
runNandOperation(const detail::FlasherTarget &target,
                 const detail::OperationRequest &request,
                 const backend::FlashProgressCallback &progress) {
  auto &flash =
      BackendManager::instance().flashForHardware(target.hardwareName);
  const auto config = detail::flashConfig(target);

  if (request.operation == QStringLiteral("Read")) {
    QString outPath = request.path;
    if (outPath.isEmpty()) {
      const QString dumpDir =
          QDir(request.appDataPath).filePath(QStringLiteral("output"));
      QDir().mkpath(dumpDir);
      outPath = QDir(dumpDir).filePath(QStringLiteral("nanddump.bin"));
    }
    return detail::toResult(
        flash.readNand(outPath.toStdString(), 0, 0, config, progress),
        QStringLiteral("NAND read complete: ") + outPath);
  }

  if (request.operation == QStringLiteral("Write")) {
    if (request.path.isEmpty() || !QFileInfo::exists(request.path)) {
      return {false, QStringLiteral("Target file not found: ") + request.path};
    }
    return detail::toResult(flash.writeNand(request.path.toStdString(), 0, true,
                                            request.verify, config, progress),
                            QStringLiteral("NAND write complete!"));
  }

  if (request.operation == QStringLiteral("Erase")) {
    return detail::toResult(flash.eraseNand(0, 0, config),
                            QStringLiteral("NAND erased successfully!"));
  }

  return {};
}

struct DiscoveredConsoles {
  QStringList devices;
  QString firstIp;
};

DiscoveredConsoles findUpdServerConsoles() {
  DiscoveredConsoles found;
  const auto consoles = BackendManager::instance().network().discoverConsoles();
  for (const auto &c : consoles) {
    QString item = QString::fromStdString(c.ipAddress);
    if (!c.consoleType.empty()) {
      item +=
          QStringLiteral(" (%1)").arg(QString::fromStdString(c.consoleType));
    }
    found.devices.append(item);
    if (found.firstIp.isEmpty()) {
      found.firstIp = QString::fromStdString(c.ipAddress);
    }
  }
  return found;
}

} // namespace

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
  syncNetworkSelection();

  if (m_isUpdClientSelected) {
    m_connectedFlasherName = kNetworkFlasherName;
    m_connectedFlasherImage = kNoFlasherImage;
    m_isFlasherConnected = true;
    Q_EMIT flasherNameChanged(m_connectedFlasherName);
    Q_EMIT flasherImageChanged(m_connectedFlasherImage);
    Q_EMIT flasherConnectionChanged(m_isFlasherConnected);
  } else {
    checkUsbDevices();
  }
}

void Flasher::syncNetworkSelection() {
  const bool isNetwork = m_selectedFlasher == kNetworkFlasherName;
  if (m_isUpdClientSelected != isNetwork) {
    m_isUpdClientSelected = isNetwork;
    Q_EMIT isUpdClientSelectedChanged();
  }
}

void Flasher::setTargetIp(const QString &ip) {
  if (m_targetIp == ip) {
    return;
  }
  m_targetIp = ip;
  Q_EMIT targetIpChanged();
}

void Flasher::checkUsbDevices() {
  auto devices = FlasherDiscovery::getAllDevices();
  QString newImage = kNoFlasherImage;
  QString newName = QStringLiteral("No Flasher Connected");
  bool isConnected = false;
  QStringList detectedUsb;

  for (const auto &dev : devices) {
    auto profile = backend::findDeviceByVidPid(dev.vendorId, dev.productId);
    if (profile.has_value()) {
      newImage = QString::fromStdString(profile->imageResource);
      newName = QString::fromStdString(profile->displayName);
      isConnected = true;
      if (!detectedUsb.contains(newName)) {
        detectedUsb.append(newName);
      }
    }
  }

  QStringList allOptions = detectedUsb;
  if (allOptions.isEmpty() && !isConnected) {
    allOptions.append(QStringLiteral("None"));
  }
  allOptions.append(kNetworkFlasherName);

  if (m_availableFlashers != allOptions) {
    m_availableFlashers = allOptions;
    Q_EMIT availableFlashersChanged();

    if (m_selectedFlasher.isEmpty() ||
        !m_availableFlashers.contains(m_selectedFlasher)) {
      m_selectedFlasher = m_availableFlashers.first();
      Q_EMIT selectedFlasherChanged();
      syncNetworkSelection();
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
}

void Flasher::searchNetworkDevices() {
  Q_EMIT logOutput(QStringLiteral(
      "[NETWORK] Scanning local subnet for Xbox 360 UpdServer consoles..."));

  gxapi::runAsync(
      this, findUpdServerConsoles, [this](const DiscoveredConsoles &found) {
        m_detectedNetworkDevices = found.devices;
        Q_EMIT detectedNetworkDevicesChanged();

        if (!found.firstIp.isEmpty() && m_targetIp.isEmpty()) {
          m_targetIp = found.firstIp;
          Q_EMIT targetIpChanged();
        }

        if (found.devices.isEmpty()) {
          Q_EMIT logOutput(
              QStringLiteral("[NETWORK] No UpdServer consoles found on local "
                             "broadcast."));
        } else {
          Q_EMIT logOutput(QStringLiteral("[NETWORK] Discovered %1 console(s).")
                               .arg(found.devices.size()));
        }
      });
}

void Flasher::detectHardware(const QString &filePath) {
  if (m_isUpdClientSelected) {
    m_detectedHardwareInfo =
        QStringLiteral("Network UpdClient active (Detection N/A)");
    Q_EMIT detectedHardwareInfoChanged();
    return;
  }

  if (m_isBusy) {
    Q_EMIT logOutput(
        QStringLiteral("[WARN] Flasher operation already in progress."));
    return;
  }

  setBusy(true);
  Q_EMIT logOutput(
      QStringLiteral("[DETECT] Querying hardware configuration..."));

  const detail::FlasherTarget target =
      detail::makeTarget(m_isUpdClientSelected, m_selectedFlasher, m_targetIp);
  const bool jtag = gxapi::isJtagTimingFile(filePath);
  gxapi::runAsync(
      this,
      [target, jtag]() {
        return jtag ? detectJtagChain(target) : detectFlashInfo(target);
      },
      [this](const QString &result) { finishDetection(result); });
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

  const detail::OperationRequest request{
      .operation = operation.trimmed(),
      .path = gxapi::toLocalPath(filePath.trimmed()),
      .verify = options.value(QStringLiteral("verify"), true).toBool(),
      .appDataPath = StartupManager::instance().appDataPath()};

  setBusy(true);
  Q_EMIT logOutput(
      QStringLiteral("[INFO] Starting %1 operation...").arg(request.operation));

  const detail::FlasherTarget target =
      detail::makeTarget(m_isUpdClientSelected, m_selectedFlasher, m_targetIp);
  gxapi::runAsync(
      this,
      [this, target, request](const gxapi::ReceiverPoster &poster) {
        const bool jtag = gxapi::isJtagTimingFile(request.path);
        if (const auto device = detail::unsupportedBy(target, jtag)) {
          const QString reason =
              jtag ? QStringLiteral("JTAG operations are not supported by ")
                   : QStringLiteral(
                         "NAND flash operations are not supported by ");
          return detail::OperationResult{false, reason + *device};
        }

        const auto reportProgress = [this, poster](const auto &info) {
          poster.post([this, fraction = info.percentage / 100.0,
                       status = QString::fromStdString(info.statusMessage)]() {
            Q_EMIT progressUpdated(fraction, status);
          });
        };
        return jtag ? programCpld(target, request.path, reportProgress)
                    : runNandOperation(target, request, reportProgress);
      },
      [this](const detail::OperationResult &result) {
        finishOperation(result.success, result.message);
      });
}

void Flasher::setBusy(bool busy) {
  m_isBusy = busy;
  Q_EMIT busyStateChanged(m_isBusy);
}

void Flasher::finishDetection(const QString &result) {
  setBusy(false);
  m_detectedHardwareInfo = result;
  Q_EMIT detectedHardwareInfoChanged();
  Q_EMIT logOutput(QStringLiteral("[DETECT] ") + result);
}

void Flasher::finishOperation(bool success, const QString &message) {
  setBusy(false);
  Q_EMIT logOutput(QStringLiteral("[%1] %2").arg(
      success ? QStringLiteral("SUCCESS") : QStringLiteral("ERROR"), message));
  Q_EMIT operationFinished(success, message);
}
