//
// Created by andiw on 30/01/2025.
//

#ifndef COMPOUND_HPP
#define COMPOUND_HPP

#include <vector>
#include <string>
#include <string_view>
#include <span>

struct NbtField;

struct NbtCompound {
    using Fields = std::vector<NbtField>;
    using const_iterator = Fields::const_iterator;

    Fields fields;

    bool operator==(const NbtCompound &) const = default;

    NbtField const &find(std::string_view const key) const;

    NbtField const &find_path(std::span<std::string> const path) const;

    const_iterator cbegin() const { return fields.cbegin(); }
    const_iterator cend() const { return fields.cend(); }

    const_iterator begin() const { return fields.begin(); }
    const_iterator end() const { return fields.end(); }
};

// forward-declare read_compound function must be defined in field.cpp
template<typename InputIt>
NbtCompound read_compound(InputIt &buffer);

// forward-declare write_compound function must be defined in field.cpp
template<typename OutputIt>
void write_compound(OutputIt &buffer, NbtCompound const& compound);


#endif //COMPOUND_HPP
