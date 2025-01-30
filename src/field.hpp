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
#include <span>

#include "helpers.hpp"

enum NbtTagType : uint8_t {
    TAG_END = 0,
    TAG_Byte,
    TAG_Short,
    TAG_Int,
    TAG_Long,
    TAG_Float,
    TAG_Double,
    TAG_Byte_Array,
    TAG_String,
    TAG_List,
    TAG_Compound,
    TAG_Int_Array,
    TAG_Long_Array
};

enum class NbtError {
    FieldNotFound,
    NotACompound
};

struct TagEnd {
    bool operator==(const TagEnd &) const { return true; }
};

struct NbtField;

struct NbtCompound {
    using Fields = std::vector<NbtField>;
    using const_iterator = Fields::const_iterator;

    Fields fields;

    bool operator==(const NbtCompound &) const = default;

    const_iterator find(std::string_view const key) const;

    const_iterator find_path(std::span<std::string> const path) const;

    const_iterator cbegin() const { return fields.cbegin(); }
    const_iterator cend() const { return fields.cend(); }

    const_iterator begin() const { return fields.begin(); }
    const_iterator end() const { return fields.end(); }
};


struct NbtList {
    using payload_t = std::variant<TagEnd,
        std::vector<byte>,
        std::vector<int16_t>,
        std::vector<int32_t>,
        std::vector<int64_t>,
        std::vector<float>,
        std::vector<double>,
        std::vector<std::vector<byte> >,
        std::vector<std::string>,
        std::vector<NbtList>,
        std::vector<NbtCompound>,
        std::vector<std::vector<int32_t> >,
        std::vector<std::vector<int64_t> > >;

    template<typename TagWrite, typename OutputIt>
    void to_buffer(OutputIt &buffer) {
        std::visit(overloaded{
                       [&](TagEnd) {
                           *buffer++ = NbtTagType::TAG_END;
                           const int32_t len = 0;
                           push_swapped4(buffer, &len);
                       },

                       [&](std::vector<byte> const &arr) {
                           *buffer++ = NbtTagType::TAG_Byte;
                           const auto len = static_cast<int32_t>(arr.size());
                           push_swapped4(buffer, &len);

                           std::copy(arr.begin(), arr.end(), buffer);
                           buffer += len;
                       },
                       [&](std::vector<int16_t> const &arr) {
                           *buffer++ = NbtTagType::TAG_Short;
                           const auto len = static_cast<int32_t>(arr.size());
                           push_swapped4(buffer, &len);

                           for (auto const i: arr) {
                               push_swapped2(buffer, &i);
                           }
                       },
                       [&](std::vector<int32_t> const &arr) {
                           *buffer++ = NbtTagType::TAG_Int;
                           const auto len = static_cast<int32_t>(arr.size());
                           push_swapped4(buffer, &len);

                           for (auto const i: arr) {
                               push_swapped4(buffer, &i);
                           }
                       },
                       [&](std::vector<int64_t> const &arr) {
                           *buffer++ = NbtTagType::TAG_Long;
                           const auto len = static_cast<int32_t>(arr.size());
                           push_swapped4(buffer, &len);

                           for (auto const i: arr) {
                               push_swapped8(buffer, &i);
                           }
                       },
                       [&](std::vector<float> const &arr) {
                           *buffer++ = NbtTagType::TAG_Float;
                           const auto len = static_cast<int32_t>(arr.size());
                           push_swapped4(buffer, &len);

                           for (auto const i: arr) {
                               push_swapped4(buffer, &i);
                           }
                       },
                       [&](std::vector<double> const &arr) {
                           *buffer++ = NbtTagType::TAG_Double;
                           const auto len = static_cast<int32_t>(arr.size());
                           push_swapped4(buffer, &len);

                           for (auto const i: arr) {
                               push_swapped8(buffer, &i);
                           }
                       },
                   }, payload);
    }

    bool operator==(const NbtList &) const = default;

    payload_t payload;
};

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

    // ---- Access -----------------------------------------------------------------------------------------------------
    NbtField const &find(std::string_view const key) const {
        auto const &compound = std::get<NbtCompound>(value);

        const auto found = compound.find(key);
        if (found == compound.end()) {
        }
        return *found;
    }

    NbtCompound::const_iterator find_path(std::span<std::string> const path) const {
        auto const &compound = std::get<NbtCompound>(value);

        return compound.find_path(path);
    }

    // ---- IO ---------------------------------------------------------------------------------------------------------

    template<typename InputIt>
    static NbtField from_buffer(InputIt &buffer) {
        return {};
    }

    template<typename TagWrite, typename OutputIt>
    void to_buffer(OutputIt &buffer) const {
        std::visit(overloaded{
                       [&](TagEnd) { *buffer++ = NbtTagType::TAG_END; },
                       [&](byte b) {
                           TagWrite::write(buffer, NbtTagType::TAG_Byte, name);
                           *buffer++ = b;
                       },
                       [&](int16_t b) {
                           TagWrite::write(buffer, NbtTagType::TAG_Short, name);
                           push_swapped2(buffer, &b);
                       },
                       [&](int32_t b) {
                           TagWrite::write(buffer, NbtTagType::TAG_Int, name);
                           push_swapped4(buffer, &b);
                       },
                       [&](int64_t b) {
                           TagWrite::write(buffer, NbtTagType::TAG_Long, name);
                           push_swapped8(buffer, &b);
                       },
                       [&](float f) {
                           TagWrite::write(buffer, NbtTagType::TAG_Float, name);
                           push_swapped4(buffer, &f);
                       },
                       [&](double d) {
                           TagWrite::write(buffer, NbtTagType::TAG_Double, name);
                           push_swapped4(buffer, &d);
                       },
                       [&](std::vector<byte> const &arr) {
                           TagWrite::write(buffer, NbtTagType::TAG_Byte_Array, name);
                           const auto len = static_cast<int32_t>(arr.size());
                           push_swapped2(buffer, &len);
                           std::copy(arr.begin(), arr.end(), buffer);
                           buffer += len;
                       },
                       [&](std::string const &str) {
                           TagWrite::write(buffer, NbtTagType::TAG_String, name);
                           const auto len = static_cast<int32_t>(str.size());
                           push_swapped2(buffer, &len);
                           std::copy(str.begin(), str.end(), buffer);
                           buffer += len;
                       },
                       [&](NbtList const &list) {
                       },
                       [&](NbtCompound const &compound) {
                           TagWrite::write(buffer, NbtTagType::TAG_Compound, name);
                           for (auto const &field: compound) {
                               field.to_buffer(buffer);
                           }
                           *buffer++ = NbtTagType::TAG_END;
                       },
                       [&](std::vector<int> const &arr) {
                           throw std::runtime_error("not implemented");
                       },
                       [&](std::vector<int64_t> const &arr) {
                           throw std::runtime_error("not implemented");
                       },
                   }, value);
    }

    friend std::ostream &operator<<(std::ostream &os, NbtField const &nbt);

    // ---- Comp -------------------------------------------------------------------------------------------------------
    bool operator==(const NbtField &) const = default;
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

inline NbtCompound::const_iterator NbtCompound::find(std::string_view const key) const {
    return std::find_if(begin(), end(), [&key](NbtField const &field) {
        return field.name == key;
    });
}

inline NbtCompound::const_iterator NbtCompound::find_path(std::span<std::string> const path) const {
    const auto found = find(path[0]);

    if (found == end()) throw NbtError::FieldNotFound;
    if (path.size() == 1) return found;

    return found->find_path(path.subspan(1, path.size() - 1));
}
#endif //FIELD_HPP
