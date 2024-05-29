#pragma once
#include <array>
#include <cstdint>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>
#include <zlib.h>

using std::variant;
using byte = unsigned char;

namespace nbt {

enum NbtTagType : uint8_t {
    TAG_END,
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

struct TagEnd {};

// forward decl of nbt_node struc
struct nbt_node;

/// special struct for holding a compound
struct compound {
    std::vector<nbt_node> content;

    /// Gets the child node with name `key`
    /// \param key
    /// \return returns `nullptr` if no child with name `key` could be found
    const nbt_node *operator[](const std::string &key) const;

    /// Gets the child node with name `key`
    /// \param key
    /// \return returns `nullptr` if no child with name `key` could be found
    nbt_node *operator[](const std::string &key) { return const_cast<nbt_node *>(std::as_const(*this)[key]); }

    template <class T> void insert_node(T const &value, std::string const &name);
    template <class T> void insert_node(T &&value, std::string const &name);

    std::vector<nbt_node>::iterator begin();
    std::vector<nbt_node>::iterator end();

    std::vector<nbt_node>::const_iterator begin() const;
    std::vector<nbt_node>::const_iterator end() const;
};

struct nbt_node {

    typedef variant<TagEnd, byte, int16_t, int32_t, int64_t, float, double, std::vector<byte>, std::string,
                    std::vector<nbt_node>, compound, std::vector<int32_t>, std::vector<int64_t>>
        payload_t;

    // ---- Init -----------------------------------------------------------------------------------
    nbt_node() {}
    nbt_node(byte b) : payload(b) {}
    nbt_node(int32_t i) : payload(i) {}
    nbt_node(int64_t l) : payload(l) {}
    nbt_node(float f) : payload(f) {}
    nbt_node(double d) : payload(d) {}
    nbt_node(std::vector<byte> b_array) : payload(b_array) {}
    nbt_node(std::vector<int32_t> i32_array) : payload(i32_array) {}
    nbt_node(std::vector<int64_t> i64_array) : payload(i64_array) {}
    nbt_node(std::vector<nbt_node> list) : payload(list) {}
    nbt_node(compound &&comp) : payload(comp) {}

    /// get type info (internally handled by std::variant)
    NbtTagType tagtype() const { return static_cast<NbtTagType>(payload.index()); }

    /// retrieve content by tagtype
    template <uint8_t I> auto get() const -> const auto & { return std::get<I>(payload); }

    /// const version of `get<I>()`
    template <uint8_t I> auto get() -> auto & { return std::get<I>(payload); }

    template <uint8_t I> const auto &get_field(const std::string &name) const {
        auto *child = at(name);
        if (child)
            return child->get<I>();
        throw std::runtime_error("child doesn't exist");
    }

    nbt_node const *at(const std::string &name) const {
        if (tagtype() == NbtTagType::TAG_Compound) {
            return get<NbtTagType::TAG_Compound>()[name];
        }
        return nullptr;
    }

    nbt_node *at(const std::string &name) { return const_cast<nbt_node *>(std::as_const(*this).at(name)); }

    /// nbt_node n == false if it is a TagEnd
    operator bool() { return payload.index() != 0; }

    /// pre-calculates the size for writing to a buffer
    size_t calc_size() const;

    std::string pretty_print(uint16_t level = 0) const;

    payload_t payload = TagEnd{};
    std::string name;
};

/// Load an nbt_node from file
nbt_node read_from_file(std::string const &filename);

/// Write file
void write_to_file(nbt_node const &node, std::string const &filename);

/// Read node from a byte buffer (e.g. from ifstream or zlib)
nbt_node read_node(const char *&buffer);

/// Write node to buffer
void write_node(const nbt_node &node, std::vector<unsigned char> &buffer);

#pragma endregion

/// (internal) read name from buffer
std::string get_name(const char *&buffer);

/// (internal) convert payload
void get_payload(NbtTagType id, const char *&buffer, nbt_node *node);

/// printing indentation level
struct plevel {
    uint16_t l;
};

template <class T> void compound::insert_node(T const &value, std::string const &name) {
    auto &node = content.emplace_back(value);
    node.name  = name;
}
template <class T> void compound::insert_node(T &&value, std::string const &name) {
    auto &node = content.emplace_back(std::move(value));
    node.name  = name;
}
} // namespace nbt

inline std::ostream &operator<<(std::ostream &os, const nbt::plevel &l) {
    for (uint16_t i = 0; i < l.l - 1; i++) {
        os.put(' ');
        os.put(' ');
    }
    os.put(' ');
    os.put(' ');
    return os;
}

std::ostream &operator<<(std::ostream &os, const nbt::nbt_node &n);
