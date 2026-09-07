#pragma once

#include <cstdint>
#include <vector>

namespace gxapi::backend {

struct DiscoveredFlasherDevice {
  uint16_t vendorId{0};
  uint16_t productId{0};
};

class FlasherDiscovery {
public:
  static std::vector<DiscoveredFlasherDevice> getAllDevices();
};

} // namespace gxapi::backend
