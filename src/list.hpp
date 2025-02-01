//
// Created by andiw on 01/02/2025.
//

#ifndef LIST_HPP
#define LIST_HPP

#include <variant>
#include <vector>
#include "nbt_error.hpp"
#include "compound.hpp"
#include "end.hpp"
#include "tagtype.hpp"
#include "helpers.hpp"

struct NbtField;

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

    template<typename OutputIt>
    void to_buffer(OutputIt &buffer) const {
        std::visit(overloaded{
                       [&](TagEnd) {
                           *buffer++ = NbtTagType::TAG_END;
                           const int32_t len = 0;
                           push_swapped(buffer, len);
                       },

                       [&](std::vector<byte> const &arr) {
                           *buffer++ = NbtTagType::TAG_Byte;
                           write_vector(buffer, arr);
                       },
                       [&](std::vector<int16_t> const &arr) {
                           *buffer++ = NbtTagType::TAG_Short;
                           write_vector(buffer, arr);
                       },
                       [&](std::vector<int32_t> const &arr) {
                           *buffer++ = NbtTagType::TAG_Int;
                           write_vector(buffer, arr);
                       },
                       [&](std::vector<int64_t> const &arr) {
                           *buffer++ = NbtTagType::TAG_Long;
                           write_vector(buffer, arr);
                       },
                       [&](std::vector<float> const &arr) {
                           *buffer++ = NbtTagType::TAG_Float;
                           write_vector(buffer, arr);
                       },
                       [&](std::vector<double> const &arr) {
                           *buffer++ = NbtTagType::TAG_Double;
                           write_vector(buffer, arr);
                       },
                       [&](std::vector<NbtCompound> const &arr) {
                           *buffer++ = NbtTagType::TAG_Compound;
                           push_swapped(buffer, static_cast<int32_t>(arr.size()));
                           for (auto const &compound: arr) {
                               write_compound(buffer, compound);
                           }
                       },
                       [&](std::vector<std::string> const &arr) {
                           throw std::runtime_error{"not implemented"};
                       },
                       [&](std::vector<NbtList> const &arr) {
                           *buffer++ = TAG_List;
                           push_swapped(buffer, static_cast<int32_t>(arr.size()));
                           for (auto const &list: arr) {
                               list.to_buffer(buffer);
                           }
                       },
                       [&](std::vector<std::vector<int32_t> > const &arr) {
                           throw std::runtime_error{"not implemented"};
                       },
                       [&](std::vector<std::vector<int64_t> > const &arr) {
                           throw std::runtime_error{"not implemented"};
                       },
                       [&](std::vector<std::vector<byte> > const &arr) {
                           throw std::runtime_error{"not implemented"};
                       },
                   }, payload);
    }

    template<typename InputIt>
    static NbtList from_buffer(InputIt &buffer) {
        const auto tag = static_cast<NbtTagType>(*buffer++);
        switch (tag) {
            case TAG_END:
                return NbtList{TagEnd{}};
            case TAG_Byte:
                return NbtList{read_vector<byte>(buffer)};
            case TAG_Short:
                return NbtList{read_vector<int16_t>(buffer)};
            case TAG_Int:
                return NbtList{read_vector<int32_t>(buffer)};
            case TAG_Long:
                return NbtList{read_vector<int64_t>(buffer)};
            case TAG_Float:
                return NbtList{read_vector<float>(buffer)};
            case TAG_Double:
                return NbtList{read_vector<double>(buffer)};
            case TAG_List: {
                const auto len = pull_swapped<int32_t>(buffer);
                std::vector<NbtList> lists{};
                lists.reserve(len);
                for (int32_t i = 0; i < len; ++i) {
                    lists.push_back(NbtList::from_buffer(buffer));
                }
                return NbtList{lists};
            }
            default:
                throw NbtInvalidTagError{tag};
        }
    }

    bool operator==(const NbtList &) const = default;

    payload_t payload;
};
#endif //LIST_HPP
