#pragma once

#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace gxapi::backend {

struct FlasherDeviceProfile {
  std::string displayName;
  std::string imageResource;
  std::string flashBackendName{};
  std::string jtagBackendName{};
  std::vector<std::pair<uint16_t, uint16_t>> vidPidList{};
  std::vector<uint16_t> vidOnlyList{};
};

inline const std::vector<FlasherDeviceProfile> KnownFlasherDevices = {
    
    {.displayName = "PicoFlasher",
     .imageResource = "qrc:/qt/qml/org/gxoss/genexis/assets/picoflasher.png",
     .flashBackendName = "NandProMax",
     .vidPidList = {{0x600D, 0x7001}},
     .vidOnlyList = {0x2E8A, 0x600D}},
    
    {.displayName = "xFlasher",
     .imageResource = "qrc:/qt/qml/org/gxoss/genexis/assets/xflasher.png",
     .flashBackendName = "FTDI2SPI",
     .jtagBackendName = "xsvftool",
     .vidPidList = {{0x0403, 0x6001},
                    {0x0403, 0x6010},
                    {0x0403, 0x6011},
                    {0x0403, 0x6014},
                    {0x0403, 0x6015}},
     .vidOnlyList = {0x0403}},
    
    {.displayName = "Nand-X / LPC",
     .imageResource = "qrc:/qt/qml/org/gxoss/genexis/assets/nandx.png",
     .flashBackendName = "NandProMax",
     .jtagBackendName = "NandProMax",
     .vidPidList = {{0xFFFF, 0x0004}},
     .vidOnlyList = {}},
    
    {.displayName = "TX DemoN",
     .imageResource = "qrc:/qt/qml/org/gxoss/genexis/assets/demon.png",
     .flashBackendName = "NandProMax",
     .jtagBackendName = "NandProMax",
     .vidPidList = {{0x11D4, 0x444E}},
     .vidOnlyList = {}},
    {.displayName = "Pico-DirtyJTAG",
     .imageResource = "qrc:/qt/qml/org/gxoss/genexis/assets/pico-djtag.png",
     .jtagBackendName = "xsvftool",
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

inline std::optional<FlasherDeviceProfile>
findDeviceByName(const std::string &name) {
  std::string lowerName = name;
  std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(),
                 ::tolower);

  for (const auto &dev : KnownFlasherDevices) {
    std::string lowerDev = dev.displayName;
    std::transform(lowerDev.begin(), lowerDev.end(), lowerDev.begin(),
                   ::tolower);
    if (lowerName.find(lowerDev) != std::string::npos ||
        lowerDev.find(lowerName) != std::string::npos) {
      return dev;
    }
  }
  return std::nullopt;
}

} 
