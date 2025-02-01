//
// Created by andiw on 01/02/2025.
//

#ifndef TAGTYPE_HPP
#define TAGTYPE_HPP

#include <cstdint>

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

#endif //TAGTYPE_HPP
