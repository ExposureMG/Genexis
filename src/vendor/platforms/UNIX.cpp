#include "vendor/platforms/UNIX.hpp"
#include "vendor/USBStorage.hpp"

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <sys/statvfs.h>
#include <vector>

#if defined(__APPLE__) || defined(__MACH__)
#include <sys/mount.h>
#include <sys/param.h>
#include <sys/ucred.h>
#else
// Linux / POSIX
#include <mntent.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace Genexis {
namespace Devices {
namespace Vendor {

static std::string normalizeUnixFileSystemName(const std::string &rawFs) {
  std::string lower = rawFs;
  std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

  if (lower == "vfat" || lower == "fat" || lower == "fat32" ||
      lower == "msdos" || lower == "fat16") {
    return "FAT32";
  }
  if (lower == "exfat") {
    return "exFAT";
  }
  if (lower == "ntfs" || lower == "ntfs-3g" || lower == "lowntfs-3g") {
    return "NTFS";
  }
  if (lower == "ext2" || lower == "ext3" || lower == "ext4") {
    return "ext4";
  }
  if (lower == "apfs") {
    return "APFS";
  }
  if (lower == "hfs" || lower == "hfsplus") {
    return "HFS+";
  }
  return rawFs;
}

#if defined(__linux__)
static bool isLinuxUsbDevice(const std::string &devPath,
                             const std::string &mountPoint) {
  // 1. Check mount point location
  if (mountPoint.rfind("/media", 0) == 0 ||
      mountPoint.rfind("/run/media", 0) == 0 ||
      mountPoint.rfind("/mnt", 0) == 0) {
    return true;
  }

  // 2. Check /sys/block/<dev>/removable or device subsystem
  if (devPath.rfind("/dev/", 0) == 0) {
    std::string baseDev = devPath.substr(5); // e.g. "sdb1" -> "sdb"
    // Strip partition digits
    size_t pos = baseDev.find_first_of("0123456789");
    if (pos != std::string::npos && pos > 0) {
      baseDev = baseDev.substr(0, pos);
    }

    // Check removable flag in sysfs
    std::string sysRemovablePath = "/sys/block/" + baseDev + "/removable";
    std::ifstream remFile(sysRemovablePath);
    if (remFile.is_open()) {
      int removable = 0;
      remFile >> removable;
      if (removable == 1)
        return true;
    }

    // Check USB subsystem symlink
    std::error_code ec;
    std::filesystem::path sysDevLink = "/sys/block/" + baseDev + "/device";
    if (std::filesystem::exists(sysDevLink, ec)) {
      std::string canonicalPath =
          std::filesystem::canonical(sysDevLink, ec).string();
      if (canonicalPath.find("usb") != std::string::npos) {
        return true;
      }
    }
  }
  return false;
}
#endif

std::vector<USBDeviceInfo> UnixUSBStorage::getConnectedDevices() {
  std::vector<USBDeviceInfo> devices;

#if defined(__APPLE__) || defined(__MACH__)
  struct statfs *mntbuf = nullptr;
  int count = getmntinfo(&mntbuf, MNT_NOWAIT);
  for (int i = 0; i < count; ++i) {
    std::string mountPoint = mntbuf[i].f_mntonname;
    std::string devPath = mntbuf[i].f_mntfromname;
    std::string rawFs = mntbuf[i].f_fstypename;

    // Skip root filesystem and non-volume mounts
    if (mountPoint == "/" || mountPoint.rfind("/System", 0) == 0 ||
        mountPoint.rfind("/dev", 0) == 0) {
      continue;
    }

    bool isVolume = (mountPoint.rfind("/Volumes", 0) == 0);
    if (!isVolume)
      continue;

    struct statvfs vfs{};
    uint64_t totalBytes = 0;
    uint64_t freeBytes = 0;
    if (statvfs(mountPoint.c_str(), &vfs) == 0) {
      totalBytes = static_cast<uint64_t>(vfs.f_blocks) * vfs.f_frsize;
      freeBytes = static_cast<uint64_t>(vfs.f_bavail) * vfs.f_frsize;
    }

    USBDeviceInfo info;
    info.deviceId = devPath;
    info.mountPoint = mountPoint;
    info.volumeName = std::filesystem::path(mountPoint).filename().string();
    info.fileSystem = normalizeUnixFileSystemName(rawFs);
    info.totalBytes = totalBytes;
    info.freeBytes = freeBytes;
    info.isMounted = true;
    info.isRemovable = true;

    devices.push_back(info);
  }
#elif defined(__linux__)
  FILE *mntFile = setmntent("/proc/mounts", "r");
  if (!mntFile) {
    mntFile = setmntent("/etc/mtab", "r");
  }

  if (mntFile) {
    struct mntent *entry = nullptr;
    while ((entry = getmntent(mntFile)) != nullptr) {
      std::string devPath = entry->mnt_fsname;
      std::string mountPoint = entry->mnt_dir;
      std::string rawFs = entry->mnt_type;

      // Only inspect actual block devices
      if (devPath.rfind("/dev/", 0) != 0)
        continue;

      if (isLinuxUsbDevice(devPath, mountPoint)) {
        struct statvfs vfs{};
        uint64_t totalBytes = 0;
        uint64_t freeBytes = 0;
        if (statvfs(mountPoint.c_str(), &vfs) == 0) {
          totalBytes = static_cast<uint64_t>(vfs.f_blocks) * vfs.f_frsize;
          freeBytes = static_cast<uint64_t>(vfs.f_bavail) * vfs.f_frsize;
        }

        USBDeviceInfo info;
        info.deviceId = devPath;
        info.mountPoint = mountPoint;
        info.volumeName = std::filesystem::path(mountPoint).filename().string();
        info.fileSystem = normalizeUnixFileSystemName(rawFs);
        info.totalBytes = totalBytes;
        info.freeBytes = freeBytes;
        info.isMounted = true;
        info.isRemovable = true;

        devices.push_back(info);
      }
    }
    endmntent(mntFile);
  }
#endif

  return devices;
}

std::string
UnixUSBStorage::detectFileSystem(const std::string &mountPointOrDevice) {
#if defined(__linux__)
  FILE *mntFile = setmntent("/proc/mounts", "r");
  if (mntFile) {
    struct mntent *entry = nullptr;
    while ((entry = getmntent(mntFile)) != nullptr) {
      if (mountPointOrDevice == entry->mnt_dir ||
          mountPointOrDevice == entry->mnt_fsname) {
        std::string fs = normalizeUnixFileSystemName(entry->mnt_type);
        endmntent(mntFile);
        return fs;
      }
    }
    endmntent(mntFile);
  }
#elif defined(__APPLE__) || defined(__MACH__)
  struct statfs *mntbuf = nullptr;
  int count = getmntinfo(&mntbuf, MNT_NOWAIT);
  for (int i = 0; i < count; ++i) {
    if (mountPointOrDevice == mntbuf[i].f_mntonname ||
        mountPointOrDevice == mntbuf[i].f_mntfromname) {
      return normalizeUnixFileSystemName(mntbuf[i].f_fstypename);
    }
  }
#endif

  return "Unknown";
}

bool UnixUSBStorage::isDeviceMounted(const std::string &mountPointOrDevice) {
  std::error_code ec;
  if (std::filesystem::exists(mountPointOrDevice, ec) &&
      std::filesystem::is_directory(mountPointOrDevice, ec)) {
    struct statvfs vfs{};
    if (statvfs(mountPointOrDevice.c_str(), &vfs) == 0) {
      return true;
    }
  }

#if defined(__linux__)
  FILE *mntFile = setmntent("/proc/mounts", "r");
  if (mntFile) {
    struct mntent *entry = nullptr;
    while ((entry = getmntent(mntFile)) != nullptr) {
      if (mountPointOrDevice == entry->mnt_dir ||
          mountPointOrDevice == entry->mnt_fsname) {
        endmntent(mntFile);
        return true;
      }
    }
    endmntent(mntFile);
  }
#elif defined(__APPLE__) || defined(__MACH__)
  struct statfs *mntbuf = nullptr;
  int count = getmntinfo(&mntbuf, MNT_NOWAIT);
  for (int i = 0; i < count; ++i) {
    if (mountPointOrDevice == mntbuf[i].f_mntonname ||
        mountPointOrDevice == mntbuf[i].f_mntfromname) {
      return true;
    }
  }
#endif

  return false;
}

USBDeviceInfo
UnixUSBStorage::getDeviceInfo(const std::string &mountPointOrDevice) {
  auto devices = getConnectedDevices();
  for (const auto &dev : devices) {
    if (dev.mountPoint == mountPointOrDevice ||
        dev.deviceId == mountPointOrDevice) {
      return dev;
    }
  }

  USBDeviceInfo emptyInfo;
  emptyInfo.mountPoint = mountPointOrDevice;
  emptyInfo.fileSystem = detectFileSystem(mountPointOrDevice);
  emptyInfo.isMounted = isDeviceMounted(mountPointOrDevice);
  emptyInfo.freeBytes = getFreeSpace(mountPointOrDevice);
  return emptyInfo;
}

bool UnixUSBStorage::readFile(const std::string &mountPoint,
                              const std::string &relativePath,
                              std::vector<uint8_t> &outData) {
  std::filesystem::path fullPath =
      std::filesystem::path(mountPoint) / relativePath;
  std::ifstream file(fullPath, std::ios::binary | std::ios::ate);
  if (!file.is_open())
    return false;

  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);

