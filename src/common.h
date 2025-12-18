#pragma once

#include <bit>
#include <cstdint>
#include <cstring>
#include <concepts>
#include <vector>

#include <spdlog/spdlog.h>

using spdlog::debug;
using spdlog::error;
using spdlog::info;
using spdlog::warn;

namespace nbt {

// ---- Byte Swap Implementation ----
// Provide our own byteswap for compilers that don't fully support C++23

namespace detail {

constexpr uint16_t byteswap(uint16_t value) noexcept {
    return static_cast<uint16_t>((value << 8) | (value >> 8));
}

constexpr int16_t byteswap(int16_t value) noexcept {
    return static_cast<int16_t>(byteswap(static_cast<uint16_t>(value)));
}

constexpr uint32_t byteswap(uint32_t value) noexcept {
    return ((value & 0x000000FFu) << 24) |
           ((value & 0x0000FF00u) << 8)  |
           ((value & 0x00FF0000u) >> 8)  |
           ((value & 0xFF000000u) >> 24);
}

constexpr int32_t byteswap(int32_t value) noexcept {
    return static_cast<int32_t>(byteswap(static_cast<uint32_t>(value)));
}

constexpr uint64_t byteswap(uint64_t value) noexcept {
    return ((value & 0x00000000000000FFull) << 56) |
           ((value & 0x000000000000FF00ull) << 40) |
           ((value & 0x0000000000FF0000ull) << 24) |
           ((value & 0x00000000FF000000ull) << 8)  |
           ((value & 0x000000FF00000000ull) >> 8)  |
           ((value & 0x0000FF0000000000ull) >> 24) |
           ((value & 0x00FF000000000000ull) >> 40) |
           ((value & 0xFF00000000000000ull) >> 56);
}

constexpr int64_t byteswap(int64_t value) noexcept {
    return static_cast<int64_t>(byteswap(static_cast<uint64_t>(value)));
}

}  // namespace detail

// ---- Big-Endian Conversion Utilities ----
// NBT format uses big-endian byte order. These functions convert between
// native and big-endian representations.

/// Convert an integral value from big-endian to native byte order
template<std::integral T>
constexpr T from_big_endian(T value) {
    if constexpr (std::endian::native == std::endian::big) {
        return value;
    } else {
        return detail::byteswap(value);
    }
}

/// Convert an integral value from native to big-endian byte order
template<std::integral T>
constexpr T to_big_endian(T value) {
    return from_big_endian(value);  // symmetric operation
}

/// Convert a floating-point value from big-endian to native byte order
template<std::floating_point T>
constexpr T from_big_endian_float(T value) {
    if constexpr (std::endian::native == std::endian::big) {
        return value;
    } else {
        if constexpr (sizeof(T) == 4) {
            auto bits = std::bit_cast<uint32_t>(value);
            return std::bit_cast<T>(detail::byteswap(bits));
        } else {
            auto bits = std::bit_cast<uint64_t>(value);
            return std::bit_cast<T>(detail::byteswap(bits));
        }
    }
}

/// Convert a floating-point value from native to big-endian byte order
template<std::floating_point T>
constexpr T to_big_endian_float(T value) {
    return from_big_endian_float(value);  // symmetric operation
}

// ---- Buffer Reading Utilities ----

/// Read a 16-bit big-endian integer from buffer and advance pointer
inline int16_t read_i16(const char*& buffer) {
    int16_t value;
    std::memcpy(&value, buffer, sizeof(value));
    buffer += sizeof(value);
    return from_big_endian(value);
}

/// Read a 32-bit big-endian integer from buffer and advance pointer
inline int32_t read_i32(const char*& buffer) {
    int32_t value;
    std::memcpy(&value, buffer, sizeof(value));
    buffer += sizeof(value);
    return from_big_endian(value);
}

/// Read a 64-bit big-endian integer from buffer and advance pointer
inline int64_t read_i64(const char*& buffer) {
    int64_t value;
    std::memcpy(&value, buffer, sizeof(value));
    buffer += sizeof(value);
    return from_big_endian(value);
}

/// Read a 32-bit big-endian float from buffer and advance pointer
inline float read_f32(const char*& buffer) {
    uint32_t bits;
    std::memcpy(&bits, buffer, sizeof(bits));
    buffer += sizeof(bits);
    return std::bit_cast<float>(from_big_endian(bits));
}

/// Read a 64-bit big-endian double from buffer and advance pointer
inline double read_f64(const char*& buffer) {
    uint64_t bits;
    std::memcpy(&bits, buffer, sizeof(bits));
    buffer += sizeof(bits);
    return std::bit_cast<double>(from_big_endian(bits));
}

// ---- Buffer Writing Utilities ----

/// Write a 16-bit integer as big-endian to buffer
inline void write_i16(std::vector<unsigned char>& buffer, int16_t value) {
    auto be = to_big_endian(value);
    auto* bytes = reinterpret_cast<const unsigned char*>(&be);
    buffer.insert(buffer.end(), bytes, bytes + sizeof(be));
}

/// Write a 32-bit integer as big-endian to buffer
inline void write_i32(std::vector<unsigned char>& buffer, int32_t value) {
    auto be = to_big_endian(value);
    auto* bytes = reinterpret_cast<const unsigned char*>(&be);
    buffer.insert(buffer.end(), bytes, bytes + sizeof(be));
}

/// Write a 64-bit integer as big-endian to buffer
inline void write_i64(std::vector<unsigned char>& buffer, int64_t value) {
    auto be = to_big_endian(value);
    auto* bytes = reinterpret_cast<const unsigned char*>(&be);
    buffer.insert(buffer.end(), bytes, bytes + sizeof(be));
}

/// Write a 32-bit float as big-endian to buffer
inline void write_f32(std::vector<unsigned char>& buffer, float value) {
    auto bits = std::bit_cast<uint32_t>(value);
    auto be = to_big_endian(bits);
    auto* bytes = reinterpret_cast<const unsigned char*>(&be);
    buffer.insert(buffer.end(), bytes, bytes + sizeof(be));
}

/// Write a 64-bit double as big-endian to buffer
inline void write_f64(std::vector<unsigned char>& buffer, double value) {
    auto bits = std::bit_cast<uint64_t>(value);
    auto be = to_big_endian(bits);
    auto* bytes = reinterpret_cast<const unsigned char*>(&be);
    buffer.insert(buffer.end(), bytes, bytes + sizeof(be));
}

}  // namespace nbt

