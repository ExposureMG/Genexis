

#include "api/Format.hpp"

#include "Utils.hpp"
#include "vendor/USBStorage.hpp"

#ifdef _WIN32
#include "vendor/platforms/Windows.hpp"
#else
#include "vendor/platforms/UNIX.hpp"
#endif

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

namespace {

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

[[nodiscard]] std::uint16_t magic16(std::span<const std::byte> data,
                                    std::size_t off = 0) {
  if (data.size() < off + 2)
    return 0;
  return (static_cast<std::uint16_t>(data[off]) << 8) |
         static_cast<std::uint16_t>(data[off + 1]);
}

[[nodiscard]] std::uint32_t magic32(std::span<const std::byte> data,
                                    std::size_t off = 0) {
  if (data.size() < off + 4)
    return 0;
  return (static_cast<std::uint32_t>(data[off]) << 24) |
         (static_cast<std::uint32_t>(data[off + 1]) << 16) |
         (static_cast<std::uint32_t>(data[off + 2]) << 8) |
         static_cast<std::uint32_t>(data[off + 3]);
}

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

[[nodiscard]] std::string sha1Hex(std::span<const std::byte> ) {
  
  return {};
}

[[nodiscard]] std::string formatSize(std::size_t n) {
  if (n < 1024)
    return std::to_string(n) + " B";
  if (n < 1024 * 1024)
    return std::to_string(n / 1024) + " KB";
  if (n < 1024 * 1024 * 1024ull)
    return std::to_string(n / (1024 * 1024)) + " MB";
  return std::to_string(n / (1024 * 1024 * 1024ull)) + " GB";
}

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

[[nodiscard]] std::string blNameFromMagic(std::uint16_t m, bool devkit) {
  
  
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
  
  default:
    return "BL";
  }
}

[[nodiscard]] std::optional<FileMetadata>
probeBootloader(std::span<const std::byte> data, std::string_view ) {
  if (data.size() < sizeof(bl_header))
    return std::nullopt;

  auto m = magic16(data, 0);
  
  
  bool retail_range =
      (m >= 0x0300 && m < 0x0400) || (m >= 0x0600 && m < 0x0700);
  if (!retail_range)
    return std::nullopt;

  auto type = static_cast<bl_type>(m & 0x0FFF);
  bool devkit = (magic16(data, 4) & 0x8000) != 0; 

  std::string name = blNameFromMagic(m, devkit);

  
  if ((m & 0x0FFF) == 0x342) {
    name = (devkit) ? "SB" : "CB";
    
    if (!devkit && magic16(data, 4) == 0x8000)
      name = "CB_B";
    
  }

  switch (type) {
  case CF: {
    CfMeta meta;
    meta.bl_name = name;
    meta.source_version = magic16(data, 2); 
    meta.target_version = magic16(data, 6); 
    meta.size = data.size();
    meta.sha1_hex = sha1Hex(data);
    
    
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
    
    
    
    return meta;
  }
  }
}

[[nodiscard]] std::optional<FileMetadata>
probeKeyvault(std::span<const std::byte> data) {
  if (data.size() != 0x4000)
    return std::nullopt;

  
  
  auto m = magic16(data, 0);
  if (m != 0xE4EE && m != 0xE4EF && (m >> 8) != 0xE4) {
    
    if (data[0] != std::byte{0x00} && data[0] != std::byte{0x01}) {
      return std::nullopt;
    }
  }

  KeyvaultMeta meta;
  meta.kv_type = static_cast<uint8_t>(data[0]);
  
  if (data.size() > 0x290) {
    meta.serial = std::string(reinterpret_cast<const char *>(&data[0x284]), 12);
  }
  
  if (data.size() > 0x110) {
    meta.dvd_key_hex = hexEncode(data.subspan(0x100, 0x10));
  }
  return meta;
}

