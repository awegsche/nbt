//
// Created by andiw on 30/01/2025.
//

#include "compound.hpp"
#include <algorithm>

#include "nbt_error.hpp"
#include "field.hpp"

NbtField const &NbtCompound::find(std::string_view const key) const {
    auto const found = std::find_if(cbegin(), cend(), [&key](NbtField const &field) {
        return field.name == key;
    });

    if (found == cend()) {
        throw NbtFieldNotFoundError(key);
    }

    return fields[found - cbegin()];
}

NbtField const &NbtCompound::find_path(std::span<std::string> const path) const {
    const auto &found = find(path[0]);
    if (path.size() == 1) { return found; }
    return found.find_path(path.subspan(1, path.size() - 1));
}
