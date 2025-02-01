//
// Created by andiw on 30/01/2025.
//

#ifndef FIELD_HPP
#define FIELD_HPP

#include <cstdint>
#include <variant>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <nbt_error.hpp>
#include <span>

#include "helpers.hpp"
#include "end.hpp"
#include "tagtype.hpp"
#include "compound.hpp"
#include "list.hpp"

using NbtValue = std::variant<TagEnd, byte, int16_t, int32_t, int64_t, float, double, std::vector<byte>, std::string,
    NbtList, NbtCompound, std::vector<int32_t>, std::vector<int64_t> >;

struct NbtField {
    std::string name;
    NbtValue value;

    // ---- Init -------------------------------------------------------------------------------------------------------

    NbtField()
        : value(TagEnd{}) {
    }

    template<typename T>
    NbtField(std::string_view const name, T &&value)
        : name(name)
          , value(value) {
    }

    //template<>
    //NbtField(std::string_view const name, std::vector<float>&& float_list)
    //    :name(name), value(NbtList{std::move(float_list)}) {

    //    }

    // ---- Access -----------------------------------------------------------------------------------------------------

    /// Find a field in this compound
    ///
    /// Throws:
    /// - std::bad_variant_access if `this` is not a compound
    /// - NbtError::FieldNotFound if there is no field with the given name.
    ///
    [[nodiscard]] NbtField const &find(std::string_view const key) const;

    /// Find a field by path in this compound
    ///
    /// Throws:
    /// - std::bad_variant_access if any of the non-leaf fields is not a compound
    /// - NbtError::FieldNotFound if there is no field with the given name.
    ///
    [[nodiscard]] NbtField const &find_path(std::span<std::string> const path) const;

    // ---- IO ---------------------------------------------------------------------------------------------------------

    template<typename InputIt>
    static NbtField from_buffer(InputIt &buffer) {
        auto const tag = static_cast<NbtTagType>(*buffer++);

        switch (tag) {
            case TAG_END:
                return NbtField{"", TagEnd{}};
            case TAG_Byte: {
                const auto name = read_name(buffer);
                return NbtField{name, static_cast<byte>(*buffer++)};
            }
            case TAG_Short: {
                const auto name = read_name(buffer);
                return NbtField{name, pull_swapped<int16_t>(buffer)};
            }
            case TAG_Int: {
                const auto name = read_name(buffer);
                return NbtField{name, pull_swapped<int32_t>(buffer)};
            }
            case TAG_Long: {
                const auto name = read_name(buffer);
                return NbtField{name, pull_swapped<int64_t>(buffer)};
            }
            case TAG_Float: {
                const auto name = read_name(buffer);
                return NbtField{name, pull_swapped<float>(buffer)};
            }
            case TAG_Double: {
                const auto name = read_name(buffer);
                return NbtField{name, pull_swapped<double>(buffer)};
            }
            case TAG_Byte_Array: {
                const auto name = read_name(buffer);
                return NbtField{name, read_vector<byte>(buffer)};
            }
            case TAG_Int_Array: {
                const auto name = read_name(buffer);
                return NbtField{name, read_vector<int32_t>(buffer)};
            }
            case TAG_Long_Array: {
                const auto name = read_name(buffer);
                return NbtField{name, read_vector<int64_t>(buffer)};
            }
            case TAG_String: {
                const auto name = read_name(buffer);
                return NbtField{name, read_string(buffer)};
            }
            case TAG_Compound: {
                const auto name = read_name(buffer);
                return NbtField{name, read_compound(buffer)};
            }
            case TAG_List: {
                const auto name = read_name(buffer);
                return NbtField(name, NbtList::from_buffer(buffer));
            }
            default:
                throw NbtInvalidTagError{tag};
        }
    }

