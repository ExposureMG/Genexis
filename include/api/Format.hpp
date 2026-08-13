#pragma once

// Filesystem API — Genexis
// Unified interface over host directories, NAND images, STFS/PEC/XCP
// containers, FATX partitions, and Xbox 360 boot-file metadata extraction.
//
// Containers (openable into a new view):
//   Dir, Nand, Stfs, Pec, Fatx, FlashFs*, SysUpdate, Xcp, Xdbf
//   (* FlashFs listing is supported; write-back not yet implemented)
//
// Files (probe() returns typed metadata):
//   Bootloaders (CB/CB_A/CB_X/CB_B/SC/CD/SD/CE/SE/CF/SF/CG/SG),
//   Keyvault, SMC firmware, SMC config, XeLL, XEX2, FMIM, XDBF, GPD, SPA,
//   Generic (SHA-1 + size)
//
// Adding new format support:
//   1. Add a new metadata struct below.
//   2. Add it as a new arm to the FileMetadata variant.
//   3. Add detection logic to probeBlob().
//   4. If it is a navigable container, add a new EntryKind value and handle
//      it in FsContext::Impl::list() and the open*() factories.

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace gxapi::Filesystem {

// ---------------------------------------------------------------------------
// EntryKind — identifies the type of a directory entry for UI badge rendering.
// The Slint string mapping lives in entryKindSlintStr().
// ---------------------------------------------------------------------------
enum class EntryKind {
  Dir,       // Regular host-OS directory
  Nand,      // Xbox 360 NAND image (SB magic 0x0F4D at offset 0)
  Stfs,      // STFS container: PIRS / LIVE / CON
  Pec,       // Per-console STFS variant (PEC header)
  Fatx,      // FATX partition (embedded in NAND or standalone slice)
  FlashFs,   // Xbox 360 flash filesystem (inside NAND at fs_addr)
  SysUpdate, // System update PIRS inside FlashFS; slot always 0
  Xcp,       // Compressed STFS package (decompress → inner StfsContainer)
  Xdbf,      // Standalone XDBF (achievements / title info)
  File,      // Leaf entry — use probe() for typed metadata
             // Future: add new container kinds here (e.g. God, Svod, …)
};

// Human-readable display label for an EntryKind.
[[nodiscard]] const char *entryKindLabel(EntryKind k) noexcept;

// Slint-compatible "entry-kind" string matching file_explorer.slint values.
[[nodiscard]] const char *entryKindSlintStr(EntryKind k) noexcept;

// ---------------------------------------------------------------------------
// FsEntry — a single row returned by FsContext::list().
// ---------------------------------------------------------------------------
struct FsEntry {
  std::string name; // Display name (filename or virtual name)
  EntryKind kind{EntryKind::File};
  std::size_t size{0};        // Byte size (0 for containers / dirs)
  std::string extra_info;     // Short human-readable descriptor for UI
  std::size_t child_count{0}; // Non-zero for navigable containers
};

// ---------------------------------------------------------------------------
// FileMetadata — typed information extracted by probe() / probeBlob().
// Each struct maps directly to an "expanded view" field set from UIPLAN.md.
//
// To add a new file type:
//   1. Define your struct here.
//   2. Add it to FileMetadata below.
//   3. Add detection in probeBlob().
// ---------------------------------------------------------------------------

// Fallback: any unrecognised file
struct GenericFileMeta {
  std::size_t size{0};
  std::string sha1_hex;      // 40-char hex, empty if not computed
  std::string crypt_key_hex; // empty if not applicable
};

// CB / CB_A / CB_X / CB_B / SB / SC / CD / SD / CE / SE
// Identified by the bl_type enum in Common.hpp (CB=0x342, SC=0x343, …)
struct BootloaderMeta {
  std::string
      bl_name; // "CB", "CB_A", "CB_X", "CB_B", "SC", "CD", "SD", "CE", "SE"
  std::uint16_t version{0}; // bl_header.version
  std::uint8_t lockdown_value{
      0}; // LDV — from more_globals[1] in bl2_header (when decrypted)
  std::uint32_t pairing_data{0}; // PD — lower 3 bytes of pairing_data field
  std::size_t size{0};
  std::string sha1_hex;
  std::string crypt_key_hex;
  bool is_devkit{false};
};

// CF / SF — patch-slot bootloaders (bl6_header / bl_type CF=0x346)
struct CfMeta {
  std::string bl_name;             // "CF" or "SF"
  std::uint16_t source_version{0}; // bl6_header.base_ver
  std::uint16_t target_version{0}; // bl6_header.target_ver
  std::uint32_t pairing_data{0};   // PD — from CfMetadataDecrypted.pairing_data
  std::uint8_t lockdown_value{0};  // LDV — CfMetadataDecrypted.lockdown_value
  std::uint8_t update_slot{0};     // CfMetadataDecrypted.update_slot
  std::size_t size{0};
  std::string sha1_hex;
  std::string crypt_key_hex;
};

// CG / SG — companion patch-slot data (bl_type CG=0x347)
struct CgMeta {
  std::string bl_name;      // "CG" or "SG"
  std::uint16_t version{0}; // bl_header.version
  std::size_t size{0};
  std::string sha1_hex;
  std::string crypt_key_hex;
};

// Keyvault (0x4000 bytes; KV magic / KV type byte at 0x0)
struct KeyvaultMeta {
  std::uint8_t kv_type{0};    // 1-digit number
  std::string mfr_date;       // "MM/DD/YYYY"
  std::string serial;         // 12-character console serial
  std::string console_id_hex; // 12 hex chars
  std::string region_hex;     // 20 hex chars
  std::string
      dvd_key_hex;      // 32 hex chars (EG. 13319182EBA65CCB1D2F63C184D4FE2E)
  bool has_osig{false}; // true if OSIG occupies the DVD key slot
};

// SMC firmware blob (XSB/PSB/KSB hardware SMC)
struct SmcFwMeta {
  std::string smc_type;         // "XSB", "PSB", "KSB"
  std::string firmware_type;    // "Retail", "JTAG", "Glitch", "Glitch3", "CR4"
  std::string firmware_version; // e.g. "3.100"
};

// SMC config (magic 0x534D4343 "SMCC" — see XConfig.cpp)
struct SmcConfigMeta {
  std::uint8_t fan_min{0};
  std::uint8_t fan_max{0};
  std::uint32_t game_region{0};
  // Full config blob retained for future field additions
  std::vector<std::byte> raw_config;
};

// XeLL payload (ELF with XeLL version string)
struct XellMeta {
  std::string xell_type;    // "1f", "2f", etc.
  std::string xell_version; // e.g. "0.9993"
};

// FATX leaf entry (file inside a FATX partition)
struct FatxFileMeta {
  std::size_t size{0};
  std::uint8_t attributes{0};
  bool is_read_only{false};
  bool is_hidden{false};
  bool is_system{false};
};

// FMIM audio track container
struct FmimMeta {
  std::u16string track_title;
  std::u16string album_name;
  std::u16string artist;
  std::u16string genre;
  std::uint32_t track_length_ms{0};
  std::uint32_t track_number{0};
};

// XEX2 executable
struct XexMeta {
  std::uint32_t module_flags{0};
  std::uint32_t title_id{0}; // optional header 0x00040004
  std::uint16_t version_major{0};
  std::uint16_t version_minor{0};
};

// GPD (Gamer Profile Data — XDBF with namespace 1 achievements)
struct GpdMeta {
  std::int32_t achievement_count{0};
  std::int32_t gamerscore_total{0};
  std::u16string title_name;
};

// SPA (Standalone Publisher Achievements — XDBF with XACH / XTHD sections)
struct SpaMeta {
  std::uint32_t title_id{0};
  int achievement_count{0};
  std::string version_str; // "major.minor.build.revision"
};

// XDBF (raw standalone file — achievements, title info, etc.)
struct XdbfMeta {
  std::uint32_t version{0};
  int entry_count{0};
};

// Future: add new metadata structs above this comment, then add them to the
// variant.

using FileMetadata =
    std::variant<GenericFileMeta, BootloaderMeta, CfMeta, CgMeta, KeyvaultMeta,
                 SmcFwMeta, SmcConfigMeta, XellMeta, FatxFileMeta, FmimMeta,
                 XexMeta, GpdMeta, SpaMeta, XdbfMeta
                 // Future: add new types to the variant list above (e.g.
                 // GodMeta, SvodMeta …)
                 >;

// ---------------------------------------------------------------------------
// FsContext — the main API object.
//
// One FsContext represents a single open "location" — a directory, a NAND
// image, a STFS container, a FATX partition slice, or the FlashFS inside a
// NAND image.
//
// Usage pattern:
//   auto ctx = gxapi::Filesystem::FsContext::openPath("/path/to/nand.bin");
//   if (!ctx) { /* probe failed */ }
//   for (auto& entry : ctx->list("/")) { /* render rows */ }
//   auto meta = ctx->probe("/bootloaders/cb_a");
// ---------------------------------------------------------------------------
class FsContext {
public:
  FsContext();
  ~FsContext();

  FsContext(const FsContext &) = delete;
  FsContext &operator=(const FsContext &) = delete;

  FsContext(FsContext &&) noexcept;
  FsContext &operator=(FsContext &&) noexcept;

  // -----------------------------------------------------------------------
  // Factory — open by magic-byte probe
  // -----------------------------------------------------------------------

  // Opens a host path (directory or image file).
  // Reads the first 8 bytes and selects the appropriate backend.
  // Returns nullopt if the path does not exist or is unrecognised.
  [[nodiscard]] static std::optional<FsContext>
  openPath(const std::filesystem::path &path);

  // Explicit NAND image open — skips magic check.
  [[nodiscard]] static std::optional<FsContext>
  openNandImage(const std::filesystem::path &path);

  // Open a STFS / PEC / XCP container from an in-memory buffer.
  // Kind is auto-detected from the first 4 bytes.
  [[nodiscard]] static std::optional<FsContext>
  openStfsBuffer(std::span<const std::byte> data);

  // Open a FATX partition at the given byte offset and size within an image.
  // offset=0 and size=0 will attempt full-image FATX open.
  [[nodiscard]] static std::optional<FsContext>
  openFatx(const std::filesystem::path &image, std::uint64_t offset = 0,
           std::uint64_t size = 0);

  // Open the FlashFS inside a NAND image.
  // Listing is supported.  Write operations return false (not yet implemented).
  [[nodiscard]] static std::optional<FsContext>
  openFlashFs(const std::filesystem::path &nand_image);

  // -----------------------------------------------------------------------
  // State
  // -----------------------------------------------------------------------

  void close();
  [[nodiscard]] bool isOpen() const noexcept;
  [[nodiscard]] EntryKind kind() const noexcept;
  [[nodiscard]] bool isWritable() const noexcept;

  // -----------------------------------------------------------------------
  // Navigation
  // -----------------------------------------------------------------------

  // List entries at path.  Use "/" or "" for the root of the container.
  // For Nand contexts, path selects a virtual sub-tree:
  //   "/"            — top-level (smc, kv, bootloaders/, flashfs, sysupdate)
  //   "/bootloaders" — individual BL entries
  [[nodiscard]] std::vector<FsEntry> list(std::string_view path = "/") const;

  // -----------------------------------------------------------------------
  // File operations
  // -----------------------------------------------------------------------

  // Probe a named path within the current context and return typed metadata.
  // For File-kind contexts, path is ignored and the root blob is probed.
  [[nodiscard]] std::optional<FileMetadata> probe(std::string_view path) const;

  // Static utility: probe a raw blob without an open context.
  // hint_name is the filename (used only to break ambiguity between
  // GPD/SPA/XDBF).
  [[nodiscard]] static std::optional<FileMetadata>
  probeBlob(std::span<const std::byte> data, std::string_view hint_name = "");

  // Read the raw bytes of a file at path.
  [[nodiscard]] std::vector<std::byte> read(std::string_view path) const;

  // Write data to path.  Returns false for read-only containers
  // (STFS/PEC/XCP/FlashFs/Nand).
  bool write(std::string_view path, std::span<const std::byte> data);

  // Create a directory at path.  Returns false for read-only containers.
  bool mkdir(std::string_view path);

  // Remove a file or empty directory.  Returns false for read-only containers.
  bool remove(std::string_view path);

  // Recursively extract src_path to dest_dir on the host filesystem.
  bool extractAll(std::string_view src_path,
                  const std::filesystem::path &dest_dir) const;

  // -----------------------------------------------------------------------
  // Container-level metadata for the UI info panel / badge
  // -----------------------------------------------------------------------

  // Returns a newline-separated string of key: value pairs describing the
  // container, e.g. "Image Type: Retail\nMotherboard: Corona\nVersion: 17559"
  [[nodiscard]] std::string extraInfo() const;

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

} // namespace gxapi::Filesystem
