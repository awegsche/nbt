#include "../include/nbt.h"
#include "common.h"
#include <cstdint>
#include <fstream>
#include <iostream>
#include <numeric>
#include <sstream>
#include <utility>
#include <vector>
#include <zconf.h>
#include <zlib.h>

using std::get;
using std::string;

constexpr uint16_t MAXLEVEL = 10;
constexpr size_t MAX_ARRAY_PRINT = 8;
constexpr size_t PRINT_THRESHOLD = MAX_ARRAY_PRINT + 2;
;

namespace nbt {

std::string nbt_node::pretty_print(uint16_t level) const
{
    using enum NbtTagType;

    if (level > MAXLEVEL) return "";

    std::stringstream ss;
    plevel plev{ level };

    ss << plev;

    if (!name.empty()) ss << "\33[1m" << name << "\o{33}[0m: ";

    switch (tagtype()) {
    case TAG_END:
        ss << "END";
        break;
    case TAG_Short:
        ss << "short " << get<TAG_Short>();
        break;
    case TAG_Compound:
        ss << "Compound {";
        for (const nbt_node &n : get<TAG_Compound>().content) { ss << "\n" << n.pretty_print(level + 1); }
        ss << "\n" << plev;

        ss << "}";
        break;
    case TAG_Float:
        ss << "float " << get<TAG_Float>();
        break;
    case TAG_Double:
        ss << "double " << get<TAG_Double>();
        break;
    case TAG_Byte:
        ss << "byte " << static_cast<int>(get<TAG_Byte>());
        break;
    case TAG_Int:
        ss << "int " << get<TAG_Int>();
        break;
    case TAG_Long:
        ss << "long " << get<TAG_Long>();
        break;
    case TAG_List: {
        auto const& list = get<TAG_List>();
        auto elem_type = list.content_type();
        ss << "List<" << std::to_underlying(elem_type) << "> [";
        
        switch (elem_type) {
        case TAG_END:
            ss << "empty";
            break;
        case TAG_Byte: {
            auto const& vec = list.get<TAG_Byte>();
            ss << vec.size() << " bytes";
            break;
        }
        case TAG_Short: {
            auto const& vec = list.get<TAG_Short>();
            if (vec.size() <= PRINT_THRESHOLD) {
                for (size_t i = 0; i < vec.size(); i++) {
                    if (i > 0) ss << ", ";
                    ss << vec[i];
                }
            } else {
                for (size_t i = 0; i < MAX_ARRAY_PRINT; i++) ss << vec[i] << ", ";
                ss << "...";
            }
            break;
        }
        case TAG_Int: {
            auto const& vec = list.get<TAG_Int>();
            if (vec.size() <= PRINT_THRESHOLD) {
                for (size_t i = 0; i < vec.size(); i++) {
                    if (i > 0) ss << ", ";
                    ss << vec[i];
                }
            } else {
                for (size_t i = 0; i < MAX_ARRAY_PRINT; i++) ss << vec[i] << ", ";
                ss << "...";
            }
            break;
        }
        case TAG_Long: {
            auto const& vec = list.get<TAG_Long>();
            if (vec.size() <= PRINT_THRESHOLD) {
                for (size_t i = 0; i < vec.size(); i++) {
                    if (i > 0) ss << ", ";
                    ss << vec[i];
                }
            } else {
                for (size_t i = 0; i < MAX_ARRAY_PRINT; i++) ss << vec[i] << ", ";
                ss << "...";
            }
            break;
        }
        case TAG_Float: {
            auto const& vec = list.get<TAG_Float>();
            if (vec.size() <= PRINT_THRESHOLD) {
                for (size_t i = 0; i < vec.size(); i++) {
                    if (i > 0) ss << ", ";
                    ss << vec[i];
                }
            } else {
                for (size_t i = 0; i < MAX_ARRAY_PRINT; i++) ss << vec[i] << ", ";
                ss << "...";
            }
            break;
        }
        case TAG_Double: {
            auto const& vec = list.get<TAG_Double>();
            if (vec.size() <= PRINT_THRESHOLD) {
                for (size_t i = 0; i < vec.size(); i++) {
                    if (i > 0) ss << ", ";
                    ss << vec[i];
                }
            } else {
                for (size_t i = 0; i < MAX_ARRAY_PRINT; i++) ss << vec[i] << ", ";
                ss << "...";
            }
            break;
        }
        case TAG_String: {
            auto const& vec = list.get<TAG_String>();
            ss << vec.size() << " strings";
            break;
        }
        case TAG_Compound: {
            auto const& vec = list.get<TAG_Compound>();
            ss << "\n";
            for (size_t i = 0; i < vec.size() && i < MAX_ARRAY_PRINT; i++) {
                ss << plevel{static_cast<uint16_t>(level + 1)} << "Compound {";
                for (auto const& child : vec[i].content) {
                    ss << "\n" << child.pretty_print(level + 2);
                }
                ss << "\n" << plevel{static_cast<uint16_t>(level + 1)} << "}";
                if (i + 1 < vec.size()) ss << ",\n";
            }
            if (vec.size() > MAX_ARRAY_PRINT) {
                ss << "\n" << plevel{static_cast<uint16_t>(level + 1)} << "... (" << vec.size() - MAX_ARRAY_PRINT << " more)";
            }
            ss << "\n" << plev;
            break;
        }
        default:
            ss << list.content.index() << " elements";
            break;
        }
        ss << "]";
        break;
    }
    case TAG_String:
        ss << "string \"" << get<TAG_String>() << "\"";
        break;
    case TAG_Byte_Array: {
        ss << "byte array {";
        if (const auto &array = get<TAG_Byte_Array>(); array.size() > PRINT_THRESHOLD) {
            for (size_t i = 0; i < MAX_ARRAY_PRINT; i++) { ss << static_cast<int>(array[i]) << ", "; }
            ss << " ...";
        } else {
            for (int b : array) { ss << b << ", "; }
        }
        ss << "}";
        break;
    }
    case TAG_Int_Array: {
        ss << "int array {";
        if (const auto &array = get<TAG_Int_Array>(); array.size() > PRINT_THRESHOLD) {
            for (size_t i = 0; i < MAX_ARRAY_PRINT; i++) { ss << array[i] << ", "; }
            ss << " ...";
        } else {
            for (int b : array) { ss << b << ", "; }
        }
        ss << "}";
        break;
    }
    default:
        ss << "TAG " << std::to_underlying(tagtype()) << " not implemented";
    }
    return ss.str();
}

/// read an nbt_node from the given buffer
nbt_node read_node(const char *&buffer)
{
    auto id = *reinterpret_cast<const NbtTagType *>(buffer++);
    if (id == NbtTagType::TAG_END) { return nbt_node{}; }

    nbt_node node{};
    node.name = get_name(buffer);

    get_payload(id, buffer, &node);

    return node;
}

std::string get_name(const char *&buffer)
{
    int16_t length = __swap2(buffer);
    buffer += 2;

    std::string name;
    name.resize(length);
    memcpy(name.data(), buffer, length);
    buffer += length;

    return name;
}

static void write_name(std::string const &name, std::vector<unsigned char> &buffer)
{

    auto length = static_cast<int16_t>(name.size());
    push_swapped2(buffer, &length);

    for (char c : name) buffer.push_back(c);
}

static void write_payload(const nbt_node &node, std::vector<unsigned char> &buffer)
{
    using enum NbtTagType;

    switch (node.tagtype()) {
    case TAG_Byte: {
        buffer.push_back(std::get<byte>(node.payload));
        break;
    }
    case TAG_Short: {
        push_swapped2(buffer, &std::get<int16_t>(node.payload));
        break;
    }
    case TAG_Int: {
        push_swapped4(buffer, &std::get<int32_t>(node.payload));
        break;
    }
    case TAG_Long: {
        push_swapped8(buffer, &std::get<int64_t>(node.payload));
        break;
    }
    case TAG_Float: {
        push_swapped4(buffer, &std::get<float>(node.payload));
        break;
    }
    case TAG_Double: {
        push_swapped8(buffer, &std::get<double>(node.payload));
        break;
    }
    case TAG_Byte_Array: {
        auto const &payload = std::get<std::vector<byte>>(node.payload);
        auto len = static_cast<int32_t>(payload.size());
        push_swapped4(buffer, &len);
        for (byte b : payload) { buffer.push_back(b); }
        break;
    }
    case TAG_Int_Array: {
        auto const &payload = node.get<TAG_Int_Array>();
        auto len = static_cast<int32_t>(payload.size());
        push_swapped4(buffer, &len);
        for (int32_t b : payload) { push_swapped4(buffer, &b); }
        break;
    }
    case TAG_Long_Array: {
        auto const &payload = node.get<TAG_Long_Array>();
        auto len = static_cast<int32_t>(payload.size());
        push_swapped4(buffer, &len);
        for (int64_t b : payload) { push_swapped8(buffer, &b); }
        break;
    }
    case TAG_List: {
        auto const& list = node.get<TAG_List>();
        auto element_type = list.content_type();
        buffer.push_back(static_cast<unsigned char>(element_type));

        switch (element_type) {
        case TAG_END: {
            // Empty list - write length 0
            push_swapped4(buffer, &(const int32_t&)0);
            break;
        }
        case TAG_Byte: {
            auto const& vec = list.get<TAG_Byte>();
            auto len = static_cast<int32_t>(vec.size());
            push_swapped4(buffer, &len);
            for (byte b : vec) buffer.push_back(b);
            break;
        }
        case TAG_Short: {
            auto const& vec = list.get<TAG_Short>();
            auto len = static_cast<int32_t>(vec.size());
            push_swapped4(buffer, &len);
            for (int16_t v : vec) push_swapped2(buffer, &v);
            break;
        }
        case TAG_Int: {
            auto const& vec = list.get<TAG_Int>();
            auto len = static_cast<int32_t>(vec.size());
            push_swapped4(buffer, &len);
            for (int32_t v : vec) push_swapped4(buffer, &v);
            break;
        }
        case TAG_Long: {
            auto const& vec = list.get<TAG_Long>();
            auto len = static_cast<int32_t>(vec.size());
            push_swapped4(buffer, &len);
            for (int64_t v : vec) push_swapped8(buffer, &v);
            break;
        }
        case TAG_Float: {
            auto const& vec = list.get<TAG_Float>();
            auto len = static_cast<int32_t>(vec.size());
            push_swapped4(buffer, &len);
            for (float v : vec) push_swapped4(buffer, &v);
            break;
        }
        case TAG_Double: {
            auto const& vec = list.get<TAG_Double>();
            auto len = static_cast<int32_t>(vec.size());
            push_swapped4(buffer, &len);
            for (double v : vec) push_swapped8(buffer, &v);
            break;
        }
        case TAG_Byte_Array: {
            auto const& vec = list.get<TAG_Byte_Array>();
            auto len = static_cast<int32_t>(vec.size());
            push_swapped4(buffer, &len);
            for (auto const& arr : vec) {
                auto arr_len = static_cast<int32_t>(arr.size());
                push_swapped4(buffer, &arr_len);
                for (byte b : arr) buffer.push_back(b);
            }
            break;
        }
        case TAG_String: {
            auto const& vec = list.get<TAG_String>();
            auto len = static_cast<int32_t>(vec.size());
            push_swapped4(buffer, &len);
            for (auto const& str : vec) {
                auto str_len = static_cast<int16_t>(str.size());
                push_swapped2(buffer, &str_len);
                for (char c : str) buffer.push_back(c);
            }
            break;
        }
        case TAG_List: {
            auto const& vec = list.get<TAG_List>();
            auto len = static_cast<int32_t>(vec.size());
            push_swapped4(buffer, &len);
            for (auto const& inner_list : vec) {
                nbt_node temp_node;
                temp_node.payload = inner_list;
                write_payload(temp_node, buffer);
            }
            break;
        }
        case TAG_Compound: {
            auto const& vec = list.get<TAG_Compound>();
            auto len = static_cast<int32_t>(vec.size());
            push_swapped4(buffer, &len);
            for (auto const& comp : vec) {
                for (auto const& child : comp.content) {
                    write_node(child, buffer);
                }
                buffer.push_back(0);  // TAG_End
            }
            break;
        }
        case TAG_Int_Array: {
            auto const& vec = list.get<TAG_Int_Array>();
            auto len = static_cast<int32_t>(vec.size());
            push_swapped4(buffer, &len);
            for (auto const& arr : vec) {
                auto arr_len = static_cast<int32_t>(arr.size());
                push_swapped4(buffer, &arr_len);
                for (int32_t v : arr) push_swapped4(buffer, &v);
            }
            break;
        }
        case TAG_Long_Array: {
            auto const& vec = list.get<TAG_Long_Array>();
            auto len = static_cast<int32_t>(vec.size());
            push_swapped4(buffer, &len);
            for (auto const& arr : vec) {
                auto arr_len = static_cast<int32_t>(arr.size());
                push_swapped4(buffer, &arr_len);
                for (int64_t v : arr) push_swapped8(buffer, &v);
            }
            break;
        }
        }
        break;
    }
    case TAG_Compound: {
        auto const &payload = node.get<TAG_Compound>().content;
        for (auto const &child : payload) { write_node(child, buffer); }
        write_node(nbt_node{}, buffer);
        break;
    }
    case TAG_String: {
        auto const &str = node.get<TAG_String>();
        auto len = static_cast<int16_t>(str.size());

        push_swapped2(buffer, &len);
        for (char c : str) { buffer.push_back(c); }

        break;
    }
    case TAG_END:
        break;
    }
}

void get_payload(const NbtTagType id, const char *&buffer, nbt_node *node)
{
    using enum nbt::NbtTagType;

    switch (id) {
    case TAG_Byte: {
        node->payload = static_cast<byte>(*(buffer++));
        break;
    }
    case TAG_Short: {
        node->payload = static_cast<int16_t>(__swap2(buffer));
        buffer += 2;
        break;
    }
    case TAG_Int: {
        node->payload = static_cast<int32_t>(__swap4(buffer));
        buffer += 4;
        break;
    }
    case TAG_Long: {
        node->payload = static_cast<int64_t>(__swap8(buffer));
        buffer += 8;
        break;
    }
    case TAG_Float: {
        node->payload = std::bit_cast<float>(__swap4(buffer));
        buffer += 4;
        break;
    }
    case TAG_Double: {
        node->payload = std::bit_cast<double>(__swap8(buffer));
        buffer += 8;
        break;
    }
    case TAG_Byte_Array: {
        int len = __swap4(buffer);
        buffer += 4;
        node->payload = std::vector<byte>();
        auto &payload = node->get<TAG_Byte_Array>();
        payload.reserve(len);

        for (int i = 0; i < len; i++) { payload.push_back(*(buffer++)); }
        break;
    }
    case TAG_List: {
        auto element_type = static_cast<NbtTagType>(*reinterpret_cast<const uint8_t*>(buffer++));
        int32_t length = __swap4(buffer);
        buffer += 4;

        node->payload = nbt_list{};
        auto& list = node->get<TAG_List>();

        switch (element_type) {
        case TAG_END:
            list.content = TagEnd{};
            break;
        case TAG_Byte: {
            auto& vec = list.content.emplace<std::vector<byte>>();
            vec.reserve(length);
            for (int32_t i = 0; i < length; i++) {
                vec.push_back(static_cast<byte>(*buffer++));
            }
            break;
        }
        case TAG_Short: {
            auto& vec = list.content.emplace<std::vector<int16_t>>();
            vec.reserve(length);
            for (int32_t i = 0; i < length; i++) {
                vec.push_back(static_cast<int16_t>(__swap2(buffer)));
                buffer += 2;
            }
            break;
        }
        case TAG_Int: {
            auto& vec = list.content.emplace<std::vector<int32_t>>();
            vec.reserve(length);
            for (int32_t i = 0; i < length; i++) {
                vec.push_back(static_cast<int32_t>(__swap4(buffer)));
                buffer += 4;
            }
            break;
        }
        case TAG_Long: {
            auto& vec = list.content.emplace<std::vector<int64_t>>();
            vec.reserve(length);
            for (int32_t i = 0; i < length; i++) {
                vec.push_back(static_cast<int64_t>(__swap8(buffer)));
                buffer += 8;
            }
            break;
        }
        case TAG_Float: {
            auto& vec = list.content.emplace<std::vector<float>>();
            vec.reserve(length);
            for (int32_t i = 0; i < length; i++) {
                vec.push_back(std::bit_cast<float>(__swap4(buffer)));
                buffer += 4;
            }
            break;
        }
        case TAG_Double: {
            auto& vec = list.content.emplace<std::vector<double>>();
            vec.reserve(length);
            for (int32_t i = 0; i < length; i++) {
                vec.push_back(std::bit_cast<double>(__swap8(buffer)));
                buffer += 8;
            }
            break;
        }
        case TAG_Byte_Array: {
            auto& vec = list.content.emplace<std::vector<std::vector<byte>>>();
            vec.reserve(length);
            for (int32_t i = 0; i < length; i++) {
                int32_t arr_len = __swap4(buffer);
                buffer += 4;
                std::vector<byte> arr;
                arr.reserve(arr_len);
                for (int32_t j = 0; j < arr_len; j++) {
                    arr.push_back(static_cast<byte>(*buffer++));
                }
                vec.push_back(std::move(arr));
            }
            break;
        }
        case TAG_String: {
            auto& vec = list.content.emplace<std::vector<std::string>>();
            vec.reserve(length);
            for (int32_t i = 0; i < length; i++) {
                auto str_len = static_cast<size_t>(__swap2(buffer));
                buffer += 2;
                vec.emplace_back(buffer, str_len);
                buffer += str_len;
            }
            break;
        }
        case TAG_List: {
            auto& vec = list.content.emplace<std::vector<nbt_list>>();
            vec.reserve(length);
            for (int32_t i = 0; i < length; i++) {
                nbt_node temp_node;
                get_payload(TAG_List, buffer, &temp_node);
                vec.push_back(std::move(temp_node.get<TAG_List>()));
            }
            break;
        }
        case TAG_Compound: {
            auto& vec = list.content.emplace<std::vector<compound>>();
            vec.reserve(length);
            for (int32_t i = 0; i < length; i++) {
                compound comp{};
                while (true) {
                    auto child = read_node(buffer);
                    if (child.tagtype() == TAG_END) break;
                    comp.content.push_back(std::move(child));
                }
                vec.push_back(std::move(comp));
            }
            break;
        }
        case TAG_Int_Array: {
            auto& vec = list.content.emplace<std::vector<std::vector<int32_t>>>();
            vec.reserve(length);
            for (int32_t i = 0; i < length; i++) {
                int32_t arr_len = __swap4(buffer);
                buffer += 4;
                std::vector<int32_t> arr;
                arr.reserve(arr_len);
                for (int32_t j = 0; j < arr_len; j++) {
                    arr.push_back(static_cast<int32_t>(__swap4(buffer)));
                    buffer += 4;
                }
                vec.push_back(std::move(arr));
            }
            break;
        }
        case TAG_Long_Array: {
            auto& vec = list.content.emplace<std::vector<std::vector<int64_t>>>();
            vec.reserve(length);
            for (int32_t i = 0; i < length; i++) {
                int32_t arr_len = __swap4(buffer);
                buffer += 4;
                std::vector<int64_t> arr;
                arr.reserve(arr_len);
                for (int32_t j = 0; j < arr_len; j++) {
                    arr.push_back(static_cast<int64_t>(__swap8(buffer)));
                    buffer += 8;
                }
                vec.push_back(std::move(arr));
            }
            break;
        }
        }
        break;
    }
    case TAG_Compound: {
        node->payload = compound{};
        auto &content = node->get<TAG_Compound>().content;
        while (true) {
            content.push_back(read_node(buffer));
            if (content.back().tagtype() == TAG_END) {
                content.pop_back();// remove the last TagEnd, this doesn't belong into the loaded compound
                break;
            }
        }
        break;
    }
    case TAG_Int_Array: {
        int len = __swap4(buffer);
        buffer += 4;
        node->payload = std::vector<int>();
        node->get<TAG_Int_Array>().reserve(len);

        for (int i = 0; i < len; i++) {
            auto value = static_cast<int>(__swap4(buffer));
            node->get<TAG_Int_Array>().push_back(value);
            buffer += 4;
        }
        break;
    }
    case TAG_Long_Array: {
        int len = __swap4(buffer);
        buffer += 4;
        node->payload = std::vector<int64_t>();
        node->get<TAG_Long_Array>().reserve(len);

        for (int i = 0; i < len; i++) {
            auto value = static_cast<int64_t>(__swap8(buffer));
            node->get<TAG_Long_Array>().push_back(value);
            buffer += 8;
        }
        break;
    }
    case TAG_String: {
        auto len = static_cast<size_t>(__swap2(buffer));
        buffer += 2;

        node->payload = std::string(buffer, len);
        buffer += len;
        break;
    }
    default:
        std::cout << "error" << std::endl;
        // error("error while reading payload: wrong ID {}", static_cast<int>(id));
    }
}

void write_node(const nbt_node &node, std::vector<unsigned char> &buffer)
{
    if (node.tagtype() == NbtTagType::TAG_END) {
        buffer.push_back(0);// no name for Tag_End
        return;
    }
    buffer.push_back(static_cast<unsigned char>(node.tagtype()));
    write_name(node.name, buffer);

    write_payload(node, buffer);
}

nbt_node read_from_file(const string &filename)
{
    std::ifstream infile{ filename, std::ios::binary };
    if (!infile) throw std::runtime_error("couldn't open file");

    infile.seekg(0, std::ifstream::end);
    auto length = static_cast<size_t>(infile.tellg()) - sizeof(uLongf);
    infile.seekg(0, std::ifstream::beg);

    uLongf uncompressed_length;
    infile.read(reinterpret_cast<char *>(&uncompressed_length), sizeof(uLongf));

    char *buffer_content = new char[length];
    char *buffer_uncompressed = new char[uncompressed_length];
    infile.read(buffer_content, length);

    auto z_result = uncompress(reinterpret_cast<unsigned char *>(buffer_uncompressed),
        &uncompressed_length,
        reinterpret_cast<unsigned char *>(buffer_content),
        length);
    switch (z_result) {
    case Z_OK:
        break;
    case Z_MEM_ERROR:
        throw std::runtime_error("Zlib::z_mem_error");
    case Z_BUF_ERROR:
        throw std::runtime_error("Zlib::z_buf_error");
    }

    const char *read_buff = reinterpret_cast<char *>(buffer_uncompressed);
    auto innode = nbt::read_node(read_buff);
    delete[] buffer_content;
    delete[] buffer_uncompressed;

    return innode;
}

void write_to_file(const nbt_node &node, const string &filename)
{

    std::vector<unsigned char> buffer;

    auto s = node.calc_size();
    buffer.reserve(s);
    nbt::write_node(node, buffer);

    uLong length_uncompressed = buffer.size();
    uLongf length_compressed = std::max((size_t)256, buffer.size());
    unsigned char *compressed_buffer =
        new unsigned char[length_compressed];// worst case: compressed has the same size as

    auto z_result = compress(compressed_buffer, &length_compressed, buffer.data(), length_uncompressed);
    switch (z_result) {
    case Z_OK:
        break;
    case Z_MEM_ERROR:
        std::cout << "z_mem_error" << std::endl;
        return;
    case Z_BUF_ERROR:
        std::cout << "z_buf_error" << std::endl;
        return;
    }
    std::ofstream out_file{ filename, std::ios::binary };
    out_file.write(reinterpret_cast<char *>(&length_uncompressed), sizeof(uLongf));
    out_file.write(reinterpret_cast<char *>(compressed_buffer), length_compressed);

    delete[] compressed_buffer;
}

static size_t calc_name_size(nbt_node const &node) { return 2 + node.name.size(); }

size_t nbt_node::calc_size() const
{
    using enum NbtTagType;

    switch (tagtype()) {
    case TAG_END:
        return 1;
    case TAG_Compound: {
        auto acc = [](size_t sum, nbt_node const &a) -> size_t { return sum + a.calc_size(); };
        auto const &content = std::get<compound>(payload).content;
        return (size_t)1 + calc_name_size(*this) + std::accumulate(content.begin(), content.end(), (size_t)0, acc);
    }
    case TAG_List: {
        // 1 (tag type) + name size + 1 (element type) + 4 (length) + elements
        auto const& list = get<TAG_List>();
        auto elem_type = list.content_type();
        size_t elements_size = 0;

        switch (elem_type) {
        case TAG_END:
            break;
        case TAG_Byte:
            elements_size = list.get<TAG_Byte>().size();
            break;
        case TAG_Short:
            elements_size = list.get<TAG_Short>().size() * 2;
            break;
        case TAG_Int:
            elements_size = list.get<TAG_Int>().size() * 4;
            break;
        case TAG_Long:
            elements_size = list.get<TAG_Long>().size() * 8;
            break;
        case TAG_Float:
            elements_size = list.get<TAG_Float>().size() * 4;
            break;
        case TAG_Double:
            elements_size = list.get<TAG_Double>().size() * 8;
            break;
        case TAG_Byte_Array: {
            for (auto const& arr : list.get<TAG_Byte_Array>()) {
                elements_size += 4 + arr.size();  // length prefix + data
            }
            break;
        }
        case TAG_String: {
            for (auto const& str : list.get<TAG_String>()) {
                elements_size += 2 + str.size();  // length prefix + data
            }
            break;
        }
        case TAG_List: {
            for (auto const& inner : list.get<TAG_List>()) {
                nbt_node temp;
                temp.payload = inner;
                // Subtract 1 (no tag type for list element) and name overhead
                elements_size += temp.calc_size() - 1 - calc_name_size(temp);
            }
            break;
        }
        case TAG_Compound: {
            for (auto const& comp : list.get<TAG_Compound>()) {
                for (auto const& child : comp.content) {
                    elements_size += child.calc_size();
                }
                elements_size += 1;  // TAG_End
            }
            break;
        }
        case TAG_Int_Array: {
            for (auto const& arr : list.get<TAG_Int_Array>()) {
                elements_size += 4 + arr.size() * 4;  // length prefix + data
            }
            break;
        }
        case TAG_Long_Array: {
            for (auto const& arr : list.get<TAG_Long_Array>()) {
                elements_size += 4 + arr.size() * 8;  // length prefix + data
            }
            break;
        }
        }
        return 1 + calc_name_size(*this) + 1 + 4 + elements_size;
    }
    case TAG_Byte_Array: {
        std::cout << get<TAG_Byte_Array>().size() << std::endl;
        return 1 + calc_name_size(*this) + get<TAG_Byte_Array>().size();
    }
    case TAG_Int_Array: {
        return 1 + calc_name_size(*this) + get<TAG_Int_Array>().size() * 4;
    }
    case TAG_Long_Array: {
        return 1 + calc_name_size(*this) + get<TAG_Long_Array>().size() * 8;
    }
    case TAG_String: {
        return 1 + calc_name_size(*this) + get<TAG_String>().size();
    }
    case TAG_Byte: {
        return 1 + calc_name_size(*this) + 1;
    }
    case TAG_Double:
    case TAG_Long: {
        return 1 + calc_name_size(*this) + 8;
    }
    case TAG_Float:
    case TAG_Int: {
        return 1 + calc_name_size(*this) + 4;
    }
    case TAG_Short: {
        return 1 + calc_name_size(*this) + 2;
    }
    }
    return 0;
}
const nbt_node *compound::operator[](const std::string &key) const
{
    auto found = std::find_if(content.begin(), content.end(), [&key](const nbt_node el) { return el.name == key; });

    if (found == content.end()) return nullptr;

    return &*found;
}
std::vector<nbt_node>::iterator compound::end() { return content.end(); }
std::vector<nbt_node>::iterator compound::begin() { return content.begin(); }

std::vector<nbt_node>::const_iterator compound::end() const { return content.end(); }
std::vector<nbt_node>::const_iterator compound::begin() const { return content.begin(); }

// ---- Gzip File I/O (Minecraft-compatible) ----

nbt_node read_from_file_gzip(const string &filename)
{
    gzFile gz = gzopen(filename.c_str(), "rb");
    if (!gz) {
        throw std::runtime_error("Failed to open gzip file: " + filename);
    }

    // Read in chunks and accumulate
    std::vector<char> buffer;
    constexpr size_t CHUNK_SIZE = 16384;
    char chunk[CHUNK_SIZE];
    int bytes_read;

    while ((bytes_read = gzread(gz, chunk, CHUNK_SIZE)) > 0) {
        buffer.insert(buffer.end(), chunk, chunk + bytes_read);
    }

    if (bytes_read < 0) {
        int errnum;
        const char* errmsg = gzerror(gz, &errnum);
        gzclose(gz);
        throw std::runtime_error(std::string("Gzip read error: ") + errmsg);
    }

    gzclose(gz);

    if (buffer.empty()) {
        throw std::runtime_error("Empty gzip file: " + filename);
    }

    const char* read_ptr = buffer.data();
    return read_node(read_ptr);
}

void write_to_file_gzip(const nbt_node &node, const string &filename)
{
    std::vector<unsigned char> buffer;
    auto s = node.calc_size();
    buffer.reserve(s);
    write_node(node, buffer);

    gzFile gz = gzopen(filename.c_str(), "wb");
    if (!gz) {
        throw std::runtime_error("Failed to create gzip file: " + filename);
    }

    int bytes_written = gzwrite(gz, buffer.data(), static_cast<unsigned>(buffer.size()));
    if (bytes_written == 0) {
        int errnum;
        const char* errmsg = gzerror(gz, &errnum);
        gzclose(gz);
        throw std::runtime_error(std::string("Gzip write error: ") + errmsg);
    }

    gzclose(gz);
}

// ---- Uncompressed File I/O ----

nbt_node read_from_file_uncompressed(const string &filename)
{
    std::ifstream infile{filename, std::ios::binary};
    if (!infile) {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    infile.seekg(0, std::ifstream::end);
    auto length = static_cast<size_t>(infile.tellg());
    infile.seekg(0, std::ifstream::beg);

    std::vector<char> buffer(length);
    infile.read(buffer.data(), length);

    const char* read_ptr = buffer.data();
    return read_node(read_ptr);
}

void write_to_file_uncompressed(const nbt_node &node, const string &filename)
{
    std::vector<unsigned char> buffer;
    auto s = node.calc_size();
    buffer.reserve(s);
    write_node(node, buffer);

    std::ofstream outfile{filename, std::ios::binary};
    if (!outfile) {
        throw std::runtime_error("Failed to create file: " + filename);
    }

    outfile.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
}

nbt_node read_from_buffer(const char* buffer, size_t size)
{
    (void)size;  // Could be used for bounds checking in the future
    return read_node(buffer);
}

}// namespace nbt
