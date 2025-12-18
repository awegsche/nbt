//
// Tests for byte-order (endianness) handling
//

#include <gtest/gtest.h>
#include <cstdint>
#include <limits>
#include <bit>
#include <vector>

#include "common.h"

using namespace nbt;

// ---- from_big_endian / to_big_endian Tests ----

TEST(Endian, Int16Roundtrip)
{
    int16_t values[] = {0, 1, -1, 127, -128, 32767, -32768, 12345, -12345};
    
    for (int16_t val : values) {
        auto be = to_big_endian(val);
        auto back = from_big_endian(be);
        ASSERT_EQ(back, val) << "Failed for value: " << val;
    }
}

TEST(Endian, Int32Roundtrip)
{
    int32_t values[] = {
        0, 1, -1, 
        127, -128, 
        32767, -32768,
        2147483647, -2147483647 - 1,
        123456789, -123456789
    };
    
    for (int32_t val : values) {
        auto be = to_big_endian(val);
        auto back = from_big_endian(be);
        ASSERT_EQ(back, val) << "Failed for value: " << val;
    }
}

TEST(Endian, Int64Roundtrip)
{
    int64_t values[] = {
        0, 1, -1,
        std::numeric_limits<int64_t>::max(),
        std::numeric_limits<int64_t>::min(),
        123456789012345LL,
        -123456789012345LL
    };
    
    for (int64_t val : values) {
        auto be = to_big_endian(val);
        auto back = from_big_endian(be);
        ASSERT_EQ(back, val) << "Failed for value: " << val;
    }
}

TEST(Endian, UInt16Roundtrip)
{
    uint16_t values[] = {0, 1, 255, 256, 65535, 12345};
    
    for (uint16_t val : values) {
        auto be = to_big_endian(val);
        auto back = from_big_endian(be);
        ASSERT_EQ(back, val) << "Failed for value: " << val;
    }
}

TEST(Endian, UInt32Roundtrip)
{
    uint32_t values[] = {0, 1, 255, 65535, 4294967295U, 123456789U};
    
    for (uint32_t val : values) {
        auto be = to_big_endian(val);
        auto back = from_big_endian(be);
        ASSERT_EQ(back, val) << "Failed for value: " << val;
    }
}

TEST(Endian, UInt64Roundtrip)
{
    uint64_t values[] = {
        0, 1,
        std::numeric_limits<uint64_t>::max(),
        123456789012345ULL
    };
    
    for (uint64_t val : values) {
        auto be = to_big_endian(val);
        auto back = from_big_endian(be);
        ASSERT_EQ(back, val) << "Failed for value: " << val;
    }
}

// ---- Buffer Read/Write Tests ----

TEST(EndianBuffer, WriteRead16)
{
    std::vector<unsigned char> buffer;
    
    int16_t values[] = {0, 1, -1, 12345, -12345, 32767, -32768};
    
    for (int16_t val : values) {
        write_i16(buffer, val);
    }
    
    const char* ptr = reinterpret_cast<const char*>(buffer.data());
    for (int16_t expected : values) {
        int16_t actual = read_i16(ptr);
        ASSERT_EQ(actual, expected);
    }
}

TEST(EndianBuffer, WriteRead32)
{
    std::vector<unsigned char> buffer;
    
    int32_t values[] = {0, 1, -1, 123456789, -123456789, 2147483647, -2147483647 - 1};
    
    for (int32_t val : values) {
        write_i32(buffer, val);
    }
    
    const char* ptr = reinterpret_cast<const char*>(buffer.data());
    for (int32_t expected : values) {
        int32_t actual = read_i32(ptr);
        ASSERT_EQ(actual, expected);
    }
}

TEST(EndianBuffer, WriteRead64)
{
    std::vector<unsigned char> buffer;
    
    int64_t values[] = {
        0, 1, -1, 
        123456789012345LL, 
        -123456789012345LL,
        std::numeric_limits<int64_t>::max(),
        std::numeric_limits<int64_t>::min()
    };
    
    for (int64_t val : values) {
        write_i64(buffer, val);
    }
    
    const char* ptr = reinterpret_cast<const char*>(buffer.data());
    for (int64_t expected : values) {
        int64_t actual = read_i64(ptr);
        ASSERT_EQ(actual, expected);
    }
}