    template<typename OutputIt>
    void to_buffer(OutputIt &buffer) const {
        std::visit(overloaded{
                       [&](TagEnd) { *buffer++ = NbtTagType::TAG_END; },
                       [&](byte value) {
                           TagWriteFull::write(buffer, NbtTagType::TAG_Byte, name);
                           *buffer++ = value;
                       },
                       [&](int16_t value) {
                           TagWriteFull::write(buffer, NbtTagType::TAG_Short, name);
                           push_swapped(buffer, value);
                       },
                       [&](int32_t value) {
                           TagWriteFull::write(buffer, NbtTagType::TAG_Int, name);
                           push_swapped(buffer, value);
                       },
                       [&](int64_t value) {
                           TagWriteFull::write(buffer, NbtTagType::TAG_Long, name);
                           push_swapped(buffer, value);
                       },
                       [&](float value) {
                           TagWriteFull::write(buffer, NbtTagType::TAG_Float, name);
                           push_swapped(buffer, value);
                       },
                       [&](double value) {
                           TagWriteFull::write(buffer, NbtTagType::TAG_Double, name);
                           push_swapped(buffer, value);
                       },
                       [&](std::vector<byte> const &arr) {
                           TagWriteFull::write(buffer, NbtTagType::TAG_Byte_Array, name);
                           write_vector(buffer, arr);
                       },
                       [&](std::string const &str) {
                           TagWriteFull::write(buffer, NbtTagType::TAG_String, name);
                           write_len(buffer, str.size());
                           buffer = std::copy(str.begin(), str.end(), buffer);
                       },
                       [&](NbtCompound const &compound) {
                           TagWriteFull::write(buffer, NbtTagType::TAG_Compound, name);
                           write_compound(buffer, compound);
                       },
                       [&](std::vector<int> const &arr) {
                           TagWriteFull::write(buffer, NbtTagType::TAG_Int_Array, name);
                           write_vector(buffer, arr);
                       },
                       [&](std::vector<int64_t> const &arr) {
                           TagWriteFull::write(buffer, NbtTagType::TAG_Long_Array, name);
                           write_vector(buffer, arr);
                       },
                       [&](NbtList const &list) {
                           TagWriteFull::write(buffer, NbtTagType::TAG_List, name);
                           list.to_buffer(buffer);
                       }
                   }, value);
    }

    friend std::ostream &operator<<(std::ostream &os, NbtField const &nbt);

    // ---- Comp -------------------------------------------------------------------------------------------------------
    bool operator==(const NbtField &) const = default;

    // ---- Properties -------------------------------------------------------------------------------------------------
    bool is_end() const {
        return std::holds_alternative<TagEnd>(value);
    }

    bool is_compound() const {
        return std::holds_alternative<NbtCompound>(value);
    }
};

inline std::ostream &operator<<(std::ostream &os, NbtField const &nbt) {
    os << nbt.name << ": ";
    std::visit(
        overloaded{
            [&](TagEnd) { os << "END"; },
            [&](byte b) {
                os << static_cast<int>(b) << "u8";
            },
            [&](int16_t value) {
                os << value << "i16";
            },
            [&](int32_t value) {
                os << value << "i32";
            },
            [&](int64_t value) {
                os << value << "i64";
            },
            [&](float value) {
                os << value << "f32";
            },
            [&](double value) {
                os << value << "f64";
            },
            [&](std::vector<byte> const &arr) {
                os << "[";
                for (byte b: arr) {
                    os << static_cast<int>(b) << ", ";
                }
                os << "] ([]u8)";
            },
            [&](std::string const &str) {
                os << '"' << str << '"';
            },
            [&](NbtList const &list) {
                throw std::runtime_error("not implemented");
            },
            [&](NbtCompound const &compound) {
                os << "{ ";
                for (auto const &field: compound.fields) {
                    os << field << ", ";
                }
                os << "}";
            },
            [&](std::vector<int> const &arr) {
                throw std::runtime_error("not implemented");
            },
            [&](std::vector<int64_t> const &arr) {
                throw std::runtime_error("not implemented");
            },
        }, nbt.value
    );

    return os;
}

template<typename InputIt>
NbtCompound read_compound(InputIt &buffer) {
    NbtCompound::Fields fields;

    while (true) {
        const auto field = NbtField::from_buffer(buffer);
        if (field.is_end()) { break; }
        fields.push_back(field);
    }
    return NbtCompound{fields};
}

template<typename OutputIt>
void write_compound(OutputIt &buffer, NbtCompound const &compound) {
    for (auto const &field: compound.fields) {
        field.to_buffer(buffer);
    }
    *buffer++ = NbtTagType::TAG_END;
}

#endif //FIELD_HPP
