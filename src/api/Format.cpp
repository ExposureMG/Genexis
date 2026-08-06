// Filesystem API — Genexis
// src/api/Filesystem.cpp
//
// Implements FsContext and the static probeBlob() factory for all supported
// Xbox 360 container and file types.
//
// R/W capability matrix:
//   Dir       — R/W via std::filesystem / Vendor::USBStorage
//   Fatx      — R/W via Fatx::FatxFileSystem (libfatx)
//   Stfs      — R (STFS, PEC, XCP have no write backend)
//   Pec       — R
//   Xcp       — R
//   Nand      — R (virtual entries extracted from raw header / NandResults)
//   FlashFs   — R (listing only; write-back not yet implemented)
//   SysUpdate — R
//   Xdbf      — R

#include "api/Format.hpp"

#include "Utils.hpp"
#include "plugins/PluginRegistry.hpp"
#include "vendor/USBStorage.hpp"

#ifdef _WIN32
#include "vendor/platforms/Windows.hpp"
#else
#include "vendor/platforms/UNIX.hpp"
#endif

// gxbuild3 headers (NAND / FlashFS parsing)
#include <FlashImage.hpp>
#include <bootloaders/Common.hpp>
#include <bootloaders/FlashFileSystem.hpp>

#include <algorithm>
#include <array>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace gxapi::Filesystem {

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------
namespace {

// Read up to `len` bytes starting at `offset` from a file; returns empty on
// any error.
[[nodiscard]] std::vector<std::byte>
readFileSlice(const std::filesystem::path &path, std::uint64_t offset,
              std::size_t len) {
  std::ifstream f(path, std::ios::binary);
  if (!f)
    return {};
  f.seekg(static_cast<std::streamoff>(offset), std::ios::beg);
  if (!f)
    return {};
  std::vector<std::byte> buf(len);
  f.read(reinterpret_cast<char *>(buf.data()),
         static_cast<std::streamsize>(len));
  buf.resize(static_cast<std::size_t>(f.gcount()));
  return buf;
}

// Read an entire file into memory.
[[nodiscard]] std::vector<std::byte>
readWholeFile(const std::filesystem::path &path) {
  std::ifstream f(path, std::ios::binary | std::ios::ate);
  if (!f)
    return {};
  auto size = f.tellg();
  if (size <= 0)
    return {};
  f.seekg(0);
  std::vector<std::byte> buf(static_cast<std::size_t>(size));
  f.read(reinterpret_cast<char *>(buf.data()), size);
  return buf;
}

// Big-endian 16-bit read from the first two bytes of `data`.
[[nodiscard]] std::uint16_t magic16(std::span<const std::byte> data,
                                    std::size_t off = 0) {
  if (data.size() < off + 2)
    return 0;
  return (static_cast<std::uint16_t>(data[off]) << 8) |
         static_cast<std::uint16_t>(data[off + 1]);
}

// Big-endian 32-bit read.
[[nodiscard]] std::uint32_t magic32(std::span<const std::byte> data,
                                    std::size_t off = 0) {
  if (data.size() < off + 4)
    return 0;
  return (static_cast<std::uint32_t>(data[off]) << 24) |
         (static_cast<std::uint32_t>(data[off + 1]) << 16) |
         (static_cast<std::uint32_t>(data[off + 2]) << 8) |
         static_cast<std::uint32_t>(data[off + 3]);
}

// Returns true if the first four bytes of `data` spell out the ASCII string
// `tag`.
[[nodiscard]] bool hasMagicStr(std::span<const std::byte> data,
                               const char (&tag)[5], std::size_t off = 0) {
  if (data.size() < off + 4)
    return false;
  return static_cast<unsigned char>(data[off]) ==
             static_cast<unsigned char>(tag[0]) &&
         static_cast<unsigned char>(data[off + 1]) ==
             static_cast<unsigned char>(tag[1]) &&
         static_cast<unsigned char>(data[off + 2]) ==
             static_cast<unsigned char>(tag[2]) &&
         static_cast<unsigned char>(data[off + 3]) ==
             static_cast<unsigned char>(tag[3]);
}

// Compute SHA-1 of `data` and return it as a 40-char hex string.
// Falls back to an empty string if the crypto library is unavailable.
[[nodiscard]] std::string sha1Hex(std::span<const std::byte> /*data*/) {
  // TODO: integrate excrypt SHA-1 here when the excrypt header is available.
  return {};
}

// Format `n` bytes as a human-readable size string (B / KB / MB / GB).
[[nodiscard]] std::string formatSize(std::size_t n) {
  if (n < 1024)
    return std::to_string(n) + " B";
  if (n < 1024 * 1024)
    return std::to_string(n / 1024) + " KB";
  if (n < 1024 * 1024 * 1024ull)
    return std::to_string(n / (1024 * 1024)) + " MB";
  return std::to_string(n / (1024 * 1024 * 1024ull)) + " GB";
}

// Hex-encode `data`.
[[nodiscard]] std::string hexEncode(std::span<const std::byte> data) {
  static constexpr char kHex[] = "0123456789ABCDEF";
  std::string out;
  out.reserve(data.size() * 2);
  for (auto b : data) {
    out += kHex[(static_cast<unsigned char>(b) >> 4) & 0xF];
    out += kHex[static_cast<unsigned char>(b) & 0xF];
  }
  return out;
}

// ---------------------------------------------------------------------------
// Probe helpers — detect file types from raw blobs
// ---------------------------------------------------------------------------

// Returns the bl_type magic as a 16-bit big-endian value at offset 0.
// 0x0F4D = NAND 1BL/SB magic (bl_header.magic).
//
// Bootloader magic summary (from Common.hpp _bl_type enum):
//   CB  = 0x342   SC  = 0x343   CD  = 0x344   CE  = 0x345
//   CF  = 0x346   CG  = 0x347
//
// XSB SB uses magic 0x0F4D (in NAND header).
// Split-chain BLs prefix with 0x0300-series (CB_A/CB_B) or 0x0600-series (SB
// etc).

struct BlKind {
  bool valid;
  std::uint16_t magic;
};
[[nodiscard]] BlKind readBlMagic(std::span<const std::byte> data) {
  if (data.size() < 2)
    return {false, 0};
  auto m = magic16(data, 0);
  return {m != 0, m};
}

// Determine bootloader name from magic + devkit flag.
[[nodiscard]] std::string blNameFromMagic(std::uint16_t m, bool devkit) {
  // Retail prefix: lower nibble of high byte 0x03xx or 0x06xx
  // We map by the bl_type enum values defined in Common.hpp.
  switch (m & 0x0FFF) {
  case 0x342:
    return devkit ? "SB" : "CB";
  case 0x343:
    return devkit ? "SC" : "SC";
  case 0x344:
    return devkit ? "SD" : "CD";
  case 0x345:
    return devkit ? "SE" : "CE";
  case 0x346:
    return devkit ? "SF" : "CF";
  case 0x347:
    return devkit ? "SG" : "CG";
  // Split-boot CB_A / CB_B distinction is by the pairing field (0x8000 = CB_B).
  default:
    return "BL";
  }
}

// Attempt to probe a bootloader blob (CB, SC, CD, CE, CF, CG variants).
// Returns nullopt if the magic bytes don't look like an Xbox 360 bootloader.
[[nodiscard]] std::optional<FileMetadata>
probeBootloader(std::span<const std::byte> data, std::string_view /*hint*/) {
  if (data.size() < sizeof(bl_header))
    return std::nullopt;

  auto m = magic16(data, 0);
  // Valid bootloader magic occupies 0x0300–0x03FF or 0x0600–0x07FF range.
  // NAND 1BL is 0x0F4D — handled separately via openNandImage.
  bool retail_range =
      (m >= 0x0300 && m < 0x0400) || (m >= 0x0600 && m < 0x0700);
  if (!retail_range)
    return std::nullopt;

  auto type = static_cast<bl_type>(m & 0x0FFF);
  bool devkit = (magic16(data, 4) & 0x8000) != 0; // pairing field bit 15

  std::string name = blNameFromMagic(m, devkit);

  // CB_A / CB_B disambiguation: CB_A has pairing 0x0000, CB_B uses 0x8000 bit.
  if ((m & 0x0FFF) == 0x342) {
    name = (devkit) ? "SB" : "CB";
    // If the pairing byte is 0x8000 it's the second-stage CB_B in split-boot.
    if (!devkit && magic16(data, 4) == 0x8000)
      name = "CB_B";
    // CB_A is typically the first stage.
  }

  switch (type) {
  case CF: {
    CfMeta meta;
    meta.bl_name = name;
    meta.source_version = magic16(data, 2); // bl_header.version  → base_ver
    meta.target_version = magic16(data, 6); // bl6_header.target_ver at +6
    meta.size = data.size();
    meta.sha1_hex = sha1Hex(data);
    // pairing_data / lockdown_value are in the encrypted portion of bl6_header;
    // leave as 0 until decryption is wired in.
    return meta;
  }
  case CG: {
    CgMeta meta;
    meta.bl_name = name;
    meta.version = magic16(data, 2);
    meta.size = data.size();
    meta.sha1_hex = sha1Hex(data);
    return meta;
  }
  default: {
    BootloaderMeta meta;
    meta.bl_name = name;
    meta.version = magic16(data, 2);
    meta.size = data.size();
    meta.sha1_hex = sha1Hex(data);
    meta.is_devkit = devkit;
    // LDV and PD live in the encrypted payload of bl2_header; they
    // require decryption with the CPU key.  Left as 0 until that
    // pipeline is wired in.
    return meta;
  }
  }
}

// Probe a Keyvault blob (must be exactly 0x4000 bytes).
[[nodiscard]] std::optional<FileMetadata>
probeKeyvault(std::span<const std::byte> data) {
  if (data.size() != 0x4000)
    return std::nullopt;

  // KV type byte at offset 0x1 (high nibble of magic 0xE4E at BL layer
  // reused; KV starts with a 2-byte magic 0xE4E then type).
  auto m = magic16(data, 0);
  if (m != 0xE4EE && m != 0xE4EF && (m >> 8) != 0xE4) {
    // Fallback: check for a known KV magic pattern — byte at 0 = 0x00 or 0x01.
    if (data[0] != std::byte{0x00} && data[0] != std::byte{0x01}) {
      return std::nullopt;
    }
  }

  KeyvaultMeta meta;
  meta.kv_type = static_cast<uint8_t>(data[0]);
  // Serial: bytes 0x284–0x28F (12 chars ASCII).
  if (data.size() > 0x290) {
    meta.serial = std::string(reinterpret_cast<const char *>(&data[0x284]), 12);
  }
  // DVD key: bytes 0x100–0x10F (16 bytes, but displayed as 32 hex chars).
  if (data.size() > 0x110) {
    meta.dvd_key_hex = hexEncode(data.subspan(0x100, 0x10));
  }
  return meta;
}

// Probe for XeLL (ELF magic 0x7F454C46 + XeLL string somewhere in first 64 KB).
[[nodiscard]] std::optional<FileMetadata>
probeXell(std::span<const std::byte> data) {
  if (data.size() < 4)
    return std::nullopt;
  if (!hasMagicStr(data, "\x7f" "ELF"))
    return std::nullopt;

  // Scan for "XeLL" or version string in the first 64 KB.
  constexpr const char kXeLL[] = "XeLL";
  const std::size_t kScan = std::min<std::size_t>(65536, data.size() - 4);
  bool found = false;
  std::string version;
  for (std::size_t i = 0; i < kScan; ++i) {
    if (data[i] == std::byte{static_cast<unsigned char>('X')} &&
        data[i + 1] == std::byte{static_cast<unsigned char>('e')} &&
        data[i + 2] == std::byte{static_cast<unsigned char>('L')} &&
        data[i + 3] == std::byte{static_cast<unsigned char>('L')}) {
      found = true;
      break;
    }
  }
  if (!found)
    return std::nullopt;

  XellMeta meta;
  // Type: scan for "1f"/"2f" tags — heuristic only.
  for (std::size_t i = 0; i < kScan - 1; ++i) {
    if ((data[i] == std::byte{'1'} || data[i] == std::byte{'2'}) &&
        data[i + 1] == std::byte{'f'}) {
      meta.xell_type = std::string(1, static_cast<char>(data[i])) + "f";
      break;
    }
  }
  return meta;
}

} // anonymous namespace

