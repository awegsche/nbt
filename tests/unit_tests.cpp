
#include <gtest/gtest.h>
#include <iostream>

#include "common.h"
#include "nbt.h"
#include <vector>


TEST(UnitTests, Swapped2)
{
    std::vector<unsigned char> buffer;

    int16_t value = 12 | 34 << 8;

    push_swapped2(buffer, &value);

    ASSERT_EQ(buffer.size(), 2);
    ASSERT_EQ(buffer[0], 34);
    ASSERT_EQ(buffer[1], 12);
}

TEST(UnitTests, Swapped4)
{
    std::vector<unsigned char> buffer;

    int32_t value = 12 | 34 << 8 | 56 << 16 | 78 << 24;

    push_swapped4(buffer, &value);

    ASSERT_EQ(buffer.size(), 4);
    ASSERT_EQ(buffer[0], 78);
    ASSERT_EQ(buffer[1], 56);
    ASSERT_EQ(buffer[2], 34);
    ASSERT_EQ(buffer[3], 12);
}

TEST(UnitTests, Swapped8)
{
    std::vector<unsigned char> buffer;

    int64_t value = 12 | 34 << 8 | 56 << 16 | 78 << 24 | 90LL << 32 | 123LL << 40 | 145LL << 48 | 167LL << 56;

    push_swapped8(buffer, &value);

    ASSERT_EQ(buffer.size(), 8);
    ASSERT_EQ(buffer[0], 167);
    ASSERT_EQ(buffer[1], 145);
    ASSERT_EQ(buffer[2], 123);
    ASSERT_EQ(buffer[3], 90);
    ASSERT_EQ(buffer[4], 78);
    ASSERT_EQ(buffer[5], 56);
    ASSERT_EQ(buffer[6], 34);
    ASSERT_EQ(buffer[7], 12);
}