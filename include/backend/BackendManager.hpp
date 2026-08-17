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

  void initialize();

  
  
  
  
  
  IFlashService &flashForHardware(const std::string &hardwareName);
  IJtagService &jtagForHardware(const std::string &hardwareName);

  
  IFlashService &flash(const std::string &name = "");
  IJtagService &jtag(const std::string &name = "");
  IBuilderService &builder(const std::string &name = "");
  INetworkService &network();

  
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
};

} 
