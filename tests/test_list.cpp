//
// Comprehensive tests for TAG_List implementation
//

#include <gtest/gtest.h>
#include <vector>
#include <string>

#include "nbt.h"

using nbt::nbt_node;
using nbt::nbt_list;
using nbt::compound;
using nbt::NbtTagType;

// ---- Basic List Creation Tests ----

TEST(TagList, EmptyList)
{
    nbt_list list;
    // Default-constructed list has TagEnd content (empty)
    ASSERT_EQ(list.content_type(), NbtTagType::TAG_END);
}

TEST(TagList, ByteList)
{
    nbt_list list;
    list.content = std::vector<byte>{1, 2, 3, 4, 5};
    
    ASSERT_EQ(list.content_type(), NbtTagType::TAG_Byte);
    auto& vec = list.get<NbtTagType::TAG_Byte>();
    ASSERT_EQ(vec.size(), 5);
    ASSERT_EQ(vec[0], 1);
    ASSERT_EQ(vec[4], 5);
}

TEST(TagList, ShortList)
{
    nbt_list list;
    list.content = std::vector<int16_t>{100, 200, 300, -100, -200};
    
    ASSERT_EQ(list.content_type(), NbtTagType::TAG_Short);
    auto& vec = list.get<NbtTagType::TAG_Short>();
    ASSERT_EQ(vec.size(), 5);
    ASSERT_EQ(vec[0], 100);
    ASSERT_EQ(vec[3], -100);
}

TEST(TagList, IntList)
{
    nbt_list list;
    list.content = std::vector<int32_t>{1000000, -1000000, 0, 2147483647, -2147483648};
    
    ASSERT_EQ(list.content_type(), NbtTagType::TAG_Int);
    auto& vec = list.get<NbtTagType::TAG_Int>();
    ASSERT_EQ(vec.size(), 5);
    ASSERT_EQ(vec[3], 2147483647);
}

TEST(TagList, LongList)
{
    nbt_list list;
    list.content = std::vector<int64_t>{
        9223372036854775807LL, 
        -9223372036854775807LL - 1, 
        0
    };
    
    ASSERT_EQ(list.content_type(), NbtTagType::TAG_Long);
    auto& vec = list.get<NbtTagType::TAG_Long>();
    ASSERT_EQ(vec.size(), 3);
    ASSERT_EQ(vec[0], 9223372036854775807LL);
}

TEST(TagList, FloatList)
{
    nbt_list list;
    list.content = std::vector<float>{1.5f, -2.5f, 0.0f, 3.14159f};
    
    ASSERT_EQ(list.content_type(), NbtTagType::TAG_Float);
    auto& vec = list.get<NbtTagType::TAG_Float>();
    ASSERT_EQ(vec.size(), 4);
    ASSERT_FLOAT_EQ(vec[3], 3.14159f);
}

TEST(TagList, DoubleList)
{
    nbt_list list;
    list.content = std::vector<double>{1.5, -2.5, 0.0, 3.141592653589793};
    
    ASSERT_EQ(list.content_type(), NbtTagType::TAG_Double);
    auto& vec = list.get<NbtTagType::TAG_Double>();
    ASSERT_EQ(vec.size(), 4);
    ASSERT_DOUBLE_EQ(vec[3], 3.141592653589793);
}

TEST(TagList, StringList)
{
    nbt_list list;
    list.content = std::vector<std::string>{"hello", "world", "nbt", ""};
    
    ASSERT_EQ(list.content_type(), NbtTagType::TAG_String);
    auto& vec = list.get<NbtTagType::TAG_String>();
    ASSERT_EQ(vec.size(), 4);
    ASSERT_EQ(vec[0], "hello");
    ASSERT_EQ(vec[3], "");  // Empty string
}

TEST(TagList, CompoundList)
{
    nbt_list list;
    
    // Create a vector of compounds
    std::vector<compound> compounds;
    
    compound c1;
    c1.insert_node(42, "value");
    compounds.push_back(std::move(c1));
    
    compound c2;
    c2.insert_node(std::string("test"), "name");
    compounds.push_back(std::move(c2));
    
    list.content = std::move(compounds);
    
    ASSERT_EQ(list.content_type(), NbtTagType::TAG_Compound);
    auto& vec = list.get<NbtTagType::TAG_Compound>();
    ASSERT_EQ(vec.size(), 2);
}

// ---- Roundtrip Tests (Write + Read) ----