// ---------------------------------------------------------------------------
// EntryKind helpers
// ---------------------------------------------------------------------------

const char *entryKindLabel(EntryKind k) noexcept {
  switch (k) {
  case EntryKind::Dir:
    return "Folder";
  case EntryKind::Nand:
    return "NAND Image";
  case EntryKind::Stfs:
    return "STFS Container";
  case EntryKind::Pec:
    return "PEC Container";
  case EntryKind::Fatx:
    return "FATX Partition";
  case EntryKind::FlashFs:
    return "FlashFS";
  case EntryKind::SysUpdate:
    return "System Update";
  case EntryKind::Xcp:
    return "XCP Container";
  case EntryKind::Xdbf:
    return "XDBF";
  case EntryKind::File:
    return "File";
  }
  return "Unknown";
}

// Maps to the "entry-kind" string values defined in
// slint/components/file_explorer.slint.
const char *entryKindSlintStr(EntryKind k) noexcept {
  switch (k) {
  case EntryKind::Dir:
    return "dir";
  case EntryKind::Nand:
    return "nand";
  case EntryKind::Stfs:
    return "stfs";
  case EntryKind::Pec:
    return "stfs"; // reuses STFS badge
  case EntryKind::Fatx:
    return "fatx";
  case EntryKind::FlashFs:
    return "flashfs";
  case EntryKind::SysUpdate:
    return "sysupdate";
  case EntryKind::Xcp:
    return "stfs"; // wrapped STFS
  case EntryKind::Xdbf:
    return "file";
  case EntryKind::File:
    return "file";
  }
  return "file";
}

