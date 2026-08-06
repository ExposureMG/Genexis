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

class UnixUSBStorage {
public:
    UnixUSBStorage() = delete;

    // Detect connected + mounted USB storage devices
    static std::vector<USBDeviceInfo> getConnectedDevices();

    // Detect filesystem for a given mount point or device path
    static std::string detectFileSystem(const std::string& mountPointOrDevice);

    // Check if device or mount path is currently mounted
    static bool isDeviceMounted(const std::string& mountPointOrDevice);

    // Retrieve detailed device info for a specific mount point or device
    static USBDeviceInfo getDeviceInfo(const std::string& mountPointOrDevice);

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
// Populated by UnixFilesystem::getBlockDevices().
// ---------------------------------------------------------------------------
struct BlockDeviceInfo {
    std::string device_path;   // e.g. "/dev/sdb"
    std::string fs_type;       // "FAT32", "FATX", "Unknown", …
    std::uint64_t size_bytes{0};
    bool is_removable{false};
};

// ---------------------------------------------------------------------------
// UnixFilesystem — OS-level filesystem utilities for Linux / macOS.
// All methods are static; do not instantiate.
// Used by the Filesystem API layer for host block-device access and
// magic-byte probing without loading whole files into memory.
// ---------------------------------------------------------------------------
class UnixFilesystem {
public:
    UnixFilesystem() = delete;

    // Enumerate block devices visible via /sys/block (Linux) or iokit (macOS).
    // Returns devices including removable storage.
    static std::vector<BlockDeviceInfo> getBlockDevices();

    // Read `len` raw bytes from `path` starting at `offset`.
    // Used to probe magic bytes of image files or raw block devices
    // without loading the entire file into memory.
    // Returns an empty vector on any I/O error.
    static std::vector<std::byte> readMagic(const std::filesystem::path& path,
                                             std::uint64_t offset,
                                             std::size_t len);
};

} // namespace Vendor
} // namespace Devices
} // namespace Genexis
