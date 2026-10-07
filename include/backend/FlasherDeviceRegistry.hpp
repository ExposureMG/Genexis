#pragma once

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace gxapi::backend {

// Backend names, as each adapter reports in serviceName().
inline constexpr char kNandProMaxBackend[] = "NandProMax";
inline constexpr char kFtdi2SpiBackend[] = "FTDI2SPI";
inline constexpr char kXsvfToolBackend[] = "xsvftool";
inline constexpr char kUpdClientBackend[] = "UpdClient";

struct FlasherDeviceProfile {
  std::string displayName;
  std::string imageResource;
  std::string flashBackendName{};
  std::string jtagBackendName{};
  // xsvftool probe: "FTDI" or "DirtyJTAG". Only used by the xsvftool backend.
  std::string jtagProbe{};
  std::vector<std::pair<uint16_t, uint16_t>> vidPidList{};
  std::vector<uint16_t> vidOnlyList{};
};

inline const std::vector<FlasherDeviceProfile> KnownFlasherDevices = {
    {.displayName = "PicoFlasher",
     .imageResource = "qrc:/qt/qml/org/gxoss/genexis/assets/picoflasher.png",
     .flashBackendName = kNandProMaxBackend,
     .vidPidList = {{0x600D, 0x7001}},
     .vidOnlyList = {0x2E8A, 0x600D}},
    {.displayName = "xFlasher",
     .imageResource = "qrc:/qt/qml/org/gxoss/genexis/assets/xflasher.png",
     .flashBackendName = kFtdi2SpiBackend,
     .jtagBackendName = kXsvfToolBackend,
     .jtagProbe = "FTDI",
     .vidPidList = {{0x0403, 0x6001},
                    {0x0403, 0x6010},
                    {0x0403, 0x6011},
                    {0x0403, 0x6014},
                    {0x0403, 0x6015}},
     .vidOnlyList = {0x0403}},
    {.displayName = "Nand-X / LPC",
     .imageResource = "qrc:/qt/qml/org/gxoss/genexis/assets/nandx.png",
     .flashBackendName = kNandProMaxBackend,
     .jtagBackendName = kNandProMaxBackend,
     .vidPidList = {{0xFFFF, 0x0004}},
     .vidOnlyList = {}},
    {.displayName = "TX DemoN",
     .imageResource = "qrc:/qt/qml/org/gxoss/genexis/assets/demon.png",
     .flashBackendName = kNandProMaxBackend,
     .jtagBackendName = kNandProMaxBackend,
     .vidPidList = {{0x11D4, 0x444E}},
     .vidOnlyList = {}},
    {.displayName = "Pico-DirtyJTAG",
     .imageResource = "qrc:/qt/qml/org/gxoss/genexis/assets/pico-djtag.png",
     .jtagBackendName = kXsvfToolBackend,
     .jtagProbe = "DirtyJTAG",
     .vidPidList = {{0x1209, 0xC0CA}},
     .vidOnlyList = {}}};

inline std::optional<FlasherDeviceProfile> findDeviceByVidPid(uint16_t vid,
                                                              uint16_t pid) {
  for (const auto &dev : KnownFlasherDevices) {
    for (const auto &[v, p] : dev.vidPidList) {
      if (v == vid && p == pid) {
        return dev;
      }
    }
    for (uint16_t v : dev.vidOnlyList) {
      if (v == vid) {
        return dev;
      }
    }
  }
  return std::nullopt;
}

// Case-insensitive exact match on displayName; an empty name never matches.
inline std::optional<FlasherDeviceProfile>
findDeviceByName(const std::string &name) {
  if (name.empty()) {
    return std::nullopt;
  }
  const auto sameChar = [](unsigned char a, unsigned char b) {
    return std::tolower(a) == std::tolower(b);
  };
  for (const auto &dev : KnownFlasherDevices) {
    if (std::ranges::equal(dev.displayName, name, sameChar)) {
      return dev;
    }
  }
  return std::nullopt;
}

// hardwareName is a profile's displayName, or kUpdClientBackend for the
// network flasher. Hardware without a profile entry for the role, including
// "None", falls back to NandProMax.
inline std::string flashBackendFor(const std::string &hardwareName) {
  if (hardwareName == kUpdClientBackend) {
    return kUpdClientBackend;
  }
  const auto dev = findDeviceByName(hardwareName);
  if (dev && !dev->flashBackendName.empty()) {
    return dev->flashBackendName;
  }
  return kNandProMaxBackend;
}

inline std::string jtagBackendFor(const std::string &hardwareName) {
  const auto dev = findDeviceByName(hardwareName);
  if (dev && !dev->jtagBackendName.empty()) {
    return dev->jtagBackendName;
  }
  return kNandProMaxBackend;
}

// Names the CPLD parts a JTAG chain scan can recognise by IDCODE.
inline std::optional<std::string_view> findJtagPartName(uint32_t idcode) {
  switch (idcode) {
  case 0x06e5e093:
  case 0x06e5c093:
    return "XC2C64A";
  default:
    return std::nullopt;
  }
}

} // namespace gxapi::backend
