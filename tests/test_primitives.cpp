//
// Created by andiw on 25/01/2023.
//

#include <gtest/gtest.h>
#include <iostream>
#include <sstream>
using std::cout;

#include "../src/field.hpp"
//#include "nbt.h"

// void test_io(nbt::nbt_node const &node) {
//     nbt::write_to_file(node, "testfile.nbt");
//
//     const auto back_in = nbt::read_from_file("testfile.nbt");
//
//     ASSERT_EQ(back_in, node);
// }

void check_printing(NbtField const &field, std::string_view const result) {
    std::stringstream str{};
    str << field;

    ASSERT_EQ(str.str(), result);
}

TEST(Field, Printing) {
    check_printing(NbtField{"five", (uint8_t) 5}, "five: 5u8");
    check_printing(NbtField{"five", (int16_t) 5}, "five: 5i16");
    check_printing(NbtField{"five", 5}, "five: 5i32");
    check_printing(NbtField{"five", 5.0f}, "five: 5f32");
    check_printing(NbtField{"five", 5.0}, "five: 5f64");
    check_printing(NbtField{"fives", std::vector<byte>{{5, 5, 5}}}, "fives: [5, 5, 5, ] ([]u8)");
    check_printing(NbtField{"five", "five"}, "five: \"five\"");


    check_printing(NbtField{
                       "comp",
                       NbtCompound{
                           {
                               NbtField{"five_int", 5},
                               NbtField{"property", "inactive"},
                               NbtField{"byte array", std::vector<byte>{{1, 2, 3}}},
                           }
                       }
                   }, "comp: { five_int: 5i32, property: \"inactive\", byte array: [1, 2, 3, ] ([]u8), }");
}

TEST(Compound, Access) {
    NbtField inner{"inner", 5};
    NbtCompound a;
    a.fields.push_back(inner);

    NbtCompound b;
    b.fields.push_back(NbtField{"a", a});

    NbtCompound c;
    c.fields.push_back(NbtField{"b", b});

    std::vector<std::string> path = {"b", "a", "inner"};
    ASSERT_EQ(*c.find_path(path), inner);

    try {
        std::vector<std::string> incorrect_path = {"c", "a"};
        const auto _found = c.find_path(incorrect_path);
        FAIL() << "there should be an NbtError::FieldNotFound";
    } catch (NbtError const& err) {
        ASSERT_EQ(err, NbtError::FieldNotFound);
    }

    try {
        std::vector<std::string> path_notcompound = {"b", "a", "inner", "some_field"};
        const auto found = c.find_path(path_notcompound);
        FAIL() << "there should be an exception thrown by std::get";
    } catch (std::bad_variant_access const& e) {
        SUCCEED();
    }
}

// TEST(HelloTest, BasicAssertions) {
//     EXPECT_STRNE("hello", "world");
//
//     EXPECT_EQ(7 * 6, 42);
// }
//
// TEST(IO, PrimitiveTypes) {
//     using nbt::nbt_node;
//     using nbt::NbtTagType;
//     using nbt::compound;
//
//     const char int_name[] = "int";
//     int int_value = 10;
//
//         compound root{};
//
//         root.insert_node(int_value, int_name);
//
//         test_io(nbt_node{std::move(root)});
// }
//
//
// TEST(NBTTags, TAGInt) {
//     using nbt::nbt_node;
//     using nbt::NbtTagType;
//
//     int value = 10;
//
//     nbt_node int_node = value;
//     ASSERT_EQ(int_node.get<NbtTagType::TAG_Int>(), value);
// }
//
// TEST(NBTTags, TAG_Long) {
//     using nbt::nbt_node;
//     using nbt::NbtTagType;
//     int64_t value = 10;
//
//     nbt_node int_node = value;
//     ASSERT_EQ(int_node.get<NbtTagType::TAG_Long>(), value);
// }
//
// TEST(NBTTags, TAG_Float) {
//     using nbt::nbt_node;
//     using nbt::NbtTagType;
//     float value = 10.0f;
//
//     nbt_node int_node = value;
//     ASSERT_EQ(int_node.get<NbtTagType::TAG_Float>(), value);
// }
//
// TEST(NBTTags, TAG_Double) {
//     using nbt::nbt_node;
//     using nbt::NbtTagType;
//     double value = 10.0;
//
//     nbt_node int_node = value;
//     nbt_node int_node2 = value;
//     ASSERT_EQ(int_node, int_node2);
// }
//
// TEST(NBTTags, TAG_Byte) {
//     using nbt::nbt_node;
//     using nbt::NbtTagType;
//     unsigned char value = 10;
//
//     nbt_node int_node = value;
//     ASSERT_EQ(int_node.get<NbtTagType::TAG_Byte>(), value);
// }
