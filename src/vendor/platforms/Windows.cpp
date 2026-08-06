#include "vendor/platforms/Windows.hpp"
#include "vendor/USBStorage.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#include <fileapi.h>
#include <windows.h>
#include <winioctl.h>

namespace Genexis {
namespace Devices {
namespace Vendor {

static std::string wcharToString(const wchar_t *wstr) {
  if (!wstr)
    return "";
  int sizeNeeded =
      WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
  if (sizeNeeded <= 0)
    return "";
  std::string str(sizeNeeded - 1, 0);
  WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &str[0], sizeNeeded, NULL, NULL);
  return str;
}

static std::wstring stringToWchar(const std::string &str) {
  if (str.empty())
    return L"";
  int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
  if (sizeNeeded <= 0)
    return L"";
  std::wstring wstr(sizeNeeded - 1, 0);
  MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], sizeNeeded);
  return wstr;
}

static std::string normalizeFileSystemName(const std::string &rawFs) {
  std::string upper = rawFs;
  std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
  if (upper == "FAT32" || upper == "FAT" || upper == "FAT16")
    return "FAT32";
  if (upper == "EXFAT")
    return "exFAT";
  if (upper == "NTFS")
    return "NTFS";
  return rawFs;
}

std::vector<USBDeviceInfo> WindowsUSBStorage::getConnectedDevices() {
  std::vector<USBDeviceInfo> devices;

  DWORD drivesBitmask = GetLogicalDrives();
  if (drivesBitmask == 0)
    return devices;

  wchar_t driveBuffer[256];
  DWORD bufferLen = GetLogicalDriveStringsW(256, driveBuffer);
  if (bufferLen == 0 || bufferLen > 256)
    return devices;

  wchar_t *drivePtr = driveBuffer;
  while (*drivePtr) {
    UINT driveType = GetDriveTypeW(drivePtr);

    // We look for removable drives or fixed USB drives
    bool isCandidate = (driveType == DRIVE_REMOVABLE);

    std::wstring driveRootW = drivePtr;              // e.g. L"E:\"
    std::string driveRoot = wcharToString(drivePtr); // e.g. "E:\"

    // Check storage bus type via IOCTL to confirm USB
    std::wstring volumeDevicePath =
        L"\\\\.\\" + driveRootW.substr(0, 2); // L"\\.\E:"
    HANDLE hDevice = CreateFileW(volumeDevicePath.c_str(), 0,
                                 FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                                 OPEN_EXISTING, 0, NULL);

    bool isUsbBus = false;
    std::string vendorIdStr;
    std::string productIdStr;
    std::string serialNumStr;
    std::string deviceIdStr = driveRoot;

    if (hDevice != INVALID_HANDLE_VALUE) {
      STORAGE_PROPERTY_QUERY query{};
      query.PropertyId = StorageDeviceProperty;
      query.QueryType = PropertyStandardQuery;

      BYTE outBuffer[1024] = {0};
      DWORD bytesReturned = 0;
      if (DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY, &query,
                          sizeof(query), outBuffer, sizeof(outBuffer),
                          &bytesReturned, NULL)) {
        STORAGE_DEVICE_DESCRIPTOR *desc =
            reinterpret_cast<STORAGE_DEVICE_DESCRIPTOR *>(outBuffer);
        if (desc->BusType == BusTypeUsb) {
          isUsbBus = true;
        }
        if (desc->VendorIdOffset > 0 && desc->VendorIdOffset < bytesReturned) {
          vendorIdStr =
              reinterpret_cast<char *>(outBuffer + desc->VendorIdOffset);
        }
        if (desc->ProductIdOffset > 0 &&
            desc->ProductIdOffset < bytesReturned) {
          productIdStr =
              reinterpret_cast<char *>(outBuffer + desc->ProductIdOffset);
        }
        if (desc->SerialNumberOffset > 0 &&
            desc->SerialNumberOffset < bytesReturned) {
          serialNumStr =
              reinterpret_cast<char *>(outBuffer + desc->SerialNumberOffset);
        }

        // Query device number if available
        STORAGE_DEVICE_NUMBER devNum{};
        if (DeviceIoControl(hDevice, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0,
                            &devNum, sizeof(devNum), &bytesReturned, NULL)) {
          deviceIdStr =
              "\\\\.\\PhysicalDrive" + std::to_string(devNum.DeviceNumber);
        }
      }
      CloseHandle(hDevice);
    }

    if (isCandidate || isUsbBus) {
      wchar_t volNameBuffer[MAX_PATH + 1] = {0};
      wchar_t fsNameBuffer[MAX_PATH + 1] = {0};
      DWORD volSerial = 0;
      DWORD maxCompLen = 0;
      DWORD fsFlags = 0;

      BOOL volOK = GetVolumeInformationW(drivePtr, volNameBuffer, MAX_PATH + 1,
                                         &volSerial, &maxCompLen, &fsFlags,
                                         fsNameBuffer, MAX_PATH + 1);

      if (volOK) {
        ULARGE_INTEGER freeBytesAvail{};
        ULARGE_INTEGER totalBytes{};
        ULARGE_INTEGER totalFreeBytes{};

        GetDiskFreeSpaceExW(drivePtr, &freeBytesAvail, &totalBytes,
                            &totalFreeBytes);

        USBDeviceInfo info;
        info.deviceId = deviceIdStr;
        info.mountPoint = driveRoot;
        info.volumeName = wcharToString(volNameBuffer);
        info.fileSystem = normalizeFileSystemName(wcharToString(fsNameBuffer));
        info.totalBytes = totalBytes.QuadPart;
        info.freeBytes = freeBytesAvail.QuadPart;
        info.isMounted = true;
        info.isRemovable = (driveType == DRIVE_REMOVABLE || isUsbBus);
        info.vendorId = vendorIdStr;
        info.productId = productIdStr;
        info.serialNumber = serialNumStr;

        devices.push_back(info);
      }
    }

    drivePtr += wcslen(drivePtr) + 1;
  }

  return devices;
}

