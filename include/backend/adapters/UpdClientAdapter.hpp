#pragma once

#include "backend/IFlashService.hpp"
#include "backend/INetworkService.hpp"
#include <memory>

namespace updclient::updserver {
class UpdServerClient;
}

namespace gxapi::backend {

class UpdClientAdapter : public INetworkService, public IFlashService {
public:
  UpdClientAdapter();
  ~UpdClientAdapter() override;

  [[nodiscard]] std::string serviceName() const override { return "UpdClient"; }

  // INetworkService
  std::vector<DiscoveredConsoleInfo> discoverConsoles() override;
  bool connect(const std::string &ipAddress, uint16_t port = 730) override;
  void disconnect() override;
  [[nodiscard]] bool isConnected() const override;
  [[nodiscard]] std::string connectedIp() const override;

  std::expected<void, std::string>
  readNand(const std::filesystem::path &outputPath, size_t dumpSize = 0,
           FlashProgressCallback progressCb = nullptr) override;

  std::expected<void, std::string>
  writeNand(const std::filesystem::path &inputPath,
            FlashProgressCallback progressCb = nullptr) override;

  std::expected<void, std::string> rebootConsole() override;
  std::expected<void, std::string> shutdownConsole() override;

  // IFlashService
  std::expected<FlashInfo, std::string>
  getFlashInfo(const FlashDeviceConfig &config = {}) override;

  std::expected<void, std::string>
  readNand(const std::filesystem::path &outputPath, uint32_t startBlock = 0,
           uint32_t blockCount = 0, const FlashDeviceConfig &config = {},
           FlashProgressCallback progressCb = nullptr) override;

  std::expected<void, std::string>
  writeNand(const std::filesystem::path &inputPath, uint32_t startBlock = 0,
            bool eraseFirst = true, bool verifyAfter = false,
            const FlashDeviceConfig &config = {},
            FlashProgressCallback progressCb = nullptr) override;

  std::expected<void, std::string>
  eraseNand(uint32_t startBlock, uint32_t blockCount,
            const FlashDeviceConfig &config = {}) override;

private:
  std::unique_ptr<updclient::updserver::UpdServerClient> m_client;
};

} // namespace gxapi::backend