TEST(TagListIO, IntListRoundtrip)
{
    compound root;
    
    nbt_list list;
    list.content = std::vector<int32_t>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    nbt_node list_node{std::move(list)};
    list_node.name = "numbers";
    root.content.push_back(std::move(list_node));
    
    nbt_node root_node{std::move(root)};
    root_node.name = "root";
    
    nbt::write_to_file(root_node, "test_list_int.nbt");
    auto read_node = nbt::read_from_file("test_list_int.nbt");
    
    ASSERT_EQ(read_node.tagtype(), NbtTagType::TAG_Compound);
    
    auto* numbers = read_node.at("numbers");
    ASSERT_NE(numbers, nullptr);
    ASSERT_EQ(numbers->tagtype(), NbtTagType::TAG_List);
    
    auto& read_list = numbers->get<NbtTagType::TAG_List>();
    ASSERT_EQ(read_list.content_type(), NbtTagType::TAG_Int);
    
    auto& vec = read_list.get<NbtTagType::TAG_Int>();
    ASSERT_EQ(vec.size(), 10);
    for (int i = 0; i < 10; i++) {
        ASSERT_EQ(vec[i], i + 1);
    }
}

TEST(TagListIO, StringListRoundtrip)
{
    compound root;
    
    nbt_list list;
    list.content = std::vector<std::string>{"alpha", "beta", "gamma", "delta"};
    
    nbt_node list_node{std::move(list)};
    list_node.name = "greeks";
    root.content.push_back(std::move(list_node));
    
    nbt_node root_node{std::move(root)};
    root_node.name = "root";
    
    nbt::write_to_file(root_node, "test_list_string.nbt");
    auto read_node = nbt::read_from_file("test_list_string.nbt");
    
    auto* greeks = read_node.at("greeks");
    ASSERT_NE(greeks, nullptr);
    
    auto& read_list = greeks->get<NbtTagType::TAG_List>();
    ASSERT_EQ(read_list.content_type(), NbtTagType::TAG_String);
    
    auto& vec = read_list.get<NbtTagType::TAG_String>();
    ASSERT_EQ(vec.size(), 4);
    ASSERT_EQ(vec[0], "alpha");
    ASSERT_EQ(vec[3], "delta");
}

TEST(TagListIO, FloatListRoundtrip)
{
    compound root;
    
    nbt_list list;
    list.content = std::vector<float>{1.0f, 2.5f, -3.7f, 0.0f};
    
    nbt_node list_node{std::move(list)};
    list_node.name = "floats";
    root.content.push_back(std::move(list_node));
    
    nbt_node root_node{std::move(root)};
    root_node.name = "root";
    
    nbt::write_to_file(root_node, "test_list_float.nbt");
    auto read_node = nbt::read_from_file("test_list_float.nbt");
    
    auto* floats = read_node.at("floats");
    ASSERT_NE(floats, nullptr);
    
    auto& vec = floats->get<NbtTagType::TAG_List>().get<NbtTagType::TAG_Float>();
    ASSERT_EQ(vec.size(), 4);
    ASSERT_FLOAT_EQ(vec[0], 1.0f);
    ASSERT_FLOAT_EQ(vec[2], -3.7f);
}

TEST(TagListIO, CompoundListRoundtrip)
{
    compound root;
    
    nbt_list list;
    std::vector<compound> items;
    
    for (int i = 0; i < 3; i++) {
        compound item;
        item.insert_node(i, "id");
        item.insert_node(std::string("item_" + std::to_string(i)), "name");
        items.push_back(std::move(item));
    }
    
    list.content = std::move(items);
    
    nbt_node list_node{std::move(list)};
    list_node.name = "items";
    root.content.push_back(std::move(list_node));
    
    nbt_node root_node{std::move(root)};
    root_node.name = "root";
    
    nbt::write_to_file(root_node, "test_list_compound.nbt");
    auto read_node = nbt::read_from_file("test_list_compound.nbt");
    
    auto* items_node = read_node.at("items");
    ASSERT_NE(items_node, nullptr);
    
    auto& compounds = items_node->get<NbtTagType::TAG_List>().get<NbtTagType::TAG_Compound>();
    ASSERT_EQ(compounds.size(), 3);
    
    // Check first compound
    auto* id = compounds[0]["id"];
    ASSERT_NE(id, nullptr);
    ASSERT_EQ(id->get<NbtTagType::TAG_Int>(), 0);
    
    auto* name = compounds[2]["name"];
    ASSERT_NE(name, nullptr);
    ASSERT_EQ(name->get<NbtTagType::TAG_String>(), "item_2");
}