std::string
WindowsUSBStorage::detectFileSystem(const std::string &mountPointOrDrive) {
  std::wstring wDrive = stringToWchar(mountPointOrDrive);
  if (wDrive.length() >= 2 && wDrive[1] == L':') {
    wDrive = wDrive.substr(0, 2) + L"\\";
  }

  wchar_t volNameBuffer[MAX_PATH + 1] = {0};
  wchar_t fsNameBuffer[MAX_PATH + 1] = {0};
  DWORD volSerial = 0, maxCompLen = 0, fsFlags = 0;

  if (GetVolumeInformationW(wDrive.c_str(), volNameBuffer, MAX_PATH + 1,
                            &volSerial, &maxCompLen, &fsFlags, fsNameBuffer,
                            MAX_PATH + 1)) {
    return normalizeFileSystemName(wcharToString(fsNameBuffer));
  }

  return "Unknown";
}

bool WindowsUSBStorage::isDeviceMounted(const std::string &mountPointOrDrive) {
  std::wstring wDrive = stringToWchar(mountPointOrDrive);
  if (wDrive.length() >= 2 && wDrive[1] == L':') {
    wDrive = wDrive.substr(0, 2) + L"\\";
  }
  UINT driveType = GetDriveTypeW(wDrive.c_str());
  return (driveType != DRIVE_UNKNOWN && driveType != DRIVE_NO_ROOT_DIR);
}

USBDeviceInfo
WindowsUSBStorage::getDeviceInfo(const std::string &mountPointOrDrive) {
  auto devices = getConnectedDevices();
  std::string normalizedTarget = mountPointOrDrive;
  std::transform(normalizedTarget.begin(), normalizedTarget.end(),
                 normalizedTarget.begin(), ::toupper);

  for (const auto &dev : devices) {
    std::string normMount = dev.mountPoint;
    std::transform(normMount.begin(), normMount.end(), normMount.begin(),
                   ::toupper);
    if (normMount.rfind(normalizedTarget, 0) == 0 ||
        normalizedTarget.rfind(normMount, 0) == 0) {
      return dev;
    }
  }

  USBDeviceInfo emptyInfo;
  emptyInfo.mountPoint = mountPointOrDrive;
  emptyInfo.fileSystem = detectFileSystem(mountPointOrDrive);
  emptyInfo.isMounted = isDeviceMounted(mountPointOrDrive);
  emptyInfo.freeBytes = getFreeSpace(mountPointOrDrive);
  return emptyInfo;
}

bool WindowsUSBStorage::readFile(const std::string &mountPoint,
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

bool WindowsUSBStorage::writeFile(const std::string &mountPoint,
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

bool WindowsUSBStorage::createDirectory(const std::string &mountPoint,
                                        const std::string &relativePath) {
  std::filesystem::path fullPath =
      std::filesystem::path(mountPoint) / relativePath;
  std::error_code ec;
  return std::filesystem::create_directories(fullPath, ec);
}

std::vector<std::string>
WindowsUSBStorage::listDirectory(const std::string &mountPoint,
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

bool WindowsUSBStorage::fileExists(const std::string &mountPoint,
                                   const std::string &relativePath) {
  std::filesystem::path fullPath =
      std::filesystem::path(mountPoint) / relativePath;
  std::error_code ec;
  return std::filesystem::exists(fullPath, ec);
}

uint64_t WindowsUSBStorage::getFreeSpace(const std::string &mountPoint) {
  std::wstring wDrive = stringToWchar(mountPoint);
  ULARGE_INTEGER freeBytesAvail{}, totalBytes{}, totalFreeBytes{};
  if (GetDiskFreeSpaceExW(wDrive.c_str(), &freeBytesAvail, &totalBytes,
                          &totalFreeBytes)) {
    return freeBytesAvail.QuadPart;
  }
  std::error_code ec;
  auto spaceInfo = std::filesystem::space(mountPoint, ec);
  return ec ? 0 : spaceInfo.available;
}

} // namespace Vendor
} // namespace Devices
} // namespace Genexis