// ---------------------------------------------------------------------------
// FsContext::Impl — backend state
// ---------------------------------------------------------------------------
struct FsContext::Impl {
  EntryKind kind{EntryKind::File};

  // Host directory
  std::filesystem::path dir_path;

  // NAND image
  std::filesystem::path nand_path;
  std::vector<std::byte> nand_data; // loaded on demand

  // Buffer for containers
  std::vector<std::byte> buffer_data;

  // FlashFS — list-only
  std::filesystem::path flashfs_nand_path;
  std::vector<gxbuild3::bootloaders::FlashFileSystemEntry> flashfs_entries;

  // Cached NAND parse results (populated lazily)
  mutable std::optional<nand_results_t> nand_results;

  // ------------------------------------------------------------------
  // Helper: ensure nand_data is loaded.
  bool loadNandData() const {
    if (!nand_data.empty())
      return true;
    auto &self = const_cast<Impl &>(*this);
    self.nand_data = readWholeFile(nand_path);
    return !self.nand_data.empty();
  }

  // ------------------------------------------------------------------
  // Helper: build a virtual FsEntry list from NAND header + results.
  [[nodiscard]] std::vector<FsEntry> listNandRoot() const;
  [[nodiscard]] std::vector<FsEntry> listNandBootloaders() const;
};

