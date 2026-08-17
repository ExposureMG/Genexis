#pragma once

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

enum class EntryKind {
  Dir,       
  Nand,      
  Stfs,      
  Pec,       
  Fatx,      
  FlashFs,   
  SysUpdate, 
  Xcp,       
  Xdbf,      
  File,      
             
};

[[nodiscard]] const char *entryKindLabel(EntryKind k) noexcept;

[[nodiscard]] const char *entryKindSlintStr(EntryKind k) noexcept;

struct FsEntry {
  std::string name; 
  EntryKind kind{EntryKind::File};
  std::size_t size{0};        
  std::string extra_info;     
  std::size_t child_count{0}; 
};

struct GenericFileMeta {
  std::size_t size{0};
  std::string sha1_hex;      
  std::string crypt_key_hex; 
};

struct BootloaderMeta {
  std::string
      bl_name; 
  std::uint16_t version{0}; 
  std::uint8_t lockdown_value{
      0}; 
  std::uint32_t pairing_data{0}; 
  std::size_t size{0};
  std::string sha1_hex;
  std::string crypt_key_hex;
  bool is_devkit{false};
};

struct CfMeta {
  std::string bl_name;             
  std::uint16_t source_version{0}; 
  std::uint16_t target_version{0}; 
  std::uint32_t pairing_data{0};   
  std::uint8_t lockdown_value{0};  
  std::uint8_t update_slot{0};     
  std::size_t size{0};
  std::string sha1_hex;
  std::string crypt_key_hex;
};

struct CgMeta {
  std::string bl_name;      
  std::uint16_t version{0}; 
  std::size_t size{0};
  std::string sha1_hex;
  std::string crypt_key_hex;
};

struct KeyvaultMeta {
  std::uint8_t kv_type{0};    
  std::string mfr_date;       
  std::string serial;         
  std::string console_id_hex; 
  std::string region_hex;     
  std::string
      dvd_key_hex;      
  bool has_osig{false}; 
};

struct SmcFwMeta {
  std::string smc_type;         
  std::string firmware_type;    
  std::string firmware_version; 
};

struct SmcConfigMeta {
  std::uint8_t fan_min{0};
  std::uint8_t fan_max{0};
  std::uint32_t game_region{0};
  
  std::vector<std::byte> raw_config;
};

struct XellMeta {
  std::string xell_type;    
  std::string xell_version; 
};

struct FatxFileMeta {
  std::size_t size{0};
  std::uint8_t attributes{0};
  bool is_read_only{false};
  bool is_hidden{false};
  bool is_system{false};
};

struct FmimMeta {
  std::u16string track_title;
  std::u16string album_name;
  std::u16string artist;
  std::u16string genre;
  std::uint32_t track_length_ms{0};
  std::uint32_t track_number{0};
};

struct XexMeta {
  std::uint32_t module_flags{0};
  std::uint32_t title_id{0}; 
  std::uint16_t version_major{0};
  std::uint16_t version_minor{0};
};

struct GpdMeta {
  std::int32_t achievement_count{0};
  std::int32_t gamerscore_total{0};
  std::u16string title_name;
};

struct SpaMeta {
  std::uint32_t title_id{0};
  int achievement_count{0};
  std::string version_str; 
};

struct XdbfMeta {
  std::uint32_t version{0};
  int entry_count{0};
};

using FileMetadata =
    std::variant<GenericFileMeta, BootloaderMeta, CfMeta, CgMeta, KeyvaultMeta,
                 SmcFwMeta, SmcConfigMeta, XellMeta, FatxFileMeta, FmimMeta,
                 XexMeta, GpdMeta, SpaMeta, XdbfMeta
                 
                 
                 >;

class FsContext {
public:
  FsContext();
  ~FsContext();

  FsContext(const FsContext &) = delete;
  FsContext &operator=(const FsContext &) = delete;

  FsContext(FsContext &&) noexcept;
  FsContext &operator=(FsContext &&) noexcept;

  
  
  

  
  
  
  [[nodiscard]] static std::optional<FsContext>
  openPath(const std::filesystem::path &path);

  
  [[nodiscard]] static std::optional<FsContext>
  openNandImage(const std::filesystem::path &path);

  
  
  [[nodiscard]] static std::optional<FsContext>
  openStfsBuffer(std::span<const std::byte> data);

  
  
  [[nodiscard]] static std::optional<FsContext>
  openFatx(const std::filesystem::path &image, std::uint64_t offset = 0,
           std::uint64_t size = 0);

  
  
  [[nodiscard]] static std::optional<FsContext>
  openFlashFs(const std::filesystem::path &nand_image);

  
  
  

  void close();
  [[nodiscard]] bool isOpen() const noexcept;
  [[nodiscard]] EntryKind kind() const noexcept;
  [[nodiscard]] bool isWritable() const noexcept;

  
  
  

  
  
  
  
  [[nodiscard]] std::vector<FsEntry> list(std::string_view path = "/") const;

  
  
  

  
  
  [[nodiscard]] std::optional<FileMetadata> probe(std::string_view path) const;

  
  
  
  [[nodiscard]] static std::optional<FileMetadata>
  probeBlob(std::span<const std::byte> data, std::string_view hint_name = "");

  
  [[nodiscard]] std::vector<std::byte> read(std::string_view path) const;

  
  
  bool write(std::string_view path, std::span<const std::byte> data);

  
  bool mkdir(std::string_view path);

  
  bool remove(std::string_view path);

  
  bool extractAll(std::string_view src_path,
                  const std::filesystem::path &dest_dir) const;

  
  
  

  
  
  [[nodiscard]] std::string extraInfo() const;

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

} 
