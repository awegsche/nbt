//
// Created by andiw on 30/01/2025.
//

#ifndef NBT_HPP
#define NBT_HPP

#include "field.hpp"
#include "filesystem"
#include <fstream>

inline void write_nbt(NbtField const &field, std::filesystem::path const &filepath) {
    std::ofstream file{filepath.string()};

    auto outit = std::ostreambuf_iterator<char>(file);

    field.to_buffer(outit);
}

inline NbtField read_nbt(std::filesystem::path const &filepath) {
    std::ifstream file{filepath.string()};
    auto buffer = std::istreambuf_iterator<char>(file);
    return NbtField::from_buffer(buffer);
}

#endif //NBT_HPP
