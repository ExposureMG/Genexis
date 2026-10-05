#include "backend/adapters/XsvfToolAdapter.hpp"
#include <xsvftool.h>

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <mutex>
#include <string>
#include <system_error>
#include <vector>

#ifdef _WIN32
#include <io.h>
#define GX_DUP _dup
#define GX_DUP2 _dup2
#define GX_CLOSE _close
#define GX_FILENO _fileno
#else
#include <unistd.h>
#define GX_DUP dup
#define GX_DUP2 dup2
#define GX_CLOSE close
#define GX_FILENO fileno
#endif

namespace gxapi::backend {

namespace {
std::mutex g_xsvfMutex;

// xsvftool reports scanned devices by printing "idcode=0x..." lines to stdout
// and has no callback API, so stdout is redirected to a temp file for the
// duration of a scan. Callers hold g_xsvfMutex.
class StdoutCapture {
public:
  StdoutCapture() {
    const auto stamp =
        std::chrono::steady_clock::now().time_since_epoch().count();
    m_path = std::filesystem::temp_directory_path() /
             ("genexis_xsvf_" + std::to_string(stamp) + ".txt");
    std::fflush(stdout);
    m_file = std::fopen(m_path.string().c_str(), "w+");
    if (!m_file)
      return;
    m_saved = GX_DUP(GX_FILENO(stdout));
    if (m_saved < 0 || GX_DUP2(GX_FILENO(m_file), GX_FILENO(stdout)) < 0) {
      if (m_saved >= 0)
        GX_CLOSE(m_saved);
      m_saved = -1;
      return;
    }
    m_active = true;
  }

  ~StdoutCapture() {
    restore();
    if (m_file)
      std::fclose(m_file);
    std::error_code ec;
    std::filesystem::remove(m_path, ec);
  }

  StdoutCapture(const StdoutCapture &) = delete;
  StdoutCapture &operator=(const StdoutCapture &) = delete;

  [[nodiscard]] bool active() const { return m_active; }

  // Restores stdout and returns everything written while capturing.
  std::string finish() {
    restore();
    std::string out;
    if (!m_file)
      return out;
    std::rewind(m_file);
    char buf[4096];
    size_t n;
    while ((n = std::fread(buf, 1, sizeof buf, m_file)) > 0)
      out.append(buf, n);
    return out;
  }

private:
  void restore() {
    if (!m_active)
      return;
    std::fflush(stdout);
    GX_DUP2(m_saved, GX_FILENO(stdout));
    GX_CLOSE(m_saved);
    m_saved = -1;
    m_active = false;
  }

  std::filesystem::path m_path;
  std::FILE *m_file{nullptr};
  int m_saved{-1};
  bool m_active{false};
};

std::vector<uint32_t> parseIdcodes(const std::string &output) {
  std::vector<uint32_t> codes;
  constexpr char kTag[] = "idcode=0x";
  size_t pos = 0;
  while ((pos = output.find(kTag, pos)) != std::string::npos) {
    pos += sizeof(kTag) - 1;
    char *end = nullptr;
    const unsigned long value = std::strtoul(output.c_str() + pos, &end, 16);
    if (end != output.c_str() + pos)
      codes.push_back(static_cast<uint32_t>(value));
  }
  return codes;
}
} // namespace

std::expected<std::vector<uint32_t>, std::string>
XsvfToolAdapter::scanChain(const JtagDeviceConfig &config) {
  std::lock_guard<std::mutex> lock(g_xsvfMutex);

  const char *backendName =
      config.backend.empty() ? nullptr : config.backend.c_str();

  StdoutCapture capture;
  const bool captured = capture.active();
  const int rc = xsvftool_scan_chain(backendName,
                                     static_cast<int>(config.clockFrequencyHz));
  const std::string output = capture.finish();

  if (rc != 0) {
    return std::unexpected("xsvftool JTAG scan chain failed with exit code: " +
                           std::to_string(rc));
  }
  if (!captured) {
    return std::unexpected(
        "JTAG scan completed but its output could not be captured");
  }

  return parseIdcodes(output);
}

std::expected<void, std::string>
XsvfToolAdapter::flashCpld(const std::filesystem::path &bitstreamPath,
                           const JtagDeviceConfig &config,
                           JtagProgressCallback progressCb) {
  if (!std::filesystem::exists(bitstreamPath)) {
    return std::unexpected("Bitstream file does not exist: " +
                           bitstreamPath.string());
  }

  std::string ext = bitstreamPath.extension().string();
  std::transform(ext.begin(), ext.end(), ext.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  if (ext != ".svf" && ext != ".xsvf") {
    return std::unexpected("Unsupported bitstream type '" + ext +
                           "' (expected .svf or .xsvf)");
  }

  std::lock_guard<std::mutex> lock(g_xsvfMutex);

  // xsvftool plays the whole file in one blocking call with no progress
  // callback, so only start and end are reported.
  if (progressCb) {
    JtagProgressInfo pInfo{.bytesDone = 0,
                           .totalBytes = 0,
                           .percentage = 0.0f,
                           .statusMessage = "Starting xsvftool CPLD flash..."};
    progressCb(pInfo);
  }

  const std::string pathStr = bitstreamPath.string();
  const char *backendName =
      config.backend.empty() ? nullptr : config.backend.c_str();
  const int freq = static_cast<int>(config.clockFrequencyHz);

  const int rc = (ext == ".svf")
                     ? xsvftool_play_svf(pathStr.c_str(), backendName, freq)
                     : xsvftool_play_xsvf(pathStr.c_str(), backendName, freq);

  if (rc != 0) {
    return std::unexpected("xsvftool CPLD flash failed with exit code: " +
                           std::to_string(rc));
  }

  if (progressCb) {
    JtagProgressInfo pInfo{.bytesDone = 100,
                           .totalBytes = 100,
                           .percentage = 100.0f,
                           .statusMessage = "CPLD Timing Flash complete."};
    progressCb(pInfo);
  }

  return {};
}

} // namespace gxapi::backend
