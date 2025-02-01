//
// Created by andiw on 25/01/2023.
//

#include <gtest/gtest.h>
#include <iostream>
#include <sstream>
#include <string_view>

#include "nbt.hpp"
using std::cout;

#include "../src/field.hpp"
#include "../src/compound.hpp"
#include "../src/nbt_error.hpp"

namespace {
    void check_printing(NbtField const &field, std::string_view const result) {
        std::stringstream str{};
        str << field;

        ASSERT_EQ(str.str(), result);
    }
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
    ASSERT_EQ(c.find_path(path), inner);

    try {
        std::vector<std::string> incorrect_path = {"c", "a"};
        const auto _found = c.find_path(incorrect_path);
        FAIL() << "there should be an NbtError::FieldNotFound";
    } catch (NbtFieldNotFoundError const &err) {
        ASSERT_STREQ(err.what(), "Field c not found");
    }

    try {
        std::vector<std::string> path_notcompound = {"b", "a", "inner", "some_field"};
        const auto found = c.find_path(path_notcompound);
        FAIL() << "there should be an exception thrown by std::get";
    } catch (std::bad_variant_access const &e) {
        SUCCEED();
    }
}

namespace {
    void check_buffer_io(NbtField const &field) {
        std::vector<uint8_t> buffer{};
        auto it = std::back_inserter(buffer);
        field.to_buffer(it);
        auto ot = buffer.begin();
        auto const read_back = NbtField::from_buffer(ot);
        ASSERT_EQ(read_back, field);
    }

    void check_file_io(NbtField const &field) {
        write_nbt(field, "test_file.nbt");
        auto const read_back = read_nbt("test_file.nbt");
        ASSERT_EQ(read_back, field);
    }

    template<typename Fn>
    void check_fields(Fn fn) {
        fn(NbtField{"five", 5});
        fn(NbtField{"five", (int64_t) 5});
        fn(NbtField{"five", (int16_t) 5});
        fn(NbtField{"five", (byte) 5});
        fn(NbtField{"byte array", std::vector<byte>{1, 2, 3}});
        fn(NbtField{"int array", std::vector<int>{1, 2, 3}});
        fn(NbtField{"long array", std::vector<int64_t>{1, 2, 3}});
        fn(NbtField{"string", std::string{"hello world"}});
        fn(NbtField{
            "compound", NbtCompound{
                {
                    NbtField{"int1", 1},
                    NbtField{"float", 3.14f}
                }
            }
        });
        fn(NbtField{
            "list_float", NbtList{
                std::vector{1.0f, 2.0f, 3.0f}
            }
        });
        fn(NbtField{
            "list_double", NbtList{
                std::vector{1.0, 2.0, 3.0}
            }
        });
        fn(NbtField{
            "list_list", NbtList{
                std::vector{
                    NbtList{std::vector{1.0f, 2.0f, 3.0f}},
                    NbtList{std::vector{1.0f, 2.0f, 3.0f}},
                    NbtList{std::vector{1.0f, 2.0f, 3.0f}},
                }
            }
        });
    }
}


TEST(SerDe, ToBuffer) {
    check_fields(check_buffer_io);
}

TEST(SerDe, ToFile) {
    check_fields(check_file_io);
}