  outData.resize(static_cast<size_t>(size));
  if (size > 0 && !file.read(reinterpret_cast<char *>(outData.data()), size)) {
    outData.clear();
    return false;
  }
  return true;
}

bool UnixUSBStorage::writeFile(const std::string &mountPoint,
                               const std::string &relativePath,
                               const std::vector<uint8_t> &data) {
  std::filesystem::path fullPath =
      std::filesystem::path(mountPoint) / relativePath;
  std::error_code ec;
  if (fullPath.has_parent_path()) {
    std::filesystem::create_directories(fullPath.parent_path(), ec);
  }

  std::ofstream file(fullPath, std::ios::binary | std::ios::trunc);
  if (!file.is_open())
    return false;

  if (!data.empty()) {
    file.write(reinterpret_cast<const char *>(data.data()), data.size());
  }
  return file.good();
}

bool UnixUSBStorage::createDirectory(const std::string &mountPoint,
                                     const std::string &relativePath) {
  std::filesystem::path fullPath =
      std::filesystem::path(mountPoint) / relativePath;
  std::error_code ec;
  return std::filesystem::create_directories(fullPath, ec);
}

std::vector<std::string>
UnixUSBStorage::listDirectory(const std::string &mountPoint,
                              const std::string &relativePath) {
  std::vector<std::string> entries;
  std::filesystem::path fullPath =
      relativePath.empty() ? std::filesystem::path(mountPoint)
                           : std::filesystem::path(mountPoint) / relativePath;

  std::error_code ec;
  if (!std::filesystem::exists(fullPath, ec) ||
      !std::filesystem::is_directory(fullPath, ec)) {
    return entries;
  }

  for (const auto &entry : std::filesystem::directory_iterator(fullPath, ec)) {
    entries.push_back(entry.path().filename().string());
  }
  return entries;
}

bool UnixUSBStorage::fileExists(const std::string &mountPoint,
                                const std::string &relativePath) {
  std::filesystem::path fullPath =
      std::filesystem::path(mountPoint) / relativePath;
  std::error_code ec;
  return std::filesystem::exists(fullPath, ec);
}

uint64_t UnixUSBStorage::getFreeSpace(const std::string &mountPoint) {
  struct statvfs vfs{};
  if (statvfs(mountPoint.c_str(), &vfs) == 0) {
    return static_cast<uint64_t>(vfs.f_bavail) * vfs.f_frsize;
  }
  std::error_code ec;
  auto spaceInfo = std::filesystem::space(mountPoint, ec);
  return ec ? 0 : spaceInfo.available;
}

} // namespace Vendor
} // namespace Devices
} // namespace Genexis