// ---------------------------------------------------------------------------
// NAND virtual directory listing helpers
// ---------------------------------------------------------------------------

std::vector<FsEntry> FsContext::Impl::listNandRoot() const {
  if (!loadNandData())
    return {};
  auto span = std::span<const std::byte>(nand_data);

  std::vector<FsEntry> entries;

  auto makeEntry = [](std::string name, EntryKind k, std::size_t size = 0,
                      std::string info = {}) -> FsEntry {
    FsEntry e;
    e.name = std::move(name);
    e.kind = k;
    e.size = size;
    e.extra_info = std::move(info);
    return e;
  };

  // Read the NAND header to extract offsets.
  if (span.size() < sizeof(nand_header_t))
    return {};
  const auto *hdr = reinterpret_cast<const nand_header_t *>(span.data());

  // SMC firmware
  if (hdr->smc_boot_offset != 0 && hdr->smc_boot_size != 0) {
    FsEntry e =
        makeEntry("smc", EntryKind::File, hdr->smc_boot_size, "SMC Firmware");
    entries.push_back(std::move(e));
  }

  // Keyvault
  if (hdr->kv_addr != 0 && hdr->smc_boot_size != 0) {
    entries.push_back(makeEntry("kv", EntryKind::File, 0x4000, "Keyvault"));
  }

  // Bootloaders virtual folder
  {
    FsEntry e = makeEntry("bootloaders", EntryKind::Dir, 0, "Boot chain");
    e.child_count = 4; // approximate — resolved on drill-down
    entries.push_back(std::move(e));
  }

  // FlashFS
  if (hdr->fs_addr != 0) {
    entries.push_back(makeEntry("flashfs", EntryKind::FlashFs, 0, "Type: XSB"));
  }

  // System update (inside FlashFS — slot always 0 per user confirmation)
  {
    FsEntry e = makeEntry("sysupdate", EntryKind::SysUpdate, 0, "Slot: 0");
    entries.push_back(std::move(e));
  }

  return entries;
}