// ---- Legacy Compatibility ----
// These functions maintain backwards compatibility with existing code.
// They delegate to the new nbt:: namespace functions.

template<typename T>
uint16_t __swap2(const T* _val) {
    uint16_t value;
    std::memcpy(&value, _val, sizeof(value));
    return nbt::from_big_endian(value);
}

template<typename T>
uint32_t __swap4(const T* _val) {
    uint32_t value;
    std::memcpy(&value, _val, sizeof(value));
    return nbt::from_big_endian(value);
}

template<typename T>
uint64_t __swap8(const T* _val) {
    uint64_t value;
    std::memcpy(&value, _val, sizeof(value));
    return nbt::from_big_endian(value);
}

template<typename T>
void push_swapped2(std::vector<unsigned char>& buffer, const T* value) {
    static_assert(sizeof(T) == 2);
    int16_t v;
    std::memcpy(&v, value, sizeof(v));
    nbt::write_i16(buffer, v);
}

template<typename T>
void push_swapped4(std::vector<unsigned char>& buffer, const T* value) {
    static_assert(sizeof(T) == 4);
    int32_t v;
    std::memcpy(&v, value, sizeof(v));
    nbt::write_i32(buffer, v);
}

template<typename T>
void push_swapped8(std::vector<unsigned char>& buffer, const T* value) {
    static_assert(sizeof(T) == 8);
    int64_t v;
    std::memcpy(&v, value, sizeof(v));
    nbt::write_i64(buffer, v);
}
