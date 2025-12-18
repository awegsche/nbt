//
// Tests for gzip compression functionality
//

#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>

#include "nbt.h"

using nbt::nbt_node;
using nbt::compound;
using nbt::NbtTagType;

namespace fs = std::filesystem;

// ---- Basic Gzip Roundtrip Tests ----

TEST(Gzip, IntRoundtrip)
{
    compound root;
    root.insert_node(12345, "value");
    
    nbt_node node{std::move(root)};
    node.name = "root";
    
    nbt::write_to_file_gzip(node, "test_gzip_int.nbt");
    auto read = nbt::read_from_file_gzip("test_gzip_int.nbt");
    
    ASSERT_EQ(read.tagtype(), NbtTagType::TAG_Compound);
    auto* value = read.at("value");
    ASSERT_NE(value, nullptr);
    ASSERT_EQ(value->get<NbtTagType::TAG_Int>(), 12345);
}

TEST(Gzip, StringRoundtrip)
{
    compound root;
    root.insert_node(std::string("Hello, Minecraft!"), "message");
    
    nbt_node node{std::move(root)};
    node.name = "root";
    
    nbt::write_to_file_gzip(node, "test_gzip_string.nbt");
    auto read = nbt::read_from_file_gzip("test_gzip_string.nbt");
    
    auto* message = read.at("message");
    ASSERT_NE(message, nullptr);
    ASSERT_EQ(message->get<NbtTagType::TAG_String>(), "Hello, Minecraft!");
}

TEST(Gzip, ComplexRoundtrip)
{
    compound root;
    root.insert_node(42, "intValue");
    root.insert_node(3.14159265358979, "doubleValue");
    root.insert_node(std::string("test string"), "stringValue");
    root.insert_node(std::vector<int32_t>{1, 2, 3, 4, 5}, "intArray");
    
    // Nested compound
    compound nested;
    nested.insert_node(100, "nestedInt");
    root.insert_node(std::move(nested), "nested");
    
    nbt_node node{std::move(root)};
    node.name = "root";
    
    nbt::write_to_file_gzip(node, "test_gzip_complex.nbt");
    auto read = nbt::read_from_file_gzip("test_gzip_complex.nbt");
    
    ASSERT_EQ(read.at("intValue")->get<NbtTagType::TAG_Int>(), 42);
    ASSERT_DOUBLE_EQ(read.at("doubleValue")->get<NbtTagType::TAG_Double>(), 3.14159265358979);
    ASSERT_EQ(read.at("stringValue")->get<NbtTagType::TAG_String>(), "test string");
    
    auto& arr = read.at("intArray")->get<NbtTagType::TAG_Int_Array>();
    ASSERT_EQ(arr.size(), 5);
    ASSERT_EQ(arr[0], 1);
    ASSERT_EQ(arr[4], 5);
    
    auto* nest = read.at("nested");
    ASSERT_NE(nest, nullptr);
    ASSERT_EQ(nest->at("nestedInt")->get<NbtTagType::TAG_Int>(), 100);
}

// ---- Uncompressed File Tests ----

TEST(Uncompressed, IntRoundtrip)
{
    compound root;
    root.insert_node(67890, "value");
    
    nbt_node node{std::move(root)};
    node.name = "root";
    
    nbt::write_to_file_uncompressed(node, "test_uncompressed.nbt");
    auto read = nbt::read_from_file_uncompressed("test_uncompressed.nbt");
    
    ASSERT_EQ(read.at("value")->get<NbtTagType::TAG_Int>(), 67890);
}

TEST(Uncompressed, ComplexRoundtrip)
{
    compound root;
    root.insert_node(int64_t{9876543210LL}, "longValue");
    root.insert_node(2.5f, "floatValue");
    root.insert_node(std::vector<byte>{1, 2, 3, 4, 5}, "byteArray");
    
    nbt_node node{std::move(root)};
    node.name = "root";
    
    nbt::write_to_file_uncompressed(node, "test_uncompressed_complex.nbt");
    auto read = nbt::read_from_file_uncompressed("test_uncompressed_complex.nbt");
    
    ASSERT_EQ(read.at("longValue")->get<NbtTagType::TAG_Long>(), 9876543210LL);
    ASSERT_FLOAT_EQ(read.at("floatValue")->get<NbtTagType::TAG_Float>(), 2.5f);
    
    auto& arr = read.at("byteArray")->get<NbtTagType::TAG_Byte_Array>();
    ASSERT_EQ(arr.size(), 5);
}

