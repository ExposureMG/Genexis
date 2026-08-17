#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <span>
#include <string>
#include <string_view>

namespace Utils {

[[nodiscard]] std::uint16_t readBE16(const std::byte *ptr) noexcept;
[[nodiscard]] std::uint16_t readBE16(std::span<const std::byte> data, std::size_t offset = 0);

[[nodiscard]] std::uint32_t readBE32(const std::byte *ptr) noexcept;
[[nodiscard]] std::uint32_t readBE32(std::span<const std::byte> data, std::size_t offset = 0);

[[nodiscard]] std::uint64_t readBE64(const std::byte *ptr) noexcept;
[[nodiscard]] std::uint64_t readBE64(std::span<const std::byte> data, std::size_t offset = 0);

[[nodiscard]] std::uint32_t readUInt24BE(const std::byte *ptr) noexcept;
[[nodiscard]] std::uint32_t readUInt24BE(std::span<const std::byte> data, std::size_t offset = 0);

[[nodiscard]] std::uint16_t readLE16(const std::byte *ptr) noexcept;
[[nodiscard]] std::uint16_t readLE16(std::span<const std::byte> data, std::size_t offset = 0);

[[nodiscard]] std::uint32_t readLE32(const std::byte *ptr) noexcept;
[[nodiscard]] std::uint32_t readLE32(std::span<const std::byte> data, std::size_t offset = 0);

[[nodiscard]] std::uint64_t readLE64(const std::byte *ptr) noexcept;
[[nodiscard]] std::uint64_t readLE64(std::span<const std::byte> data, std::size_t offset = 0);

[[nodiscard]] std::uint32_t readUInt24LE(const std::byte *ptr) noexcept;
[[nodiscard]] std::uint32_t readUInt24LE(std::span<const std::byte> data, std::size_t offset = 0);

[[nodiscard]] std::uint16_t readU16(const std::byte *ptr, bool big_endian) noexcept;
[[nodiscard]] std::uint32_t readU32(const std::byte *ptr, bool big_endian) noexcept;
[[nodiscard]] std::uint64_t readU64(const std::byte *ptr, bool big_endian) noexcept;

[[nodiscard]] std::string lowerAscii(std::string_view value);
[[nodiscard]] std::u16string readUtf16LeString(const std::byte *ptr, std::size_t max_bytes);
[[nodiscard]] std::u16string readNullTerminatedUtf16Le(const std::byte *ptr,
                                                       std::size_t max_bytes,
                                                       std::size_t &bytes_read);
[[nodiscard]] std::filesystem::path safeJoin(const std::filesystem::path &parent,
                                             std::string_view child);

} 
