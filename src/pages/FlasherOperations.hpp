#pragma once

#include "backend/FlasherDeviceRegistry.hpp"
#include "backend/IFlashService.hpp"
#include "backend/IJtagService.hpp"

#include <QString>

#include <cstdint>
#include <expected>
#include <optional>
#include <string>
#include <vector>

namespace gxapi::pages::detail {

// The hardware an operation runs against. It is copied on the GUI thread so
// that workers never read Flasher members the GUI thread can change.
struct FlasherTarget {
  std::string hardwareName;
  std::optional<backend::FlasherDeviceProfile> profile;
  bool isNetwork{false};
  std::string ipAddress;
};

inline FlasherTarget makeTarget(bool isNetwork, const QString &selectedFlasher,
                                const QString &targetIp) {
  FlasherTarget target;
  target.isNetwork = isNetwork;
  target.hardwareName = isNetwork ? std::string(backend::kUpdClientBackend)
                                  : selectedFlasher.toStdString();
  target.profile = backend::findDeviceByName(target.hardwareName);
  target.ipAddress = targetIp.toStdString();
  return target;
}

// The device's display name when its profile has no backend for the
// operation; hardware without a profile is always attempted.
inline std::optional<QString> unsupportedBy(const FlasherTarget &target,
                                            bool jtag) {
  if (!target.profile) {
    return std::nullopt;
  }
  const std::string &backendName =
      jtag ? target.profile->jtagBackendName : target.profile->flashBackendName;
  if (!backendName.empty()) {
    return std::nullopt;
  }
  return QString::fromStdString(target.profile->displayName);
}

inline backend::JtagDeviceConfig jtagConfig(const FlasherTarget &target) {
  backend::JtagDeviceConfig config;
  if (target.profile) {
    config.backend = target.profile->jtagProbe;
  }
  return config;
}

inline backend::FlashDeviceConfig flashConfig(const FlasherTarget &target) {
  backend::FlashDeviceConfig config;
  if (target.isNetwork) {
    config.ipAddress = target.ipAddress;
  }
  return config;
}

inline QString hex32(std::uint32_t value) {
  return QStringLiteral("%1").arg(value, 8, 16, QLatin1Char('0'));
}

inline QString formatJtagChain(const std::vector<std::uint32_t> &idcodes) {
  QString text = QStringLiteral("JTAG Chain: %1 device(s)")
                     .arg(static_cast<qsizetype>(idcodes.size()));
  for (std::size_t i = 0; i < idcodes.size(); ++i) {
    const auto part = backend::findJtagPartName(idcodes[i]);
    text +=
        QStringLiteral(" | Device %1: %2 (IDCODE: 0x%3)")
            .arg(QString::number(i),
                 part ? QString(QLatin1StringView(part->data(), part->size()))
                      : QStringLiteral("Unknown JTAG"),
                 hex32(idcodes[i]));
  }
  return text;
}

inline QString formatFlashInfo(const backend::FlashInfo &info) {
  return QStringLiteral("Config: 0x%1 | Size: %2MB | %3")
      .arg(hex32(info.configWord),
           QString::number(info.totalBytes / (1024 * 1024)),
           QString::fromStdString(info.flashType));
}

struct OperationRequest {
  QString operation;
  QString path;
  bool verify{true};
  QString appDataPath;
};

struct OperationResult {
  bool success{false};
  QString message;
};

inline OperationResult toResult(const std::expected<void, std::string> &outcome,
                                const QString &successMessage) {
  if (!outcome) {
    return {false, QString::fromStdString(outcome.error())};
  }
  return {true, successMessage};
}

} // namespace gxapi::pages::detail
