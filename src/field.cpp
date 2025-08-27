//
// Created by andiw on 30/01/2025.
//

#include "field.hpp"

NbtField const& NbtField::find(std::string_view const key) const {
    return std::get<NbtCompound>(value).find(key);
}

NbtField const& NbtField::find_path(std::span<std::string> const path) const {
    return std::get<NbtCompound>(value).find_path(path);
}