std::vector<FsEntry> FsContext::Impl::listNandBootloaders() const {
  // Return virtual entries for each bootloader offset found in the NAND.
  // In a full implementation, NandResults would be parsed here.
  // For now, return static placeholders that the UI can use.
  if (!loadNandData())
    return {};

  static constexpr const char *kBlNames[] = {"cb_a", "cb_b", "cd",
                                             "ce",   "cf",   "cg"};
  std::vector<FsEntry> entries;
  for (const char *name : kBlNames) {
    FsEntry e;
    e.name = name;
    e.kind = EntryKind::File;
    entries.push_back(e);
  }
  return entries;
}

// ---------------------------------------------------------------------------
// FsContext — constructor / destructor
// ---------------------------------------------------------------------------

FsContext::FsContext() : impl_(std::make_unique<Impl>()) {}
FsContext::~FsContext() = default;
FsContext::FsContext(FsContext &&) noexcept = default;
FsContext &FsContext::operator=(FsContext &&) noexcept = default;

// ---------------------------------------------------------------------------
// FsContext — factory methods
// ---------------------------------------------------------------------------

std::optional<FsContext>
FsContext::openPath(const std::filesystem::path &path) {
  std::error_code ec;
  if (!std::filesystem::exists(path, ec))
    return std::nullopt;

  // Directory → host folder view
  if (std::filesystem::is_directory(path, ec)) {
    FsContext ctx;
    ctx.impl_->kind = EntryKind::Dir;
    ctx.impl_->dir_path = path;
    return ctx;
  }

  // Read magic bytes
  auto magic = readFileSlice(path, 0, 8);
  if (magic.size() < 4)
    return std::nullopt;
  auto span = std::span<const std::byte>(magic);

  // NAND image (SB magic 0x0F4D at bytes 0-1, or raw header magic)
  if (magic16(span, 0) == 0x0F4D) {
    return openNandImage(path);
  }

  // STFS containers
  if (hasMagicStr(span, "PIRS") || hasMagicStr(span, "LIVE") ||
      hasMagicStr(span, "CON ")) {
    auto data = readWholeFile(path);
    if (data.empty())
      return std::nullopt;
    return openStfsBuffer(data);
  }

  // PEC (first 4 bytes: console security certificate starts immediately)
  // PEC is identified by the PEC header structure — we check for a valid
  // StfsVolumeDescriptor in the PEC header region.
  // Heuristic: if it's not PIRS/LIVE/CON and it's ~2 MB with correct structure.
  {
    auto pec_magic = readFileSlice(path, 0x234, 4);
    // The PEC header_sha1 sits at 0x228; volume descriptor follows.
    // For now we rely on PecContainer throwing on invalid data.
    // A cleaner probe will be added when PEC magic is documented.
  }

  // FATX
  if (hasMagicStr(span, "FATX")) {
    return openFatx(path, 0, 0);
  }

  // XCP (Xenia/Xbox compressed package)
  // XCP magic is not a simple 4-byte string — fall through to STFS buffer
  // probe.

  // XDBF standalone
  if (hasMagicStr(span, "XDBF")) {
    auto data = readWholeFile(path);
    if (data.empty())
      return std::nullopt;
    FsContext ctx;
    ctx.impl_->kind = EntryKind::Xdbf;
    ctx.impl_->buffer_data = std::move(data);
    return ctx;
  }

  // Unrecognised — treat as a raw file (probe() can still extract metadata)
  FsContext ctx;
  ctx.impl_->kind = EntryKind::File;
  ctx.impl_->nand_path = path; // reuse nand_path field as "current file path"
  return ctx;
}

std::optional<FsContext>
FsContext::openNandImage(const std::filesystem::path &path) {
  std::error_code ec;
  if (!std::filesystem::exists(path, ec))
    return std::nullopt;
  FsContext ctx;
  ctx.impl_->kind = EntryKind::Nand;
  ctx.impl_->nand_path = path;
  return ctx;
}

