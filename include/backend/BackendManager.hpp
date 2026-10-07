#pragma once

#include "backend/IBuilderService.hpp"
#include "backend/IFlashService.hpp"
#include "backend/IJtagService.hpp"
#include "backend/INetworkService.hpp"

#include <memory>
#include <string>
#include <vector>

namespace gxapi::backend {

class NandProMaxAdapter;
class Ftdi2SpiAdapter;
class XsvfToolAdapter;
class UpdClientAdapter;
class GxBuild3Adapter;

class BackendManager {
public:
  static BackendManager &instance();

  // hardwareName is a flasher profile's displayName, or kUpdClientBackend for
  // the network flasher (see FlasherDeviceRegistry.hpp).
  IFlashService &flashForHardware(const std::string &hardwareName);
  IJtagService &jtagForHardware(const std::string &hardwareName);

  IBuilderService &builder();
  INetworkService &network();

  // serviceName() of every constructed backend, per role.
  [[nodiscard]] std::vector<std::string> getAvailableFlashBackends() const;
  [[nodiscard]] std::vector<std::string> getAvailableJtagBackends() const;
  [[nodiscard]] std::vector<std::string> getAvailableBuilderBackends() const;
  [[nodiscard]] std::vector<std::string> getAvailableNetworkBackends() const;

private:
  BackendManager();
  ~BackendManager();
  BackendManager(const BackendManager &) = delete;
  BackendManager &operator=(const BackendManager &) = delete;

  std::shared_ptr<NandProMaxAdapter> m_nandProMax;
  std::shared_ptr<Ftdi2SpiAdapter> m_ftdi2Spi;
  std::shared_ptr<XsvfToolAdapter> m_xsvfTool;
  std::shared_ptr<UpdClientAdapter> m_updClient;
  std::shared_ptr<GxBuild3Adapter> m_gxBuild3;

  std::vector<IFlashService *> m_flashServices;
  std::vector<IJtagService *> m_jtagServices;
  std::vector<IBuilderService *> m_builderServices;
  std::vector<INetworkService *> m_networkServices;
};

} // namespace gxapi::backend
