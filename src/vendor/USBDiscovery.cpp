#include "vendor/USBDiscovery.hpp"

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#include "vendor/platforms/Windows.hpp"
using PlatformDiscovery = Genexis::Devices::Vendor::WindowsUSBDiscovery;
#else
#include "vendor/platforms/UNIX.hpp"
using PlatformDiscovery = Genexis::Devices::Vendor::UnixUSBDiscovery;
#endif

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>

namespace Genexis {
namespace Devices {
namespace Vendor {

namespace {

std::string toLowerTrimmed(const std::string &str) {
  std::string result;
  for (char c : str) {
    if (!std::isspace(static_cast<unsigned char>(c))) {
      result += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
  }
  return result;
}

uint16_t hexToUint16(const std::string &hexStr) {
  if (hexStr.empty())
    return 0;
  try {
    return static_cast<uint16_t>(std::stoul(hexStr, nullptr, 16));
  } catch (...) {
    return 0;
  }
}

} // namespace

std::vector<RawUSBDevice> USBDiscovery::getAllDevices() {
  return PlatformDiscovery::getAllDevices();
}

std::vector<RawUSBDevice> USBDiscovery::findDevicesByVidPid(uint16_t vid,
                                                            uint16_t pid) {
  std::vector<RawUSBDevice> result;
  auto all = getAllDevices();
  for (const auto &dev : all) {
    if (dev.vendorId == vid && dev.productId == pid) {
      result.push_back(dev);
    }
  }
  return result;
}

std::vector<RawUSBDevice>
USBDiscovery::findDevicesByVidPid(const std::string &vidHex,
                                  const std::string &pidHex) {
  std::string vLower = toLowerTrimmed(vidHex);
  std::string pLower = toLowerTrimmed(pidHex);
  uint16_t vid = hexToUint16(vLower);
  uint16_t pid = hexToUint16(pLower);

  std::vector<RawUSBDevice> result;
  auto all = getAllDevices();
  for (const auto &dev : all) {
    bool matchVid = (vLower.empty() || dev.vendorHex == vLower ||
                     (vid != 0 && dev.vendorId == vid));
    bool matchPid = (pLower.empty() || dev.productHex == pLower ||
                     (pid != 0 && dev.productId == pid));
    if (matchVid && matchPid) {
      result.push_back(dev);
    }
  }
  return result;
}

bool USBDiscovery::isDeviceConnected(uint16_t vid, uint16_t pid) {
  return !findDevicesByVidPid(vid, pid).empty();
}

} // namespace Vendor
} // namespace Devices
} // namespace Genexis
