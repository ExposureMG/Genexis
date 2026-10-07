#include "backend/BackendManager.hpp"
#include "backend/FlasherDeviceRegistry.hpp"
#include "backend/adapters/Ftdi2SpiAdapter.hpp"
#include "backend/adapters/GxBuild3Adapter.hpp"
#include "backend/adapters/NandProMaxAdapter.hpp"
#include "backend/adapters/UpdClientAdapter.hpp"
#include "backend/adapters/XsvfToolAdapter.hpp"

#include <algorithm>

namespace gxapi::backend {

namespace {

template <typename Service>
Service &findService(const std::vector<Service *> &services,
                     const std::string &name, Service &fallback) {
  const auto it = std::ranges::find_if(
      services, [&name](const Service *s) { return s->serviceName() == name; });
  return it != services.end() ? **it : fallback;
}

template <typename Service>
std::vector<std::string> serviceNames(const std::vector<Service *> &services) {
  std::vector<std::string> names;
  names.reserve(services.size());
  for (const Service *s : services) {
    names.push_back(s->serviceName());
  }
  return names;
}

} // namespace

BackendManager::BackendManager()
    : m_nandProMax(std::make_shared<NandProMaxAdapter>()),
      m_ftdi2Spi(std::make_shared<Ftdi2SpiAdapter>()),
      m_xsvfTool(std::make_shared<XsvfToolAdapter>()),
      m_updClient(std::make_shared<UpdClientAdapter>()),
      m_gxBuild3(std::make_shared<GxBuild3Adapter>()),
      m_flashServices{m_nandProMax.get(), m_ftdi2Spi.get(), m_updClient.get()},
      m_jtagServices{m_nandProMax.get(), m_xsvfTool.get()},
      m_builderServices{m_gxBuild3.get()},
      m_networkServices{m_updClient.get()} {}

BackendManager::~BackendManager() = default;

BackendManager &BackendManager::instance() {
  static BackendManager inst;
  return inst;
}

IFlashService &
BackendManager::flashForHardware(const std::string &hardwareName) {
  return findService<IFlashService>(
      m_flashServices, flashBackendFor(hardwareName), *m_nandProMax);
}

IJtagService &BackendManager::jtagForHardware(const std::string &hardwareName) {
  return findService<IJtagService>(m_jtagServices, jtagBackendFor(hardwareName),
                                   *m_nandProMax);
}

IBuilderService &BackendManager::builder() { return *m_gxBuild3; }

INetworkService &BackendManager::network() { return *m_updClient; }

std::vector<std::string> BackendManager::getAvailableFlashBackends() const {
  return serviceNames(m_flashServices);
}

std::vector<std::string> BackendManager::getAvailableJtagBackends() const {
  return serviceNames(m_jtagServices);
}

std::vector<std::string> BackendManager::getAvailableBuilderBackends() const {
  return serviceNames(m_builderServices);
}

std::vector<std::string> BackendManager::getAvailableNetworkBackends() const {
  return serviceNames(m_networkServices);
}

} // namespace gxapi::backend