[[nodiscard]] std::optional<FileMetadata>
probeXell(std::span<const std::byte> data) {
  if (data.size() < 4)
    return std::nullopt;
  if (!hasMagicStr(data, "\x7f"
                         "ELF"))
    return std::nullopt;

  
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
  
  for (std::size_t i = 0; i < kScan - 1; ++i) {
    if ((data[i] == std::byte{'1'} || data[i] == std::byte{'2'}) &&
        data[i + 1] == std::byte{'f'}) {
      meta.xell_type = std::string(1, static_cast<char>(data[i])) + "f";
      break;
    }
  }
  return meta;
}

} 

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

const char *entryKindSlintStr(EntryKind k) noexcept {
  switch (k) {
  case EntryKind::Dir:
    return "dir";
  case EntryKind::Nand:
    return "nand";
  case EntryKind::Stfs:
    return "stfs";
  case EntryKind::Pec:
    return "stfs"; 
  case EntryKind::Fatx:
    return "fatx";
  case EntryKind::FlashFs:
    return "flashfs";
  case EntryKind::SysUpdate:
    return "sysupdate";
  case EntryKind::Xcp:
    return "stfs"; 
  case EntryKind::Xdbf:
    return "file";
  case EntryKind::File:
    return "file";
  }
  return "file";
}

struct FsContext::Impl {
  EntryKind kind{EntryKind::File};

  
  std::filesystem::path dir_path;

  
  std::filesystem::path nand_path;
  std::vector<std::byte> nand_data; 

  
  std::vector<std::byte> buffer_data;

  
  std::filesystem::path flashfs_nand_path;
  std::vector<gxbuild3::bootloaders::FlashFileSystemEntry> flashfs_entries;

  
  mutable std::optional<nand_results_t> nand_results;

  
  
  bool loadNandData() const {
    if (!nand_data.empty())
      return true;
    auto &self = const_cast<Impl &>(*this);
    self.nand_data = readWholeFile(nand_path);
    return !self.nand_data.empty();
  }

  
  
  [[nodiscard]] std::vector<FsEntry> listNandRoot() const;
  [[nodiscard]] std::vector<FsEntry> listNandBootloaders() const;
};

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

  
  if (span.size() < sizeof(nand_header_t))
    return {};
  const auto *hdr = reinterpret_cast<const nand_header_t *>(span.data());

  
  if (hdr->smc_boot_offset != 0 && hdr->smc_boot_size != 0) {
    FsEntry e =
        makeEntry("smc", EntryKind::File, hdr->smc_boot_size, "SMC Firmware");
    entries.push_back(std::move(e));
  }

  
  if (hdr->kv_addr != 0 && hdr->smc_boot_size != 0) {
    entries.push_back(makeEntry("kv", EntryKind::File, 0x4000, "Keyvault"));
  }

  
  {
    FsEntry e = makeEntry("bootloaders", EntryKind::Dir, 0, "Boot chain");
    e.child_count = 4; 
    entries.push_back(std::move(e));
  }

  
  if (hdr->fs_addr != 0) {
    entries.push_back(makeEntry("flashfs", EntryKind::FlashFs, 0, "Type: XSB"));
  }

  
  {
    FsEntry e = makeEntry("sysupdate", EntryKind::SysUpdate, 0, "Slot: 0");
    entries.push_back(std::move(e));
  }

  return entries;
}

