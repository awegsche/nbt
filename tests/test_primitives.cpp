//
// Created by andiw on 25/01/2023.
//

#include <gtest/gtest.h>
#include <iostream>

#include "nbt.h"

TEST(HelloTest, BasicAssertions) {
    EXPECT_STRNE("hello", "world");

    EXPECT_EQ(7 * 6, 42);
}

TEST(IO, PrimitiveTypes) {
    using nbt::nbt_node;
    using nbt::NbtTagType;
    using nbt::compound;

    const char int_name[] = "int";
    int int_value = 10;

    {
        compound root{};

        root.insert_node(int_value, int_name);

        std::cout << "write_output" << std::endl;

        nbt::write_to_file(std::move(root), "testfile.nbt");
    }

    {
        std::cout << "read input" << std::endl;
        const auto back_in = nbt::read_from_file("testfile.nbt");

        ASSERT_EQ(back_in.at(int_name)->get<NbtTagType::TAG_Int>(), int_value);
    }
}


TEST(NBTTags, TAGInt) {
    using nbt::nbt_node;
    using nbt::NbtTagType;

    int value = 10;

    nbt_node int_node = value;
    ASSERT_EQ(int_node.get<NbtTagType::TAG_Int>(), value);
}

TEST(NBTTags, TAG_Long) {
    using nbt::nbt_node;
    using nbt::NbtTagType;
    int64_t value = 10;

    nbt_node int_node = value;
    ASSERT_EQ(int_node.get<NbtTagType::TAG_Long>(), value);
}

TEST(NBTTags, TAG_Float) {
    using nbt::nbt_node;
    using nbt::NbtTagType;
    float value = 10.0f;

    nbt_node int_node = value;
    ASSERT_EQ(int_node.get<NbtTagType::TAG_Float>(), value);
}

TEST(NBTTags, TAG_Double) {
    using nbt::nbt_node;
    using nbt::NbtTagType;
    double value = 10.0;

    nbt_node int_node = value;
    ASSERT_EQ(int_node.get<NbtTagType::TAG_Double>(), value);
}

TEST(NBTTags, TAG_Byte) {
    using nbt::nbt_node;
    using nbt::NbtTagType;
    unsigned char value = 10;

    nbt_node int_node = value;
    ASSERT_EQ(int_node.get<NbtTagType::TAG_Byte>(), value);
}
