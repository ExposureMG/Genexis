#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace Genexis {
namespace Devices {
namespace Vendor {

struct RawUSBDevice {
    uint16_t vendorId{0};       // USB Vendor ID (e.g. 0x2E8A)
    uint16_t productId{0};      // USB Product ID (e.g. 0x000A)
    std::string vendorHex;      // Lowercased 4-digit hex string (e.g. "2e8a")
    std::string productHex;     // Lowercased 4-digit hex string (e.g. "000a")
    std::string manufacturer;   // Vendor string if available (e.g. "Raspberry Pi")
    std::string productName;    // Product name if available (e.g. "PicoFlasher")
    std::string serialNumber;   // Serial number string if available
    std::string devicePath;     // OS physical device path or USB bus node
    bool isMassStorage{false};  // True if device exposes USB mass storage interface
};

class USBDiscovery {
public:
    USBDiscovery() = delete;

    /// Enumerate all raw USB devices currently connected to the system.
    static std::vector<RawUSBDevice> getAllDevices();

    /// Find connected USB devices matching numeric Vendor ID and Product ID.
    static std::vector<RawUSBDevice> findDevicesByVidPid(uint16_t vid, uint16_t pid);

    /// Find connected USB devices matching hex string Vendor ID and Product ID (e.g. "2e8a", "000a").
    static std::vector<RawUSBDevice> findDevicesByVidPid(const std::string &vidHex, const std::string &pidHex);

    /// Returns true if at least one connected USB device matches the specified VID and PID.
    static bool isDeviceConnected(uint16_t vid, uint16_t pid);
};

} // namespace Vendor
} // namespace Devices
} // namespace Genexis
