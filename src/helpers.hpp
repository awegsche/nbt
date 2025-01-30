//
// Created by andiw on 30/01/2025.
//

#ifndef HELPERS_HPP
#define HELPERS_HPP
#include <cstdint>

using byte = unsigned char;

template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };

template<typename T, typename OutputIt> void push_swapped2(OutputIt& buffer, const T *value)
{
    static_assert(sizeof(T) == 2);
    *buffer++ = *(reinterpret_cast<const unsigned char *>(value) + 1);
    *buffer++ = *reinterpret_cast<const unsigned char *>(value);
}

template<typename T, typename OutputIt> void push_swapped4(OutputIt &buffer, const T *value)
{
    static_assert(sizeof(T) == 4);
    *buffer++ = *(reinterpret_cast<const unsigned char *>(value) + 3);
    *buffer++ = *(reinterpret_cast<const unsigned char *>(value) + 2);
    *buffer++ = *(reinterpret_cast<const unsigned char *>(value) + 1);
    *buffer++ = *reinterpret_cast<const unsigned char *>(value);
}

template<typename T, typename OutputIt> void push_swapped8(OutputIt &buffer, const T *value)
{
    static_assert(sizeof(T) == 8);
    *buffer++ = *(reinterpret_cast<const unsigned char *>(value) + 7);
    *buffer++ = *(reinterpret_cast<const unsigned char *>(value) + 6);
    *buffer++ = *(reinterpret_cast<const unsigned char *>(value) + 5);
    *buffer++ = *(reinterpret_cast<const unsigned char *>(value) + 4);
    *buffer++ = *(reinterpret_cast<const unsigned char *>(value) + 3);
    *buffer++ = *(reinterpret_cast<const unsigned char *>(value) + 2);
    *buffer++ = *(reinterpret_cast<const unsigned char *>(value) + 1);
    *buffer++ = *reinterpret_cast<const unsigned char *>(value);
}

template<typename T, typename OutputIt> void push_unswapped2(OutputIt& buffer, const T *value)
{
    static_assert(sizeof(T) == 2);
    *buffer++ = *(reinterpret_cast<const unsigned char *>(value));
    *buffer++ = *(reinterpret_cast<const unsigned char *>(value)+ 1);
}

template<typename T, typename OutputIt> void push_unswapped4(OutputIt &buffer, const T *value)
{
    static_assert(sizeof(T) == 4);
    *buffer++ = *reinterpret_cast<const unsigned char *>(value);
    *buffer++ = *(reinterpret_cast<const unsigned char *>(value) + 1);
    *buffer++ = *(reinterpret_cast<const unsigned char *>(value) + 2);
    *buffer++ = *(reinterpret_cast<const unsigned char *>(value) + 3);
}

template<typename T, typename OutputIt> void push_unswapped8(OutputIt &buffer, const T *value)
{
    static_assert(sizeof(T) == 8);
    *buffer++ = *reinterpret_cast<const unsigned char *>(value);
    *buffer++ = *(reinterpret_cast<const unsigned char *>(value) + 1);
    *buffer++ = *(reinterpret_cast<const unsigned char *>(value) + 2);
    *buffer++ = *(reinterpret_cast<const unsigned char *>(value) + 3);
    *buffer++ = *(reinterpret_cast<const unsigned char *>(value) + 4);
    *buffer++ = *(reinterpret_cast<const unsigned char *>(value) + 5);
    *buffer++ = *(reinterpret_cast<const unsigned char *>(value) + 6);
    *buffer++ = *(reinterpret_cast<const unsigned char *>(value) + 7);
}

struct TagWriteFull {
    template<typename OutputIt>
    static void write(OutputIt& buffer, byte tag, std::string const& name) {
        *buffer++ = tag;
        const auto len = static_cast<int16_t>(name.size());
        push_swapped2(buffer, &len);

        std::copy(name.begin(), name.end(), buffer);

        buffer += len;
    }
};

#endif //HELPERS_HPP