// ---------------------------------------------------------------------------
// UnixFilesystem implementation
// ---------------------------------------------------------------------------

#include <fstream>

namespace Genexis {
namespace Devices {
namespace Vendor {

std::vector<std::byte>
UnixFilesystem::readMagic(const std::filesystem::path &path,
                          std::uint64_t offset, std::size_t len) {
  std::ifstream f(path, std::ios::binary);
  if (!f)
    return {};
  f.seekg(static_cast<std::streamoff>(offset), std::ios::beg);
  if (!f)
    return {};
  std::vector<std::byte> buf(len);
  f.read(reinterpret_cast<char *>(buf.data()),
         static_cast<std::streamsize>(len));
  buf.resize(static_cast<std::size_t>(f.gcount()));
  return buf;
}

std::vector<BlockDeviceInfo> UnixFilesystem::getBlockDevices() {
  std::vector<BlockDeviceInfo> results;

#if defined(__linux__)
  // Enumerate /sys/block — each subdirectory is a block device.
  std::error_code ec;
  const std::filesystem::path sysblock{"/sys/block"};
  if (!std::filesystem::exists(sysblock, ec))
    return results;

  for (const auto &entry : std::filesystem::directory_iterator(sysblock, ec)) {
    BlockDeviceInfo info;
    info.device_path = "/dev/" + entry.path().filename().string();

    // Check removable flag
    std::ifstream rem(entry.path() / "removable");
    int removable = 0;
    if (rem)
      rem >> removable;
    info.is_removable = (removable == 1);

    // Read size in 512-byte sectors
    std::ifstream sz(entry.path() / "size");
    std::uint64_t sectors = 0;
    if (sz)
      sz >> sectors;
    info.size_bytes = sectors * 512;

    // Detect filesystem type via /proc/mounts
    info.fs_type = UnixUSBStorage::detectFileSystem(info.device_path);
    if (info.fs_type.empty())
      info.fs_type = "Unknown";

    results.push_back(std::move(info));
  }
#elif defined(__APPLE__) || defined(__MACH__)
  struct statfs *mntbuf = nullptr;
  int count = getmntinfo(&mntbuf, MNT_NOWAIT);
  for (int i = 0; i < count; ++i) {
    BlockDeviceInfo info;
    info.device_path = mntbuf[i].f_mntfromname;
    info.fs_type = normalizeUnixFileSystemName(mntbuf[i].f_fstypename);
    info.is_removable = true;

    struct statvfs vfs{};
    if (statvfs(mntbuf[i].f_mntonname, &vfs) == 0)
      info.size_bytes = static_cast<std::uint64_t>(vfs.f_blocks) * vfs.f_frsize;

    results.push_back(std::move(info));
  }
#endif

  return results;
}

} // namespace Vendor
} // namespace Devices
} // namespace Genexis
