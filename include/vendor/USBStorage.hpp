#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace Genexis {
namespace Devices {
namespace Vendor {

struct USBDeviceInfo {
    std::string deviceId;       // Physical or logical OS device identifier (e.g., "\\.\PhysicalDrive1", "/dev/sdb1", "/dev/disk2s1")
    std::string mountPoint;     // Mounted path (e.g., "E:\", "/media/user/USB", "/Volumes/USB")
    std::string volumeName;     // Volume label / disk name
    std::string fileSystem;     // Filesystem format (e.g., "FAT32", "exFAT", "NTFS", "ext4", "apfs")
    uint64_t totalBytes{0};     // Total storage capacity in bytes
    uint64_t freeBytes{0};      // Free storage space available in bytes
    bool isMounted{false};      // Whether device is currently mounted
    bool isRemovable{true};     // Whether device is reported as removable storage
    std::string vendorId;       // USB Vendor ID string if available
    std::string productId;      // USB Product ID string if available
    std::string serialNumber;   // Serial number string if available
};

} // namespace Vendor
} // namespace Devices
} // namespace Genexis

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#include "vendor/platforms/Windows.hpp"
namespace Genexis {
namespace Devices {
namespace Vendor {
using USBStorage = WindowsUSBStorage;
} // namespace Vendor
} // namespace Devices
} // namespace Genexis
#else
#include "vendor/platforms/UNIX.hpp"
namespace Genexis {
namespace Devices {
namespace Vendor {
using USBStorage = UnixUSBStorage;
} // namespace Vendor
} // namespace Devices
} // namespace Genexis
#endif