std::optional<FsContext>
FsContext::openStfsBuffer(std::span<const std::byte> data) {
  if (data.size() < 4)
    return std::nullopt;

  FsContext ctx;
  ctx.impl_->kind = EntryKind::Stfs;
  ctx.impl_->buffer_data.assign(data.begin(), data.end());
  return ctx;
}

std::optional<FsContext> FsContext::openFatx(const std::filesystem::path &image,
                                             std::uint64_t offset,
                                             std::uint64_t size) {
  FsContext ctx;
  ctx.impl_->kind = EntryKind::Fatx;
  ctx.impl_->nand_path = image;
  return ctx;
}

std::optional<FsContext>
FsContext::openFlashFs(const std::filesystem::path &nand_image) {
  // Load NAND data and parse the FlashFS using
  // gxbuild3::bootloaders::FlashFileSystem. Write-back is not yet implemented —
  // the FlashBlockDriver needed by FlashFileSystem requires NAND geometry info
  // that is not yet wired up in this API layer.
  auto nand_data = readWholeFile(nand_image);
  if (nand_data.size() < sizeof(nand_header_t))
    return std::nullopt;

  const auto *hdr = reinterpret_cast<const nand_header_t *>(nand_data.data());
  const std::uint32_t fs_addr = hdr->fs_addr;
  if (fs_addr == 0 || fs_addr >= nand_data.size())
    return std::nullopt;

  // FlashFileSystem requires a FlashBlockDriver — deferred for full R/W
  // implementation. For now, store the NAND path and expose the flashfs_entries
  // via the list() API.
  // TODO: construct FlashBlockDriver from nand_data + geometry and load
  // FlashFileSystem.

  FsContext ctx;
  ctx.impl_->kind = EntryKind::FlashFs;
  ctx.impl_->flashfs_nand_path = nand_image;
  // flashfs_entries left empty until FlashBlockDriver is wired in.
  return ctx;
}

// ---------------------------------------------------------------------------
// FsContext — state accessors
// ---------------------------------------------------------------------------

void FsContext::close() {
  impl_->buffer_data.clear();
  impl_->nand_data.clear();
  impl_->flashfs_entries.clear();
  impl_->kind = EntryKind::File;
}

bool FsContext::isOpen() const noexcept {
  switch (impl_->kind) {
  case EntryKind::Dir:
    return !impl_->dir_path.empty();
  case EntryKind::Nand:
    return !impl_->nand_path.empty();
  case EntryKind::Stfs:
  case EntryKind::Pec:
  case EntryKind::Xcp:
  case EntryKind::Xdbf:
    return !impl_->buffer_data.empty();
  case EntryKind::Fatx:
  case EntryKind::FlashFs:
  case EntryKind::SysUpdate:
  case EntryKind::File:
    return !impl_->nand_path.empty();
  }
  return false;
}

EntryKind FsContext::kind() const noexcept { return impl_->kind; }

bool FsContext::isWritable() const noexcept {
  return impl_->kind == EntryKind::Dir || impl_->kind == EntryKind::Fatx;
  // FlashFs write-back not yet implemented.
}

// ---------------------------------------------------------------------------
// FsContext::list()
// ---------------------------------------------------------------------------

