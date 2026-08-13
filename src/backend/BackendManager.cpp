#include "backend/BackendManager.hpp"
#include "backend/FlasherDeviceRegistry.hpp"
#include "backend/adapters/Ftdi2SpiAdapter.hpp"
#include "backend/adapters/GxBuild3Adapter.hpp"
#include "backend/adapters/NandProMaxAdapter.hpp"
#include "backend/adapters/UpdClientAdapter.hpp"
#include "backend/adapters/XsvfToolAdapter.hpp"

#include <algorithm>

namespace gxapi::backend {

BackendManager::BackendManager()
    : m_nandProMax(std::make_shared<NandProMaxAdapter>()),
      m_ftdi2Spi(std::make_shared<Ftdi2SpiAdapter>()),
      m_xsvfTool(std::make_shared<XsvfToolAdapter>()),
      m_updClient(std::make_shared<UpdClientAdapter>()),
      m_gxBuild3(std::make_shared<GxBuild3Adapter>()) {}

BackendManager::~BackendManager() = default;

BackendManager &BackendManager::instance() {
  static BackendManager inst;
  return inst;
}

void BackendManager::initialize() {}

IFlashService &
BackendManager::flashForHardware(const std::string &hardwareName) {
  std::string lower = hardwareName;
  std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

  if (lower.find("updclient") != std::string::npos ||
      lower.find("updserver") != std::string::npos ||
      lower.find("network") != std::string::npos) {
    return *m_updClient;
  }

  auto dev = findDeviceByName(hardwareName);
  if (dev.has_value()) {
    if (dev->flashBackendName == "FTDI2SPI") {
      return *m_ftdi2Spi;
    }
  }

  return *m_nandProMax;
}

IJtagService &BackendManager::jtagForHardware(const std::string &hardwareName) {
  auto dev = findDeviceByName(hardwareName);
  if (dev.has_value()) {
    if (dev->jtagBackendName == "xsvftool") {
      return *m_xsvfTool;
    }
  }

  return *m_nandProMax;
}

IFlashService &BackendManager::flash(const std::string &name) {
  if (!name.empty()) {
    return flashForHardware(name);
  }
  return *m_nandProMax;
}

IJtagService &BackendManager::jtag(const std::string &name) {
  if (!name.empty()) {
    return jtagForHardware(name);
  }
  return *m_xsvfTool;
}

IBuilderService &BackendManager::builder(const std::string &name) {
  (void)name;
  return *m_gxBuild3;
}

INetworkService &BackendManager::network() { return *m_updClient; }

std::vector<std::string> BackendManager::getAvailableFlashBackends() const {
  return {"NandProMax", "FTDI2SPI", "UpdClient"};
}

std::vector<std::string> BackendManager::getAvailableJtagBackends() const {
  return {"NandProMax", "xsvftool"};
}

std::vector<std::string> BackendManager::getAvailableBuilderBackends() const {
  return {"gxbuild3"};
}

std::vector<std::string> BackendManager::getAvailableNetworkBackends() const {
  return {"UpdClient"};
}

} // namespace gxapi::backend