#else // Fallback stub if compiled on non-Windows

namespace Genexis {
namespace Devices {
namespace Vendor {

std::vector<USBDeviceInfo> WindowsUSBStorage::getConnectedDevices() {
  return {};
}
std::string WindowsUSBStorage::detectFileSystem(const std::string &) {
  return "Unknown";
}
bool WindowsUSBStorage::isDeviceMounted(const std::string &) { return false; }
USBDeviceInfo WindowsUSBStorage::getDeviceInfo(const std::string &) {
  return {};
}
bool WindowsUSBStorage::readFile(const std::string &, const std::string &,
                                 std::vector<uint8_t> &) {
  return false;
}
bool WindowsUSBStorage::writeFile(const std::string &, const std::string &,
                                  const std::vector<uint8_t> &) {
  return false;
}
bool WindowsUSBStorage::createDirectory(const std::string &,
                                        const std::string &) {
  return false;
}
std::vector<std::string> WindowsUSBStorage::listDirectory(const std::string &,
                                                          const std::string &) {
  return {};
}
bool WindowsUSBStorage::fileExists(const std::string &, const std::string &) {
  return false;
}
uint64_t WindowsUSBStorage::getFreeSpace(const std::string &) { return 0; }

} // namespace Vendor
} // namespace Devices
} // namespace Genexis

#endif

// ---------------------------------------------------------------------------
// WindowsFilesystem implementation (Windows-only)
// ---------------------------------------------------------------------------

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <devguid.h>
#include <ntdddisk.h>
#include <setupapi.h>
#include <windows.h>

#include <fstream>
#include <sstream>

namespace Genexis {
namespace Devices {
namespace Vendor {

std::vector<std::byte>
WindowsFilesystem::readMagic(const std::filesystem::path &path,
                             std::uint64_t offset, std::size_t len) {
  // Use CreateFileA so it also works on \\.\PhysicalDriveN paths.
  HANDLE h = CreateFileA(path.string().c_str(), GENERIC_READ,
                         FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                         OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
  if (h == INVALID_HANDLE_VALUE)
    return {};

  LARGE_INTEGER li{};
  li.QuadPart = static_cast<LONGLONG>(offset);
  if (!SetFilePointerEx(h, li, nullptr, FILE_BEGIN)) {
    CloseHandle(h);
    return {};
  }

  std::vector<std::byte> buf(len);
  DWORD read_bytes = 0;
  bool ok = ReadFile(h, buf.data(), static_cast<DWORD>(len), &read_bytes,
                     nullptr) != FALSE;
  CloseHandle(h);

  if (!ok)
    return {};
  buf.resize(read_bytes);
  return buf;
}

std::vector<BlockDeviceInfo> WindowsFilesystem::getBlockDevices() {
  std::vector<BlockDeviceInfo> results;

  // Probe \\.\PhysicalDrive0 … PhysicalDrive15 (arbitrary upper bound).
  for (int i = 0; i < 16; ++i) {
    std::string dev = "\\\\.\\PhysicalDrive" + std::to_string(i);

    HANDLE h = CreateFileA(dev.c_str(), GENERIC_READ,
                           FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE)
      break;

    BlockDeviceInfo info;
    info.device_path = dev;

    // Query disk size
    GET_LENGTH_INFORMATION gli{};
    DWORD returned = 0;
    if (DeviceIoControl(h, IOCTL_DISK_GET_LENGTH_INFO, nullptr, 0, &gli,
                        sizeof(gli), &returned, nullptr))
      info.size_bytes = static_cast<std::uint64_t>(gli.Length.QuadPart);

    // Query removable flag
    STORAGE_HOTPLUG_INFO hpi{};
    if (DeviceIoControl(h, IOCTL_STORAGE_GET_HOTPLUG_INFO, nullptr, 0, &hpi,
                        sizeof(hpi), &returned, nullptr))
      info.is_removable = (hpi.MediaRemovable != 0);

    CloseHandle(h);

    // Probe first 4 bytes for filesystem magic (best effort).
    auto magic = readMagic(dev, 0, 4);
    if (magic.size() == 4) {
      if (magic[0] == std::byte{0xEB} || magic[0] == std::byte{0xFA})
        info.fs_type = "FAT32";
      else if (magic[0] == std::byte{'F'} && magic[1] == std::byte{'A'} &&
               magic[2] == std::byte{'T'} && magic[3] == std::byte{'X'})
        info.fs_type = "FATX";
      else
        info.fs_type = "Unknown";
    }

    results.push_back(std::move(info));
  }

  return results;
}

} // namespace Vendor
} // namespace Devices
} // namespace Genexis

#endif // WIN32