// ---- File Size Comparison ----

TEST(Gzip, CompressionRatio)
{
    // Create a large, compressible structure
    compound root;
    
    // Repetitive data compresses well
    std::vector<int32_t> data;
    for (int i = 0; i < 1000; i++) {
        data.push_back(i % 10);  // Very repetitive
    }
    root.insert_node(data, "repetitive");
    
    nbt_node node{std::move(root)};
    node.name = "root";
    
    nbt::write_to_file_gzip(node, "test_compression_gzip.nbt");
    nbt::write_to_file_uncompressed(node, "test_compression_raw.nbt");
    
    auto gzip_size = fs::file_size("test_compression_gzip.nbt");
    auto raw_size = fs::file_size("test_compression_raw.nbt");
    
    // Gzip should be significantly smaller for repetitive data
    ASSERT_LT(gzip_size, raw_size);
    
    // Verify both can be read back correctly
    auto gzip_read = nbt::read_from_file_gzip("test_compression_gzip.nbt");
    auto raw_read = nbt::read_from_file_uncompressed("test_compression_raw.nbt");
    
    auto& gzip_arr = gzip_read.at("repetitive")->get<NbtTagType::TAG_Int_Array>();
    auto& raw_arr = raw_read.at("repetitive")->get<NbtTagType::TAG_Int_Array>();
    
    ASSERT_EQ(gzip_arr.size(), 1000);
    ASSERT_EQ(raw_arr.size(), 1000);
    ASSERT_EQ(gzip_arr, raw_arr);
}

// ---- Error Handling ----

TEST(Gzip, NonExistentFileThrows)
{
    ASSERT_THROW(nbt::read_from_file_gzip("nonexistent_file.nbt"), std::runtime_error);
}

TEST(Uncompressed, NonExistentFileThrows)
{
    ASSERT_THROW(nbt::read_from_file_uncompressed("nonexistent_file.nbt"), std::runtime_error);
}

// ---- Edge Cases ----

TEST(Gzip, EmptyCompound)
{
    compound root;  // Empty compound
    
    nbt_node node{std::move(root)};
    node.name = "empty";
    
    nbt::write_to_file_gzip(node, "test_gzip_empty.nbt");
    auto read = nbt::read_from_file_gzip("test_gzip_empty.nbt");
    
    ASSERT_EQ(read.tagtype(), NbtTagType::TAG_Compound);
    ASSERT_EQ(read.get<NbtTagType::TAG_Compound>().content.size(), 0);
}

TEST(Gzip, LargeFile)
{
    compound root;
    
    // Create a reasonably large structure
    for (int i = 0; i < 100; i++) {
        std::vector<int64_t> arr;
        for (int j = 0; j < 100; j++) {
            arr.push_back(static_cast<int64_t>(i) * 1000 + j);
        }
        root.insert_node(arr, "array_" + std::to_string(i));
    }
    
    nbt_node node{std::move(root)};
    node.name = "large";
    
    nbt::write_to_file_gzip(node, "test_gzip_large.nbt");
    auto read = nbt::read_from_file_gzip("test_gzip_large.nbt");
    
    ASSERT_EQ(read.get<NbtTagType::TAG_Compound>().content.size(), 100);
    
    auto* arr50 = read.at("array_50");
    ASSERT_NE(arr50, nullptr);
    auto& values = arr50->get<NbtTagType::TAG_Long_Array>();
    ASSERT_EQ(values.size(), 100);
    ASSERT_EQ(values[25], 50025);
}

TEST(Gzip, UnicodeString)
{
    compound root;
    root.insert_node(std::string("日本語テスト"), "japanese");
    root.insert_node(std::string("Ümläuts änd spëcïäl chäräctërs"), "german");
    root.insert_node(std::string("🎮🎲🎯"), "emoji");
    
    nbt_node node{std::move(root)};
    node.name = "unicode";
    
    nbt::write_to_file_gzip(node, "test_gzip_unicode.nbt");
    auto read = nbt::read_from_file_gzip("test_gzip_unicode.nbt");
    
    ASSERT_EQ(read.at("japanese")->get<NbtTagType::TAG_String>(), "日本語テスト");
    ASSERT_EQ(read.at("german")->get<NbtTagType::TAG_String>(), "Ümläuts änd spëcïäl chäräctërs");
    ASSERT_EQ(read.at("emoji")->get<NbtTagType::TAG_String>(), "🎮🎲🎯");
}

