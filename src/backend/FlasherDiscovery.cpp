#include "backend/FlasherDiscovery.hpp"

#include <QSerialPortInfo>
#include <set>

#if defined(__linux__)
#include <filesystem>
#include <fstream>
#include <string>
#endif

namespace gxapi::backend {

std::vector<DiscoveredFlasherDevice> FlasherDiscovery::getAllDevices() {
  std::vector<DiscoveredFlasherDevice> devices;
  std::set<std::pair<uint16_t, uint16_t>> seen;

  const auto ports = QSerialPortInfo::availablePorts();
  for (const auto &port : ports) {
    if (port.hasVendorIdentifier() && port.hasProductIdentifier()) {
      uint16_t vid = port.vendorIdentifier();
      uint16_t pid = port.productIdentifier();
      if (seen.insert({vid, pid}).second) {
        devices.push_back({vid, pid});
      }
    }
  }

#if defined(__linux__)
  std::error_code ec;
  std::filesystem::path sysUsbDir("/sys/bus/usb/devices");
  if (std::filesystem::exists(sysUsbDir, ec)) {
    for (const auto &entry : std::filesystem::directory_iterator(sysUsbDir, ec)) {
      if (ec)
        break;
      std::filesystem::path devPath = entry.path();
      std::ifstream vFile(devPath / "idVendor");
      std::ifstream pFile(devPath / "idProduct");
      if (!vFile.is_open() || !pFile.is_open())
        continue;

      std::string vHex, pHex;
      vFile >> vHex;
      pFile >> pHex;
      if (vHex.empty() || pHex.empty())
        continue;

      try {
        uint16_t vid = static_cast<uint16_t>(std::stoul(vHex, nullptr, 16));
        uint16_t pid = static_cast<uint16_t>(std::stoul(pHex, nullptr, 16));
        if (seen.insert({vid, pid}).second) {
          devices.push_back({vid, pid});
        }
      } catch (...) {
      }
    }
  }
#endif

  return devices;
}

} // namespace gxapi::backend
