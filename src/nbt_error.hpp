//
// Created by andiw on 30/01/2025.
//

#ifndef NBT_ERROR_HPP
#define NBT_ERROR_HPP
#include <cstdint>
#include <format>
#include <string>

class NbtFieldNotFoundError : public std::exception {
public:
    explicit NbtFieldNotFoundError(std::string_view field_name)
        : m_error_msg(std::format("Field {} not found", field_name)) {
    }

    [[nodiscard]] const char *what() const noexcept override {
        return m_error_msg.c_str();
    }

private:
    std::string m_error_msg;
};

class NbtInvalidTagError : public std::exception {
public:
    explicit NbtInvalidTagError(uint8_t tag)
        : m_error_msg(std::format("Invalid NBT Tag {}", static_cast<int>(tag))) {
    }

    [[nodiscard]] const char *what() const noexcept override {
        return m_error_msg.c_str();
    }

private:
    std::string m_error_msg;
};

#endif //NBT_ERROR_HPP
