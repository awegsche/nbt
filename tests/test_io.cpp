#include <gtest/gtest.h>
#include <iostream>

#include "nbt.h"

TEST(IO, NbtNodeIntRoundtrip)
{
    using nbt::nbt_node;
    using nbt::NbtTagType;

    int32_t value = 12345;
    std::string name = "myInt";

    // Create node and set name
    nbt_node node = value;
    node.name = name;

    // Write to file
    nbt::write_to_file(node, "test_nbt_int.nbt");

    // Read from file
    nbt_node read_node = nbt::read_from_file("test_nbt_int.nbt");

    // Check tag type, name, and value
    ASSERT_EQ(read_node.tagtype(), NbtTagType::TAG_Int);
    ASSERT_EQ(read_node.name, name);
    ASSERT_EQ(read_node.get<NbtTagType::TAG_Int>(), value);
}

TEST(IO, NbtNodeLongRoundtrip)
{
    using nbt::nbt_node;
    using nbt::NbtTagType;

    int64_t value = 1234567890123456789LL;
    std::string name = "myLong";

    // Create node and set name
    nbt_node node = value;
    node.name = name;

    // Write to file
    nbt::write_to_file(node, "test_nbt_long.nbt");

    // Read from file
    nbt_node read_node = nbt::read_from_file("test_nbt_long.nbt");

    // Check tag type, name, and value
    ASSERT_EQ(read_node.tagtype(), NbtTagType::TAG_Long);
    ASSERT_EQ(read_node.name, "myLong");
    ASSERT_EQ(read_node.get<NbtTagType::TAG_Long>(), value);
}

TEST(IO, NbtNodeFloatRoundtrip)
{
    using nbt::nbt_node;
    using nbt::NbtTagType;

    float value = 12345.67f;
    std::string name = "myFloat";

    // Create node and set name
    nbt_node node = value;
    node.name = name;

    // Write to file
    nbt::write_to_file(node, "test_nbt_float.nbt");

    // Read from file
    nbt_node read_node = nbt::read_from_file("test_nbt_float.nbt");

    // Check tag type, name, and value
    ASSERT_EQ(read_node.tagtype(), NbtTagType::TAG_Float);
    ASSERT_EQ(read_node.name, "myFloat");
    ASSERT_EQ(read_node.get<NbtTagType::TAG_Float>(), value);
}

TEST(IO, NbtNodeDoubleRoundtrip)
{
    using nbt::nbt_node;
    using nbt::NbtTagType;

    double value = 12345.672349890123;
    std::string name = "myDouble";

    // Create node and set name
    nbt_node node = value;
    node.name = name;

    // Write to file
    nbt::write_to_file(node, "test_nbt_double.nbt");

    // Read from file
    nbt_node read_node = nbt::read_from_file("test_nbt_double.nbt");

    // Check tag type, name, and value
    ASSERT_EQ(read_node.tagtype(), NbtTagType::TAG_Double);
    ASSERT_EQ(read_node.name, "myDouble");
    ASSERT_EQ(read_node.get<NbtTagType::TAG_Double>(), value);
}

TEST(IO, NbtNodeShortRoundtrip)
{
    using nbt::nbt_node;
    using nbt::NbtTagType;

    int16_t value = 12345;
    std::string name = "myShort";

    // Create node and set name
    nbt_node node = value;
    node.name = name;

    // Write to file
    nbt::write_to_file(node, "test_nbt_short.nbt");

    // Read from file
    nbt_node read_node = nbt::read_from_file("test_nbt_short.nbt");

    // Check tag type, name, and value
    ASSERT_EQ(read_node.tagtype(), NbtTagType::TAG_Short);
    ASSERT_EQ(read_node.name, "myShort");
    ASSERT_EQ(read_node.get<NbtTagType::TAG_Short>(), value);
}


TEST(IO, NbtNodeByteRoundtrip)
{
    using nbt::nbt_node;
    using nbt::NbtTagType;

    uint8_t value = 123;
    std::string name = "myByte";

    // Create node and set name
    nbt_node node = value;
    node.name = name;

    // Write to file
    nbt::write_to_file(node, "test_nbt_byte.nbt");

    // Read from file
    nbt_node read_node = nbt::read_from_file("test_nbt_byte.nbt");

    // Check tag type, name, and value
    ASSERT_EQ(read_node.tagtype(), NbtTagType::TAG_Byte);
    ASSERT_EQ(read_node.name, "myByte");
    ASSERT_EQ(read_node.get<NbtTagType::TAG_Byte>(), value);
}

TEST(IO, NbtNodeIntArrayRoundtrip)
{
    using nbt::nbt_node;
    using nbt::NbtTagType;

    std::vector<int32_t> value = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    std::string name = "myIntArray";

    // Create node and set name
    nbt_node node = value;
    node.name = name;

    // Write to file
    nbt::write_to_file(node, "test_nbt_int_array.nbt");

    // Read from file
    nbt_node read_node = nbt::read_from_file("test_nbt_int_array.nbt");

    // Check tag type, name, and value
    ASSERT_EQ(read_node.tagtype(), NbtTagType::TAG_Int_Array);
    ASSERT_EQ(read_node.name, "myIntArray");
    ASSERT_EQ(read_node.get<NbtTagType::TAG_Int_Array>(), value);
}

TEST(IO, NbtNodeLongArrayRoundtrip)
{
    using nbt::nbt_node;
    using nbt::NbtTagType;

    std::vector<int64_t> value = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    std::string name = "myLongArray";

    // Create node and set name
    nbt_node node = value;
    node.name = name;

    // Write to file
    nbt::write_to_file(node, "test_nbt_long_array.nbt");

    // Read from file
    nbt_node read_node = nbt::read_from_file("test_nbt_long_array.nbt");

    // Check tag type, name, and value
    ASSERT_EQ(read_node.tagtype(), NbtTagType::TAG_Long_Array);
    ASSERT_EQ(read_node.name, "myLongArray");
    ASSERT_EQ(read_node.get<NbtTagType::TAG_Long_Array>(), value);
}

TEST(IO, NbtNodeByteArrayRoundtrip)
{
    using nbt::nbt_node;
    using nbt::NbtTagType;

    std::vector<uint8_t> value = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    std::string name = "myByteArray";

    // Create node and set name
    nbt_node node = value;
    node.name = name;

    // Write to file
    nbt::write_to_file(node, "test_nbt_byte_array.nbt");

    // Read from file
    nbt_node read_node = nbt::read_from_file("test_nbt_byte_array.nbt");

    // Check tag type, name, and value
    ASSERT_EQ(read_node.tagtype(), NbtTagType::TAG_Byte_Array);
    ASSERT_EQ(read_node.name, "myByteArray");
    ASSERT_EQ(read_node.get<NbtTagType::TAG_Byte_Array>(), value);
}

TEST(IO, NbtNodeStringRoundtrip)
{
    using nbt::nbt_node;
    using nbt::NbtTagType;

    std::string value = "Hello, NBT!";
    std::string name = "myString";

    // Create node and set name
    nbt_node node = value;
    node.name = name;

    // Write to file
    nbt::write_to_file(node, "test_nbt_string.nbt");

    // Read from file
    nbt_node read_node = nbt::read_from_file("test_nbt_string.nbt");

    // Check tag type, name, and value
    ASSERT_EQ(read_node.tagtype(), NbtTagType::TAG_String);
    ASSERT_EQ(read_node.name, "myString");
    ASSERT_EQ(read_node.get<NbtTagType::TAG_String>(), value);
}
