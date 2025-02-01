//
// Created by andiw on 30/01/2025.
//

#ifndef HELPERS_HPP
#define HELPERS_HPP
#include <cstdint>
#include <array>

using byte = unsigned char;

template<class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};

template<typename T, typename InputIt>
T pull_swapped(InputIt &buffer) {
    std::array<byte, sizeof(T)> data;
    for (size_t pos = 0;  pos < sizeof(T); ++pos) {
        data[sizeof(T) - pos - 1] = *buffer++;
    }

    return *reinterpret_cast<T const*>(data.data());
}


template<typename T, typename OutputIt>
void push_swapped(OutputIt &buffer, const T &value) {
    const auto* data = reinterpret_cast<const unsigned char *>(&value);

    for (size_t pos = 0;  pos < sizeof(T); ++pos) {
        *buffer++ = *(data + sizeof(T) - pos - 1);
    }

}

struct TagWriteFull {
    template<typename OutputIt>
    static void write(OutputIt &buffer, byte tag, std::string const &name) {
        *buffer++ = tag;
        const auto len = static_cast<uint16_t>(name.size());
        push_swapped(buffer, len);

        buffer = std::copy(name.begin(), name.end(), buffer);
    }
};

template<typename InputIt>
std::string read_name(InputIt &buffer) {
    auto const len = pull_swapped<uint16_t>(buffer);

    std::string str;
    str.reserve(len);

    for (size_t i = 0; i < len; ++i) {
        str.push_back(*buffer++);
    }
    return str;
}

template<typename InputIt>
std::string read_string(InputIt &buffer) {
    const auto len = pull_swapped<int32_t>(buffer);

    std::string str{};
    str.reserve(len);

    for (int32_t i = 0; i < len; ++i) {
        str.push_back(*buffer++);
    }
    return str;
}

template<typename OutputIt>
void write_len(OutputIt &buffer, int32_t len) {
    push_swapped(buffer, len);
}

template<typename InputIt>
int32_t read_len(InputIt &buffer) {
    return pull_swapped<int32_t>(buffer);
}

template<typename T, typename InputIt>
std::vector<T> read_vector(InputIt &buffer) {
    const auto len = pull_swapped<int32_t>(buffer);
    std::vector<T> arr{};
    arr.reserve(len);

    for (int32_t i = 0; i < len; ++i) {
        arr.push_back(pull_swapped<T>(buffer));
    }

    return arr;
}

template<typename T, typename OutputIt>
void write_vector(OutputIt &buffer, std::vector<T> const &arr) {

    const auto len = static_cast<int32_t>(arr.size());
    push_swapped(buffer, len);

    for (auto const &i: arr) {
        push_swapped(buffer, i);
    }
}
#endif //HELPERS_HPP