TEST(EndianBuffer, WriteReadFloat)
{
    std::vector<unsigned char> buffer;
    
    float values[] = {0.0f, 1.0f, -1.0f, 3.14159f, -2.71828f, 
                      std::numeric_limits<float>::max(),
                      std::numeric_limits<float>::min(),
                      std::numeric_limits<float>::epsilon()};
    
    for (float val : values) {
        write_f32(buffer, val);
    }
    
    const char* ptr = reinterpret_cast<const char*>(buffer.data());
    for (float expected : values) {
        float actual = read_f32(ptr);
        ASSERT_FLOAT_EQ(actual, expected);
    }
}

TEST(EndianBuffer, WriteReadDouble)
{
    std::vector<unsigned char> buffer;
    
    double values[] = {0.0, 1.0, -1.0, 3.141592653589793, -2.718281828459045,
                       std::numeric_limits<double>::max(),
                       std::numeric_limits<double>::min(),
                       std::numeric_limits<double>::epsilon()};
    
    for (double val : values) {
        write_f64(buffer, val);
    }
    
    const char* ptr = reinterpret_cast<const char*>(buffer.data());
    for (double expected : values) {
        double actual = read_f64(ptr);
        ASSERT_DOUBLE_EQ(actual, expected);
    }
}

// ---- Legacy Function Tests ----

TEST(EndianLegacy, Swap2)
{
    std::vector<unsigned char> buffer;
    int16_t value = 0x1234;
    
    push_swapped2(buffer, &value);
    
    ASSERT_EQ(buffer.size(), 2);
    // On little-endian, bytes should be swapped
    if constexpr (std::endian::native == std::endian::little) {
        ASSERT_EQ(buffer[0], 0x12);
        ASSERT_EQ(buffer[1], 0x34);
    }
    
    // Verify roundtrip
    auto back = __swap2(buffer.data());
    ASSERT_EQ(back, static_cast<uint16_t>(value));
}

TEST(EndianLegacy, Swap4)
{
    std::vector<unsigned char> buffer;
    int32_t value = 0x12345678;
    
    push_swapped4(buffer, &value);
    
    ASSERT_EQ(buffer.size(), 4);
    if constexpr (std::endian::native == std::endian::little) {
        ASSERT_EQ(buffer[0], 0x12);
        ASSERT_EQ(buffer[1], 0x34);
        ASSERT_EQ(buffer[2], 0x56);
        ASSERT_EQ(buffer[3], 0x78);
    }
    
    auto back = __swap4(buffer.data());
    ASSERT_EQ(back, static_cast<uint32_t>(value));
}

TEST(EndianLegacy, Swap8)
{
    std::vector<unsigned char> buffer;
    int64_t value = 0x123456789ABCDEF0LL;
    
    push_swapped8(buffer, &value);
    
    ASSERT_EQ(buffer.size(), 8);
    if constexpr (std::endian::native == std::endian::little) {
        ASSERT_EQ(buffer[0], 0x12);
        ASSERT_EQ(buffer[1], 0x34);
        ASSERT_EQ(buffer[2], 0x56);
        ASSERT_EQ(buffer[3], 0x78);
        ASSERT_EQ(buffer[4], 0x9A);
        ASSERT_EQ(buffer[5], 0xBC);
        ASSERT_EQ(buffer[6], 0xDE);
        ASSERT_EQ(buffer[7], 0xF0);
    }
    
    auto back = __swap8(buffer.data());
    ASSERT_EQ(back, static_cast<uint64_t>(value));
}

// ---- Known Value Tests ----

TEST(EndianKnown, BigEndianInt32)
{
    // Big-endian representation of 0x01020304 should be {0x01, 0x02, 0x03, 0x04}
    unsigned char be_bytes[] = {0x01, 0x02, 0x03, 0x04};
    
    auto value = __swap4(be_bytes);
    ASSERT_EQ(value, 0x01020304u);
}

TEST(EndianKnown, BigEndianInt16)
{
    unsigned char be_bytes[] = {0x01, 0x02};
    
    auto value = __swap2(be_bytes);
    ASSERT_EQ(value, 0x0102u);
}

TEST(EndianKnown, BigEndianInt64)
{
    unsigned char be_bytes[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    
    auto value = __swap8(be_bytes);
    ASSERT_EQ(value, 0x0102030405060708ull);
}