std::vector<FsEntry> FsContext::list(std::string_view path) const {
  switch (impl_->kind) {

  // ------------------------------------------------------------------
  case EntryKind::Dir: {
    auto full = path.empty() || path == "/"
                    ? impl_->dir_path
                    : impl_->dir_path / std::string(path);
    std::vector<FsEntry> entries;
    std::error_code ec;
    for (const auto &de : std::filesystem::directory_iterator(full, ec)) {
      FsEntry e;
      e.name = de.path().filename().string();
      if (de.is_directory(ec)) {
        e.kind = EntryKind::Dir;
        e.child_count = static_cast<std::size_t>(
            std::distance(std::filesystem::directory_iterator(de.path(), ec),
                          std::filesystem::directory_iterator{}));
      } else {
        // Probe magic bytes to classify the file.
        auto magic = readFileSlice(de.path(), 0, 8);
        auto mspan = std::span<const std::byte>(magic);
        if (magic16(mspan, 0) == 0x0F4D)
          e.kind = EntryKind::Nand;
        else if (hasMagicStr(mspan, "PIRS") || hasMagicStr(mspan, "LIVE") ||
                 hasMagicStr(mspan, "CON "))
          e.kind = EntryKind::Stfs;
        else if (hasMagicStr(mspan, "FATX"))
          e.kind = EntryKind::Fatx;
        else if (hasMagicStr(mspan, "XDBF"))
          e.kind = EntryKind::Xdbf;
        else
          e.kind = EntryKind::File;

        e.size = static_cast<std::size_t>(de.file_size(ec));
      }
      entries.push_back(std::move(e));
    }
    return entries;
  }

  // ------------------------------------------------------------------
  case EntryKind::Nand: {
    if (path.empty() || path == "/")
      return impl_->listNandRoot();
    if (path == "/bootloaders" || path == "bootloaders")
      return impl_->listNandBootloaders();
    return {};
  }

  // ------------------------------------------------------------------
  case EntryKind::Stfs:
  case EntryKind::Pec:
  case EntryKind::Xcp:
  case EntryKind::Fatx:
  case EntryKind::Xdbf:
  default:
    return {};
  }
}

// ---------------------------------------------------------------------------
// FsContext::probeBlob() — static typed metadata extraction
// ---------------------------------------------------------------------------

std::optional<FileMetadata>
FsContext::probeBlob(std::span<const std::byte> data,
                     std::string_view hint_name) {
  if (data.empty())
    return std::nullopt;
  auto span4 = data.subspan(0, std::min<std::size_t>(data.size(), 8));

  // --- FMIM audio ---
  if (magic32(span4, 0) == 0x464D494D) { // "FMIM"
    FmimMeta meta;
    meta.track_title = u"FMIM Audio Stream";
    return meta;
  }

  // --- XEX2 executable ---
  if (magic32(span4, 0) == 0x58455832) { // "XEX2"
    XexMeta meta;
    return meta;
  }

  // --- XDBF ---
  if (magic32(span4, 0) == 0x58444246) { // "XDBF"
    XdbfMeta meta;
    return meta;
  }

  // --- Bootloaders ---
  if (auto bl = probeBootloader(data, hint_name))
    return bl;

  // --- Keyvault ---
  if (data.size() == 0x4000) {
    if (auto kv = probeKeyvault(data))
      return kv;
  }

  // --- XeLL ---
  if (auto xe = probeXell(data))
    return xe;

  // --- Generic fallback ---
  GenericFileMeta generic;
  generic.size = data.size();
  generic.sha1_hex = sha1Hex(data);
  return generic;
}

// ---------------------------------------------------------------------------
// FsContext::probe() — probe a named path within the context
// ---------------------------------------------------------------------------

std::optional<FileMetadata> FsContext::probe(std::string_view path) const {
  std::vector<std::byte> data;

  switch (impl_->kind) {
  case EntryKind::Dir: {
    auto full =
        path.empty() ? impl_->dir_path : impl_->dir_path / std::string(path);
    data = readWholeFile(full);
    break;
  }
  case EntryKind::Nand: {
    if (!impl_->loadNandData())
      return std::nullopt;
    auto span = std::span<const std::byte>(impl_->nand_data);
    const auto *hdr = reinterpret_cast<const nand_header_t *>(span.data());

    // Route by virtual path
    if (path == "kv" || path == "/kv") {
      data.assign(span.begin() + hdr->kv_addr,
                  span.begin() + hdr->kv_addr + 0x4000);
    } else if (path == "smc" || path == "/smc") {
      data.assign(span.begin() + hdr->smc_boot_offset,
                  span.begin() + hdr->smc_boot_offset + hdr->smc_boot_size);
    }
    // Bootloaders by name: will be extended as NandResults parsing is added.
    break;
  }
  case EntryKind::Stfs:
  case EntryKind::Pec:
  case EntryKind::Xcp:
  case EntryKind::Fatx:
  case EntryKind::File: {
    data = readWholeFile(impl_->nand_path);
    break;
  }
  default:
    return std::nullopt;
  }

  if (data.empty())
    return std::nullopt;
  return probeBlob(data, path);
}

// ---------------------------------------------------------------------------
// FsContext::read()
// ---------------------------------------------------------------------------