TEST(TagListIO, NestedListRoundtrip)
{
    compound root;
    
    // Create a list of lists of ints
    nbt_list outer_list;
    std::vector<nbt_list> inner_lists;
    
    for (int i = 0; i < 3; i++) {
        nbt_list inner;
        std::vector<int32_t> values;
        for (int j = 0; j < 4; j++) {
            values.push_back(i * 10 + j);
        }
        inner.content = std::move(values);
        inner_lists.push_back(std::move(inner));
    }
    
    outer_list.content = std::move(inner_lists);
    
    nbt_node list_node{std::move(outer_list)};
    list_node.name = "matrix";
    root.content.push_back(std::move(list_node));
    
    nbt_node root_node{std::move(root)};
    root_node.name = "root";
    
    nbt::write_to_file(root_node, "test_list_nested.nbt");
    auto read_node = nbt::read_from_file("test_list_nested.nbt");
    
    auto* matrix = read_node.at("matrix");
    ASSERT_NE(matrix, nullptr);
    
    auto& outer = matrix->get<NbtTagType::TAG_List>();
    ASSERT_EQ(outer.content_type(), NbtTagType::TAG_List);
    
    auto& rows = outer.get<NbtTagType::TAG_List>();
    ASSERT_EQ(rows.size(), 3);
    
    // Check second row, third element (should be 12)
    auto& row1 = rows[1];
    ASSERT_EQ(row1.content_type(), NbtTagType::TAG_Int);
    ASSERT_EQ(row1.get<NbtTagType::TAG_Int>()[2], 12);
}

// ---- Edge Cases ----

TEST(TagListIO, EmptyListRoundtrip)
{
    compound root;
    
    nbt_list list;
    // Empty list stays as TagEnd
    
    nbt_node list_node{std::move(list)};
    list_node.name = "empty";
    root.content.push_back(std::move(list_node));
    
    nbt_node root_node{std::move(root)};
    root_node.name = "root";
    
    nbt::write_to_file(root_node, "test_list_empty.nbt");
    auto read_node = nbt::read_from_file("test_list_empty.nbt");
    
    auto* empty = read_node.at("empty");
    ASSERT_NE(empty, nullptr);
    ASSERT_EQ(empty->tagtype(), NbtTagType::TAG_List);
    ASSERT_EQ(empty->get<NbtTagType::TAG_List>().content_type(), NbtTagType::TAG_END);
}

TEST(TagListIO, LargeListRoundtrip)
{
    compound root;
    
    nbt_list list;
    std::vector<int32_t> values;
    for (int i = 0; i < 10000; i++) {
        values.push_back(i);
    }
    list.content = std::move(values);
    
    nbt_node list_node{std::move(list)};
    list_node.name = "large";
    root.content.push_back(std::move(list_node));
    
    nbt_node root_node{std::move(root)};
    root_node.name = "root";
    
    nbt::write_to_file(root_node, "test_list_large.nbt");
    auto read_node = nbt::read_from_file("test_list_large.nbt");
    
    auto* large = read_node.at("large");
    ASSERT_NE(large, nullptr);
    
    auto& vec = large->get<NbtTagType::TAG_List>().get<NbtTagType::TAG_Int>();
    ASSERT_EQ(vec.size(), 10000);
    ASSERT_EQ(vec[0], 0);
    ASSERT_EQ(vec[9999], 9999);
}

TEST(TagListIO, ByteArrayListRoundtrip)
{
    compound root;
    
    nbt_list list;
    std::vector<std::vector<byte>> arrays;
    
    for (int i = 0; i < 3; i++) {
        std::vector<byte> arr;
        for (int j = 0; j < 5; j++) {
            arr.push_back(static_cast<byte>(i * 10 + j));
        }
        arrays.push_back(std::move(arr));
    }
    
    list.content = std::move(arrays);
    
    nbt_node list_node{std::move(list)};
    list_node.name = "byte_arrays";
    root.content.push_back(std::move(list_node));
    
    nbt_node root_node{std::move(root)};
    root_node.name = "root";
    
    nbt::write_to_file(root_node, "test_list_byte_arrays.nbt");
    auto read_node = nbt::read_from_file("test_list_byte_arrays.nbt");
    
    auto* ba = read_node.at("byte_arrays");
    ASSERT_NE(ba, nullptr);
    
    auto& arrays_read = ba->get<NbtTagType::TAG_List>().get<NbtTagType::TAG_Byte_Array>();
    ASSERT_EQ(arrays_read.size(), 3);
    ASSERT_EQ(arrays_read[1][2], 12);
}

