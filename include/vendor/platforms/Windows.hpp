#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace Genexis {
namespace Devices {
namespace Vendor {

struct USBDeviceInfo;
struct RawUSBDevice;

class WindowsUSBStorage {
public:
    WindowsUSBStorage() = delete;

    // Detect connected + mounted USB storage devices
    static std::vector<USBDeviceInfo> getConnectedDevices();

    // Detect filesystem for a given mount point or drive letter
    static std::string detectFileSystem(const std::string& mountPointOrDrive);

    // Check if device or mount path is currently mounted
    static bool isDeviceMounted(const std::string& mountPointOrDrive);

    // Retrieve detailed device info for a specific mount point or drive
    static USBDeviceInfo getDeviceInfo(const std::string& mountPointOrDrive);

    // Access mounted USB storage: Read a file relative to mount point
    static bool readFile(const std::string& mountPoint, const std::string& relativePath, std::vector<uint8_t>& outData);

    // Access mounted USB storage: Write data to a file relative to mount point
    static bool writeFile(const std::string& mountPoint, const std::string& relativePath, const std::vector<uint8_t>& data);

    // Access mounted USB storage: Create a directory/folder relative to mount point
    static bool createDirectory(const std::string& mountPoint, const std::string& relativePath);

    // Access mounted USB storage: List directory entries relative to mount point
    static std::vector<std::string> listDirectory(const std::string& mountPoint, const std::string& relativePath = "");

    // Access mounted USB storage: Check if a file or directory exists relative to mount point
    static bool fileExists(const std::string& mountPoint, const std::string& relativePath);

    // Access mounted USB storage: Get free space in bytes for mount point
    static uint64_t getFreeSpace(const std::string& mountPoint);
};

// ---------------------------------------------------------------------------
// BlockDeviceInfo — describes a raw block device visible to the OS.
// Populated by WindowsFilesystem::getBlockDevices().
// Mirrors the definition in UNIX.hpp; both are resolved via USBStorage.hpp.
// ---------------------------------------------------------------------------
struct BlockDeviceInfo {
    std::string device_path;   // e.g. "\\\\.\\PhysicalDrive1"
    std::string fs_type;       // "FAT32", "FATX", "Unknown", …
    std::uint64_t size_bytes{0};
    bool is_removable{false};
};

// ---------------------------------------------------------------------------
// WindowsFilesystem — OS-level filesystem utilities for Windows.
// All methods are static; do not instantiate.
// Used by the Filesystem API layer for host block-device access and
// magic-byte probing without loading whole files into memory.
// Raw device access uses CreateFileA + ReadFile on \\.\PhysicalDriveN paths.
// ---------------------------------------------------------------------------
class WindowsFilesystem {
public:
    WindowsFilesystem() = delete;

    // Enumerate physical drives via SetupDiGetClassDevs / DeviceIoControl.
    static std::vector<BlockDeviceInfo> getBlockDevices();

    // Read `len` raw bytes from `path` starting at `offset`.
    // Works on both regular image files and raw \\.\PhysicalDriveN paths.
    // Returns an empty vector on any I/O error.
    static std::vector<std::byte> readMagic(const std::filesystem::path& path,
                                             std::uint64_t offset,
                                             std::size_t len);
};

// ---------------------------------------------------------------------------
// WindowsUSBDiscovery — OS-level raw USB device enumeration for Windows.
// ---------------------------------------------------------------------------
class WindowsUSBDiscovery {
public:
    WindowsUSBDiscovery() = delete;
    static std::vector<RawUSBDevice> getAllDevices();
};

} // namespace Vendor
} // namespace Devices
} // namespace Genexis