std::vector<FsEntry> FsContext::Impl::listNandBootloaders() const {
  
  
  
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

FsContext::FsContext() : impl_(std::make_unique<Impl>()) {}
FsContext::~FsContext() = default;
FsContext::FsContext(FsContext &&) noexcept = default;
FsContext &FsContext::operator=(FsContext &&) noexcept = default;

std::optional<FsContext>
FsContext::openPath(const std::filesystem::path &path) {
  std::error_code ec;
  if (!std::filesystem::exists(path, ec))
    return std::nullopt;

  
  if (std::filesystem::is_directory(path, ec)) {
    FsContext ctx;
    ctx.impl_->kind = EntryKind::Dir;
    ctx.impl_->dir_path = path;
    return ctx;
  }

  
  auto magic = readFileSlice(path, 0, 8);
  if (magic.size() < 4)
    return std::nullopt;
  auto span = std::span<const std::byte>(magic);

  
  if (magic16(span, 0) == 0x0F4D) {
    return openNandImage(path);
  }

  
  if (hasMagicStr(span, "PIRS") || hasMagicStr(span, "LIVE") ||
      hasMagicStr(span, "CON ")) {
    auto data = readWholeFile(path);
    if (data.empty())
      return std::nullopt;
    return openStfsBuffer(data);
  }

  
  
  
  
  {
    auto pec_magic = readFileSlice(path, 0x234, 4);
    
    
    
  }

  
  if (hasMagicStr(span, "FATX")) {
    return openFatx(path, 0, 0);
  }

  
  
  

  
  if (hasMagicStr(span, "XDBF")) {
    auto data = readWholeFile(path);
    if (data.empty())
      return std::nullopt;
    FsContext ctx;
    ctx.impl_->kind = EntryKind::Xdbf;
    ctx.impl_->buffer_data = std::move(data);
    return ctx;
  }

  
  FsContext ctx;
  ctx.impl_->kind = EntryKind::File;
  ctx.impl_->nand_path = path; 
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
  
  
  
  
  auto nand_data = readWholeFile(nand_image);
  if (nand_data.size() < sizeof(nand_header_t))
    return std::nullopt;

  const auto *hdr = reinterpret_cast<const nand_header_t *>(nand_data.data());
  const std::uint32_t fs_addr = hdr->fs_addr;
  if (fs_addr == 0 || fs_addr >= nand_data.size())
    return std::nullopt;

  
  
  
  
  

  FsContext ctx;
  ctx.impl_->kind = EntryKind::FlashFs;
  ctx.impl_->flashfs_nand_path = nand_image;
  
  return ctx;
}

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
  
}

std::vector<FsEntry> FsContext::list(std::string_view path) const {
  switch (impl_->kind) {

  
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

  
  case EntryKind::Nand: {
    if (path.empty() || path == "/")
      return impl_->listNandRoot();
    if (path == "/bootloaders" || path == "bootloaders")
      return impl_->listNandBootloaders();
    return {};
  }

  
  case EntryKind::Stfs:
  case EntryKind::Pec:
  case EntryKind::Xcp:
  case EntryKind::Fatx:
  case EntryKind::Xdbf:
  default:
    return {};
  }
}

std::optional<FileMetadata>
FsContext::probeBlob(std::span<const std::byte> data,
                     std::string_view hint_name) {
  if (data.empty())
    return std::nullopt;
  auto span4 = data.subspan(0, std::min<std::size_t>(data.size(), 8));

  
  if (magic32(span4, 0) == 0x464D494D) { 
    FmimMeta meta;
    meta.track_title = u"FMIM Audio Stream";
    return meta;
  }

  
  if (magic32(span4, 0) == 0x58455832) { 
    XexMeta meta;
    return meta;
  }

  
  if (magic32(span4, 0) == 0x58444246) { 
    XdbfMeta meta;
    return meta;
  }

  
  if (auto bl = probeBootloader(data, hint_name))
    return bl;

  
  if (data.size() == 0x4000) {
    if (auto kv = probeKeyvault(data))
      return kv;
  }

  
  if (auto xe = probeXell(data))
    return xe;

  
  GenericFileMeta generic;
  generic.size = data.size();
  generic.sha1_hex = sha1Hex(data);
  return generic;
}

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

    
    if (path == "kv" || path == "/kv") {
      data.assign(span.begin() + hdr->kv_addr,
                  span.begin() + hdr->kv_addr + 0x4000);
    } else if (path == "smc" || path == "/smc") {
      data.assign(span.begin() + hdr->smc_boot_offset,
                  span.begin() + hdr->smc_boot_offset + hdr->smc_boot_size);
    }
    
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
    return false; 
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

} 