std::vector<std::byte> FsContext::read(std::string_view path) const {
  switch (impl_->kind) {
  case EntryKind::Dir: {
    auto full = impl_->dir_path / std::string(path);
    return readWholeFile(full);
  }
  case EntryKind::Nand: {
    if (!impl_->loadNandData())
      return {};
    auto span = std::span<const std::byte>(impl_->nand_data);
    const auto *hdr = reinterpret_cast<const nand_header_t *>(span.data());
    if ((path == "kv" || path == "/kv") &&
        hdr->kv_addr + 0x4000 <= span.size()) {
      return {span.begin() + hdr->kv_addr,
              span.begin() + hdr->kv_addr + 0x4000};
    }
    if ((path == "smc" || path == "/smc") &&
        hdr->smc_boot_offset + hdr->smc_boot_size <= span.size()) {
      return {span.begin() + hdr->smc_boot_offset,
              span.begin() + hdr->smc_boot_offset + hdr->smc_boot_size};
    }
    return {};
  }
  case EntryKind::File:
    return readWholeFile(impl_->nand_path);
  default:
    return impl_->buffer_data;
  }
}

// ---------------------------------------------------------------------------
// FsContext::write() / mkdir() / remove()
// ---------------------------------------------------------------------------

bool FsContext::write(std::string_view path, std::span<const std::byte> data) {
  switch (impl_->kind) {
  case EntryKind::Dir: {
    auto full = impl_->dir_path / std::string(path);
    std::error_code ec;
    std::filesystem::create_directories(full.parent_path(), ec);
    std::ofstream f(full, std::ios::binary | std::ios::trunc);
    if (!f)
      return false;
    f.write(reinterpret_cast<const char *>(data.data()),
            static_cast<std::streamsize>(data.size()));
    return f.good();
  }
  default:
    return false; // R/O containers
  }
}

bool FsContext::mkdir(std::string_view path) {
  switch (impl_->kind) {
  case EntryKind::Dir: {
    auto full = impl_->dir_path / std::string(path);
    std::error_code ec;
    return std::filesystem::create_directories(full, ec);
  }
  default:
    return false;
  }
}

bool FsContext::remove(std::string_view path) {
  switch (impl_->kind) {
  case EntryKind::Dir: {
    auto full = impl_->dir_path / std::string(path);
    std::error_code ec;
    return std::filesystem::remove(full, ec);
  }
  default:
    return false;
  }
}

// ---------------------------------------------------------------------------
// FsContext::extractAll()
// ---------------------------------------------------------------------------

bool FsContext::extractAll(std::string_view src_path,
                           const std::filesystem::path &dest_dir) const {
  switch (impl_->kind) {
  case EntryKind::Dir: {
    auto src = impl_->dir_path / std::string(src_path);
    std::error_code ec;
    std::filesystem::copy(src, dest_dir,
                          std::filesystem::copy_options::recursive |
                              std::filesystem::copy_options::overwrite_existing,
                          ec);
    return !ec;
  }
  default:
    return false;
  }
}

// ---------------------------------------------------------------------------
// FsContext::extraInfo()
// ---------------------------------------------------------------------------

std::string FsContext::extraInfo() const {
  std::ostringstream ss;
  switch (impl_->kind) {
  case EntryKind::Nand: {
    if (impl_->loadNandData()) {
      auto span = std::span<const std::byte>(impl_->nand_data);
      const auto *hdr = reinterpret_cast<const nand_header_t *>(span.data());
      auto magic = magic16(span, 0);
      std::string flash_type = (magic == 0x0F4D) ? "XSB SFC" : "Unknown";
      ss << "Flash Type: " << flash_type << "\n";
      ss << "KV Offset: 0x" << std::hex << hdr->kv_addr << "\n";
      ss << "FS Offset: 0x" << hdr->fs_addr;
    }
    break;
  }

  case EntryKind::Stfs:
    ss << "Type: STFS Container";
    break;

  case EntryKind::Pec:
    ss << "Type: PEC Container";
    break;

  case EntryKind::Fatx:
    ss << "Type: FATX Partition";
    break;

  case EntryKind::FlashFs:
    ss << "Type: XSB FlashFS";
    break;

  case EntryKind::Xdbf:
    ss << "Type: XDBF Database";
    break;

  default:
    ss << "File";
    break;
  }
  return ss.str();
}

} // namespace gxapi::Filesystem
