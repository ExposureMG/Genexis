#pragma once

#include "vendor/USBStorage.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace gxapi {
namespace USB {

using USBDeviceInfo = Genexis::Devices::Vendor::USBDeviceInfo;

// Retrieve list of connected and mounted USB storage devices
std::vector<USBDeviceInfo> getConnectedDevices();

// Detect filesystem format of a mounted USB device
std::string detectFileSystem(const std::string &mountPointOrDevice);

// Check if a USB device or mount point is mounted
bool isDeviceMounted(const std::string &mountPointOrDevice);

// Retrieve detailed metadata for a USB device
USBDeviceInfo getDeviceInfo(const std::string &mountPointOrDevice);

// Read a file from the mounted USB storage
bool readFile(const std::string &mountPoint, const std::string &relativePath,
              std::vector<uint8_t> &outData);

// Write binary data to a file on the mounted USB storage
bool writeFile(const std::string &mountPoint, const std::string &relativePath,
               const std::vector<uint8_t> &data);

// Create a directory/folder on the mounted USB storage
bool createDirectory(const std::string &mountPoint,
                     const std::string &relativePath);

// List entries in a directory on the mounted USB storage
std::vector<std::string> listDirectory(const std::string &mountPoint,
                                       const std::string &relativePath = "");

// Check if a file or directory exists on the mounted USB storage
bool fileExists(const std::string &mountPoint, const std::string &relativePath);

// Query free storage space in bytes on the mounted USB storage
uint64_t getFreeSpace(const std::string &mountPoint);

} // namespace USB
} // namespace gxapi
