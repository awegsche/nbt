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

// #include "Timer.h"


using std::get;
using std::string;

constexpr uint16_t MAXLEVEL = 10;

namespace nbt {

std::string nbt_node::pretty_print(uint16_t level) const
{
    using enum NbtTagType;

    if (level > MAXLEVEL) return "";

    std::stringstream ss;
    plevel plev{ level };

    ss << plev;

    if (!name.empty()) ss << "\o{33}[1m" << name << "\o{33}[0m: ";

    switch (tagtype()) {
    case TAG_END:
        ss << "END";
        break;
    case TAG_Short:
        ss << "short " << get<TAG_Short>();
        break;
    case TAG_Compound:
        ss << "Compound {";
        for (nbt_node n : get<TAG_Compound>().content) ss << "\n" << n.pretty_print(level + 1);
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
    case TAG_List:
        ss << "List {";
        // for (const nbt_node &n : get<TAG_List>()) { ss << "\n" << n.pretty_print(level + 1); }
        ss << " not implemented ";
        ss << "}" << plev;
        break;
    case TAG_String:
        ss << "string \"" << get<TAG_String>() << "\"";
        break;
    case TAG_Byte_Array: {
        ss << "byte array {";
        if (const auto &array = get<TAG_Byte_Array>(); array.size() > 10) {
            for (size_t i = 0; i < 8; i++) ss << static_cast<int>(array[i]) << ", ";
            ss << " ...";
        } else {
            for (int b : array) ss << b << ", ";
        }
        ss << "}";
        break;
    }
    case TAG_Int_Array: {
        ss << "int array {";
        if (const auto &array = get<TAG_Int_Array>(); array.size() > 10) {
            for (size_t i = 0; i < 8; i++) ss << array[i] << ", ";
            ss << " ...";
        } else {
            for (int b : array) ss << b << ", ";
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
    if (id == NbtTagType::TAG_END) return nbt_node{};

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

void write_name(std::string const &name, std::vector<unsigned char> &buffer)
{

    auto length = static_cast<int16_t>(name.size());
    push_swapped2(buffer, &length);

    for (char c : name) buffer.push_back(c);
}

void write_payload(const nbt_node &node, std::vector<unsigned char> &buffer)
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
        throw std::runtime_error("not implemented");
        /*
        auto const &payload = node.get<TAG_List>();
        // get type from first element
        auto content_id = payload[0].tagtype();
        auto len = static_cast<int32_t>(payload.size());
        buffer.push_back(static_cast<unsigned char>(content_id));
        push_swapped4(buffer, &len);
        for (const auto &child : payload) write_payload(child, buffer);
        */
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
        for (char c : str) buffer.push_back(c);

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
        // auto contentid = *reinterpret_cast<const NbtTagType *>(buffer++);
        throw std::runtime_error("not implemented");
        /*
        auto contentid = static_cast<NbtTagType>(*buffer++);
        int len = __swap4(buffer);
        buffer += 4;
        node->payload = std::vector<nbt_node>();

        auto &content = node->get<TAG_List>();
        for (int i = 0; i < len; i++) {
            auto &item = content.emplace_back(contentid);
            get_payload(contentid, buffer, &item);
        }
        */
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

        char *buf = new char[len + 1];
        memcpy(buf, buffer, len);
        buf[len] = 0;
        node->payload = std::string(buf);
        buffer += len;
        delete[] buf;
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

// const nbt_node* get_child(const std::string& name, const nbt_node& parent)
//{
//	if (parent.tagtype == NbtTagType::TAG_Compound) {
//        return parent.get<TAG_Compound>()[name];
//	}

//	std::stringstream ss;
//	ss << "there is no child\"" << name;
//	throw std::runtime_error(ss.str().c_str());

//	//return __end__;
//}

/*
std::vector<nbt_node*> load_region(const char* filename) {
        FILE* file = fopen(filename, "rb");
        std::vector<std::unique_ptr<nbt_node>> chunk;
        char* buffer = new char[HEADER_SIZE];
        char* swapped_header = new char[HEADER_SIZE];

        for (int i = 0; i < 10; i++) {
                fseek(file, i * HEADER_SIZE, SEEK_SET);
                fread(buffer, 1, HEADER_SIZE, file);

                buffer[3] = 0;
                swap_bytes(buffer, HEADER_SIZE);
                int offset = *reinterpret_cast<int*>(buffer) >> 8;

                if (offset == 0) continue;
                std::cout << "chunk " << i << ", offset = " << offset <<
std::endl;
        }
}
*/

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
        // same as for compound, expect the size of each child node is 3 smaller,
        // - because it doesn't carry a tag (-1)
        // - nor a name (-2 for the int16_t length)
        throw std::runtime_error("not implemented");
        /*
        auto acc = [](size_t sum, nbt_node const &a) -> size_t { return sum + a.calc_size() - (size_t)3; };
        auto const &content = std::get<TAG_List>(payload);
        return (size_t)1 + calc_name_size(*this) + std::accumulate(content.begin(), content.end(), (size_t)0, acc);
        */
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
}// namespace nbt

std::ostream &operator<<(std::ostream &os, const nbt::nbt_node &n)
{
    os << n.pretty_print();
    return os;
}
