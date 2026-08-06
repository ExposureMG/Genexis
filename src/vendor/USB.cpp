#include "vendor/USB.hpp"

namespace gxapi {
namespace USB {

std::vector<USBDeviceInfo> getConnectedDevices() {
  return Genexis::Devices::Vendor::USBStorage::getConnectedDevices();
}

std::string detectFileSystem(const std::string &mountPointOrDevice) {
  return Genexis::Devices::Vendor::USBStorage::detectFileSystem(
      mountPointOrDevice);
}

bool isDeviceMounted(const std::string &mountPointOrDevice) {
  return Genexis::Devices::Vendor::USBStorage::isDeviceMounted(
      mountPointOrDevice);
}

USBDeviceInfo getDeviceInfo(const std::string &mountPointOrDevice) {
  return Genexis::Devices::Vendor::USBStorage::getDeviceInfo(
      mountPointOrDevice);
}

bool readFile(const std::string &mountPoint, const std::string &relativePath,
              std::vector<uint8_t> &outData) {
  return Genexis::Devices::Vendor::USBStorage::readFile(mountPoint,
                                                        relativePath, outData);
}

bool writeFile(const std::string &mountPoint, const std::string &relativePath,
               const std::vector<uint8_t> &data) {
  return Genexis::Devices::Vendor::USBStorage::writeFile(mountPoint,
                                                         relativePath, data);
}

bool createDirectory(const std::string &mountPoint,
                     const std::string &relativePath) {
  return Genexis::Devices::Vendor::USBStorage::createDirectory(mountPoint,
                                                               relativePath);
}

std::vector<std::string> listDirectory(const std::string &mountPoint,
                                       const std::string &relativePath) {
  return Genexis::Devices::Vendor::USBStorage::listDirectory(mountPoint,
                                                             relativePath);
}

bool fileExists(const std::string &mountPoint,
                const std::string &relativePath) {
  return Genexis::Devices::Vendor::USBStorage::fileExists(mountPoint,
                                                          relativePath);
}

uint64_t getFreeSpace(const std::string &mountPoint) {
  return Genexis::Devices::Vendor::USBStorage::getFreeSpace(mountPoint);
}

} // namespace USB
} // namespace gxapi
