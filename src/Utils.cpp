#include "Utils.hpp"

#include <algorithm>
#include <cctype>
#include <stdexcept>

namespace Utils {

std::uint16_t readBE16(const std::byte *ptr) noexcept {
  return (static_cast<std::uint16_t>(ptr[0]) << 8) |
         static_cast<std::uint16_t>(ptr[1]);
}

std::uint16_t readBE16(std::span<const std::byte> data, std::size_t offset) {
  if (offset + 2 > data.size()) {
    throw std::runtime_error("Unexpected end of buffer reading u16 BE");
  }
  return readBE16(data.data() + offset);
}

std::uint32_t readBE32(const std::byte *ptr) noexcept {
  return (static_cast<std::uint32_t>(ptr[0]) << 24) |
         (static_cast<std::uint32_t>(ptr[1]) << 16) |
         (static_cast<std::uint32_t>(ptr[2]) << 8) |
         static_cast<std::uint32_t>(ptr[3]);
}

std::uint32_t readBE32(std::span<const std::byte> data, std::size_t offset) {
  if (offset + 4 > data.size()) {
    throw std::runtime_error("Unexpected end of buffer reading u32 BE");
  }
  return readBE32(data.data() + offset);
}

std::uint64_t readBE64(const std::byte *ptr) noexcept {
  std::uint64_t result = 0;
  for (int i = 0; i < 8; ++i) {
    result = (result << 8) | static_cast<std::uint64_t>(ptr[i]);
  }
  return result;
}

std::uint64_t readBE64(std::span<const std::byte> data, std::size_t offset) {
  if (offset + 8 > data.size()) {
    throw std::runtime_error("Unexpected end of buffer reading u64 BE");
  }
  return readBE64(data.data() + offset);
}

std::uint32_t readUInt24BE(const std::byte *ptr) noexcept {
  return (static_cast<std::uint32_t>(ptr[0]) << 16) |
         (static_cast<std::uint32_t>(ptr[1]) << 8) |
         static_cast<std::uint32_t>(ptr[2]);
}

std::uint32_t readUInt24BE(std::span<const std::byte> data,
                           std::size_t offset) {
  if (offset + 3 > data.size()) {
    throw std::runtime_error("Unexpected end of buffer reading u24 BE");
  }
  return readUInt24BE(data.data() + offset);
}

std::uint16_t readLE16(const std::byte *ptr) noexcept {
  return static_cast<std::uint16_t>(ptr[0]) |
         (static_cast<std::uint16_t>(ptr[1]) << 8);
}

std::uint16_t readLE16(std::span<const std::byte> data, std::size_t offset) {
  if (offset + 2 > data.size()) {
    throw std::runtime_error("Unexpected end of buffer reading u16 LE");
  }
  return readLE16(data.data() + offset);
}

std::uint32_t readLE32(const std::byte *ptr) noexcept {
  return static_cast<std::uint32_t>(ptr[0]) |
         (static_cast<std::uint32_t>(ptr[1]) << 8) |
         (static_cast<std::uint32_t>(ptr[2]) << 16) |
         (static_cast<std::uint32_t>(ptr[3]) << 24);
}

std::uint32_t readLE32(std::span<const std::byte> data, std::size_t offset) {
  if (offset + 4 > data.size()) {
    throw std::runtime_error("Unexpected end of buffer reading u32 LE");
  }
  return readLE32(data.data() + offset);
}

std::uint64_t readLE64(const std::byte *ptr) noexcept {
  std::uint64_t result = 0;
  for (int i = 7; i >= 0; --i) {
    result = (result << 8) | static_cast<std::uint64_t>(ptr[i]);
  }
  return result;
}

std::uint64_t readLE64(std::span<const std::byte> data, std::size_t offset) {
  if (offset + 8 > data.size()) {
    throw std::runtime_error("Unexpected end of buffer reading u64 LE");
  }
  return readLE64(data.data() + offset);
}

std::uint32_t readUInt24LE(const std::byte *ptr) noexcept {
  return static_cast<std::uint32_t>(ptr[0]) |
         (static_cast<std::uint32_t>(ptr[1]) << 8) |
         (static_cast<std::uint32_t>(ptr[2]) << 16);
}

std::uint32_t readUInt24LE(std::span<const std::byte> data,
                           std::size_t offset) {
  if (offset + 3 > data.size()) {
    throw std::runtime_error("Unexpected end of buffer reading u24 LE");
  }
  return readUInt24LE(data.data() + offset);
}

std::uint16_t readU16(const std::byte *ptr, bool big_endian) noexcept {
  return big_endian ? readBE16(ptr) : readLE16(ptr);
}

std::uint32_t readU32(const std::byte *ptr, bool big_endian) noexcept {
  return big_endian ? readBE32(ptr) : readLE32(ptr);
}

std::uint64_t readU64(const std::byte *ptr, bool big_endian) noexcept {
  return big_endian ? readBE64(ptr) : readLE64(ptr);
}

std::string lowerAscii(std::string_view value) {
  std::string result{value};
  std::transform(
      result.begin(), result.end(), result.begin(),
      [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
  return result;
}

std::u16string readUtf16LeString(const std::byte *ptr, std::size_t max_bytes) {
  std::u16string str;
  const std::size_t max_chars = max_bytes / 2;
  str.reserve(max_chars);

  for (std::size_t i = 0; i < max_chars; ++i) {
    std::uint16_t ch = static_cast<std::uint8_t>(ptr[i * 2]) |
                       (static_cast<std::uint8_t>(ptr[i * 2 + 1]) << 8);
    if (ch == 0) {
      break;
    }
    str.push_back(static_cast<char16_t>(ch));
  }
  return str;
}

std::u16string readNullTerminatedUtf16Le(const std::byte *ptr,
                                         std::size_t max_bytes,
                                         std::size_t &bytes_read) {
  std::u16string str;
  bytes_read = 0;
  const std::size_t max_chars = max_bytes / 2;

  for (std::size_t i = 0; i < max_chars; ++i) {
    std::uint16_t ch = static_cast<std::uint8_t>(ptr[i * 2]) |
                       (static_cast<std::uint8_t>(ptr[i * 2 + 1]) << 8);
    bytes_read += 2;
    if (ch == 0) {
      break;
    }
    str.push_back(static_cast<char16_t>(ch));
  }
  return str;
}

std::filesystem::path safeJoin(const std::filesystem::path &parent,
                               std::string_view child) {
  std::filesystem::path child_path{std::string{child}};
  if (child_path.is_absolute()) {
    throw std::runtime_error("Entry uses an absolute path");
  }

  const auto normalized = child_path.lexically_normal();
  for (const auto &part : normalized) {
    if (part == "..") {
      throw std::runtime_error("Entry escapes target directory");
    }
  }

  return parent / normalized;
}

} // namespace Utils
