#include "../include/nbt.h"
#include "common.h"
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#ifdef _MSC_VER
#include <intrin.h>
template <typename T, typename U> T __reint(U _val) {
  return *reinterpret_cast<T *>(&U);
}
template <typename T> unsigned long __swap4(T *_val) {
  return _byteswap_ulong(*reinterpret_cast<unsigned long *>(_val));
}
template <typename T> unsigned long __swap4(const T *_val) {
  return _byteswap_ulong(*reinterpret_cast<const unsigned long *>(_val));
}
template <typename T> unsigned short __swap2(T *_val) {
  return _byteswap_ushort(*reinterpret_cast<unsigned short *>(_val));
}
template <typename T> unsigned short __swap2(const T *_val) {
  return _byteswap_ushort(*reinterpret_cast<const unsigned short *>(_val));
}
template <typename T> unsigned long long __swap8(T *_val) {
  return _byteswap_uint64(*reinterpret_cast<unsigned __int64 *>(_val));
}
template <typename T> unsigned long long __swap8(const T *_val) {
  return _byteswap_uint64(*reinterpret_cast<const unsigned __int64 *>(_val));
}
#elif __GNUC__
template <typename T> unsigned long __swap4(T *_val) {
  return __builtin_bswap32(*reinterpret_cast<unsigned long *>(_val));
}
template <typename T> unsigned long __swap4(const T *_val) {
  return __builtin_bswap32(*reinterpret_cast<const unsigned long *>(_val));
}
template <typename T> unsigned short __swap2(T *_val) {
  return __builtin_bswap16(*reinterpret_cast<unsigned short *>(_val));
}
template <typename T> unsigned short __swap2(const T *_val) {
  return __builtin_bswap16(*reinterpret_cast<const unsigned short *>(_val));
}
template <typename T> unsigned long long __swap8(T *_val) {
  return __builtin_bswap64(*reinterpret_cast<uint64_t *>(_val));
}
template <typename T> unsigned long long __swap8(const T *_val) {
  return __builtin_bswap64(*reinterpret_cast<const uint64_t *>(_val));
}

#endif

//#include "Timer.h"

constexpr auto REGION_OFFSET = 4096;
constexpr auto CHUNKXZ = 32;
constexpr auto CHUNKS_IN_REGION = CHUNKXZ * CHUNKXZ;
constexpr auto UNC_CHUNKLENGTH = 2 << 17;

using std::get;
using std::string;

constexpr uint16_t MAXLEVEL = 10;

namespace nbt {

std::string nbt_node::pretty_print(uint16_t level) const {
  if (level > MAXLEVEL)
    return "";

  std::stringstream ss;
  plevel plev{level};

  ss << plev;

  if (!name.empty())
    ss << "\33[1m" << name << "\33[0m: ";

  switch (tagtype()) {
  case NbtTagType::TAG_END:
    ss << "END";
    break;
  case NbtTagType::TAG_Short:
    ss << "short " << get<TAG_Short>();
    break;
  case NbtTagType::TAG_Compound:
    ss << "Compound {";
    // ss << "\n" << plevel{level-(uint16_t)1} << "|-|";
    for (nbt_node n : get<TAG_Compound>().content)
      ss << "\n" << n.pretty_print(level + 1);
    ss << "\n" << plev;

    ss << "}";
    break;
  case NbtTagType::TAG_Float:
    ss << "float " << get<TAG_Float>();
    break;
  case NbtTagType::TAG_Double:
    ss << "double " << get<TAG_Double>();
    break;
  case NbtTagType::TAG_Byte:
    ss << "byte " << static_cast<int>(get<TAG_Byte>());
    break;
  case NbtTagType::TAG_Int:
    ss << "int " << get<TAG_Int>();
    break;
  case NbtTagType::TAG_Long:
    ss << "long " << get<TAG_Long>();
    break;
  case NbtTagType::TAG_List:
    ss << "List {";
    for (nbt_node n : get<TAG_List>())
      ss << "\n" << n.pretty_print(level + 1);
    ss << "}" << plev;
    break;
  case NbtTagType::TAG_String:
    ss << "string \"" << get<TAG_String>() << "\"";
    break;
  case NbtTagType::TAG_Byte_Array: {
    ss << "byte array {";
    auto &array = get<TAG_Byte_Array>();
    if (array.size() > 10) {
      for (size_t i = 0; i < 8; i++)
        ss << static_cast<int>(array[i]) << ", ";
      ss << " ...";
    } else {
      for (int b : array)
        ss << b << ", ";
    }
    ss << "}";
    break;
  }
  default:
    ss << "TAG " << (int)tagtype() << " not implemented";
  }
  return ss.str();
}

std::vector<nbt_node> load_region(const char *filename) {
  std::ifstream region(filename, std::ios::binary);
  if (!region) {
    // throw std::runtime_error("failed to load \"{}\"", filename);
    throw std::runtime_error("failed to load file");
  }
  region.seekg(0, std::ifstream::end);
  int length = region.tellg();
  region.seekg(0, std::ifstream::beg);

  char *buffer = new char[length];
  region.read(buffer, length);
  // debug("read {} bytes of region data.", length);

  std::vector<nbt_node> chunks;

  for (size_t i = 0; i < CHUNKS_IN_REGION; i++) {
    auto location = _byteswap_ulong(*reinterpret_cast<int *>(buffer + i * 4));
    auto offset = static_cast<size_t>(location >> 8);
    if (offset == 0)
      continue;

    auto chunkb = buffer + offset * REGION_OFFSET;
    auto length = __swap4(chunkb);
    chunkb += 4;
    auto ctype = *reinterpret_cast<unsigned char *>(chunkb);

    if (ctype != 2) {
      // error("unknown compression type");
      continue;
    }
    chunkb++;

    uLongf length_unc = UNC_CHUNKLENGTH;
    auto unc_chunkb = new char[UNC_CHUNKLENGTH];
    auto z_result =
        uncompress(reinterpret_cast<Bytef *>(unc_chunkb), &length_unc,
                   reinterpret_cast<Bytef *>(chunkb), length);
    switch (z_result) {
    case Z_OK:
      break;
    case Z_MEM_ERROR:
      // error("out of memory");
      continue;
    case Z_BUF_ERROR:
      // error("output buffer wasn't large enough!");
      continue;
    }
    const char *chunk_ptr = unc_chunkb;

    auto &new_chunk = chunks.emplace_back(read_node(chunk_ptr));
    new_chunk.name = "root";
    delete[] unc_chunkb;
  }
  delete[] buffer;

  return chunks;
}

/// read an nbt_node from the given buffer
nbt_node read_node(const char *&buffer) {
  auto id = *reinterpret_cast<const NbtTagType *>(buffer++);
  if (id == NbtTagType::TAG_END)
    return nbt_node{};

  auto node = nbt_node{id};
  node.name = get_name(buffer);

  get_payload(id, buffer, &node);

  return node;
}

std::string get_name(const char *&buffer) {
  int16_t length = __swap2(buffer);
  buffer += 2;

  char *string = new char[static_cast<size_t>(length) + 1];
  memcpy(string, buffer, length);
  string[length] = 0;
  buffer += length;

  return {string};
}

template <typename T>
void push_swapped2(std::vector<char> &buffer, const T *value) {
  assert(sizeof(T) == 2);
  buffer.push_back(*(reinterpret_cast<const char *>(value) + 1));
  buffer.push_back(*reinterpret_cast<const char *>(value));
}

template <typename T>
void push_swapped4(std::vector<char> &buffer, const T *value) {
  assert(sizeof(T) == 4);
  buffer.push_back(*(reinterpret_cast<const char *>(value) + 3));
  buffer.push_back(*(reinterpret_cast<const char *>(value) + 2));
  buffer.push_back(*(reinterpret_cast<const char *>(value) + 1));
  buffer.push_back(*reinterpret_cast<const char *>(value));
}

template <typename T>
void push_swapped8(std::vector<char> &buffer, const T *value) {
  assert(sizeof(T) == 8);
  buffer.push_back(*(reinterpret_cast<const char *>(value) + 7));
  buffer.push_back(*(reinterpret_cast<const char *>(value) + 6));
  buffer.push_back(*(reinterpret_cast<const char *>(value) + 5));
  buffer.push_back(*(reinterpret_cast<const char *>(value) + 4));
  buffer.push_back(*(reinterpret_cast<const char *>(value) + 3));
  buffer.push_back(*(reinterpret_cast<const char *>(value) + 2));
  buffer.push_back(*(reinterpret_cast<const char *>(value) + 1));
  buffer.push_back(*reinterpret_cast<const char *>(value));
}

void write_name(std::string const &name, std::vector<char> &buffer) {

  auto length = static_cast<int16_t>(name.size());
  push_swapped2(buffer, &length);

  for (char c : name)
    buffer.push_back(c);
}

void write_payload(const nbt_node &node, std::vector<char> &buffer) {
  switch (node.tagtype()) {
  case NbtTagType::TAG_Byte: {
    buffer.push_back(std::get<byte>(node.payload));
    break;
  }
  case NbtTagType::TAG_Short: {
    push_swapped2(buffer, &std::get<int16_t>(node.payload));
    break;
  }
  case NbtTagType::TAG_Int: {
    push_swapped4(buffer, &std::get<int32_t>(node.payload));
    break;
  }
  case NbtTagType::TAG_Long: {
    push_swapped8(buffer, &std::get<int64_t>(node.payload));
    break;
  }
  case NbtTagType::TAG_Float: {
    push_swapped4(buffer, &std::get<float>(node.payload));
    break;
  }
  case NbtTagType::TAG_Double: {
    push_swapped8(buffer, &std::get<double>(node.payload));
    break;
  }
  case NbtTagType::TAG_Byte_Array: {
    auto const &payload = std::get<std::vector<byte>>(node.payload);
    auto len = static_cast<int32_t>(payload.size());
    push_swapped4(buffer, &len);
    for (byte b : payload) {
      buffer.push_back(b);
    }
    break;
  }
  case NbtTagType::TAG_Int_Array: {
    auto const &payload = node.get<NbtTagType::TAG_Int_Array>();
    int32_t len = static_cast<int32_t>(payload.size());
    push_swapped4(buffer, &len);
    for (int32_t b : payload) {
      buffer.push_back(b);
    }
    break;
  }
  case NbtTagType::TAG_Long_Array: {
    auto const &payload = node.get<NbtTagType::TAG_Long_Array>();
    int32_t len = static_cast<int32_t>(payload.size());
    push_swapped4(buffer, &len);
    for (int64_t b : payload) {
      buffer.push_back(b);
    }
    break;
  }
  case NbtTagType::TAG_List: {
    auto const payload = node.get<NbtTagType::TAG_List>();
    // get type from first element
    auto content_id = payload[0].tagtype();
    auto len = static_cast<int32_t>(payload.size());
    push_swapped4(buffer, &len);
    buffer.push_back(static_cast<char>(content_id));
    for (const auto &child : payload)
      write_node(child, buffer);
    break;
  }
  case NbtTagType::TAG_Compound: {
    auto const &payload = node.get<NbtTagType::TAG_Compound>().content;
    for (auto const &child : payload) {
      write_node(child, buffer);
    }
    break;
  }
  case NbtTagType::TAG_String: {
    auto const &str = node.get<NbtTagType::TAG_String>();
    auto len = static_cast<int16_t>(str.size());

    push_swapped2(buffer, &len);
    for (char c : str)
      buffer.push_back(c);

    break;
  }
  case NbtTagType::TAG_END:
    break;
  }
}

void get_payload(const NbtTagType id, const char *&buffer, nbt_node *node) {

  switch (id) {
  case NbtTagType::TAG_Byte: {
    node->payload = static_cast<byte>(*(buffer++));
    break;
  }
  case NbtTagType::TAG_Short: {
    node->payload = static_cast<int16_t>(__swap2(buffer));
    buffer += 2;
    break;
  }
  case NbtTagType::TAG_Int: {
    node->payload = static_cast<int32_t>(__swap4(buffer));
    buffer += 4;
    break;
  }
  case NbtTagType::TAG_Long: {
    node->payload = static_cast<int64_t>(__swap8(buffer));
    buffer += 8;
    break;
  }
  case NbtTagType::TAG_Float: {
    auto value = __swap4(buffer);
    node->payload = *reinterpret_cast<float *>(&value);
    buffer += 4;
    break;
  }
  case NbtTagType::TAG_Double: {
    auto value = __swap8(buffer);
    node->payload = *reinterpret_cast<double *>(&value);
    buffer += 8;
    break;
  }
  case NbtTagType::TAG_Byte_Array: {
    int len = __swap4(buffer);
    buffer += 4;
    node->payload = std::vector<byte>();
    auto &payload = node->get<TAG_Byte_Array>();
    payload.reserve(len);

    for (int i = 0; i < len; i++) {
      payload.push_back(*(buffer++));
    }
    break;
  }
  case NbtTagType::TAG_List: {
    auto contentid = *reinterpret_cast<const NbtTagType *>(buffer++);
    int len = __swap4(buffer);
    buffer += 4;
    node->payload = std::vector<nbt_node>();

    for (int i = 0; i < len; i++) {
      auto &content = node->get<TAG_List>();
      auto &item = content.emplace_back(contentid);
      get_payload(contentid, buffer, &item);
    }
    break;
  }
  case NbtTagType::TAG_Compound: {
    node->payload = nbt_node::compound{};
    while (true) {
      auto &content = node->get<TAG_Compound>().content;
      content.push_back(read_node(buffer));
      if (content[content.size() - 1].tagtype() == NbtTagType::TAG_END)
        break;
    }
    break;
  }
  case NbtTagType::TAG_Int_Array: {
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
  case NbtTagType::TAG_Long_Array: {
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
  case NbtTagType::TAG_String: {
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

void write_node(const nbt_node &node, std::vector<char> &buffer) {
  buffer.push_back(static_cast<char>(node.tagtype()));
  write_name(node.name, buffer);

  write_payload(node, buffer);
}

nbt_node read_from_file(const string &filename) {
  std::ifstream infile{"test_file.nbt"};
  if (!infile)
    throw std::runtime_error("couldn't open file");

  infile.seekg(0, std::ifstream::end);
  auto length = infile.tellg();
  infile.seekg(0, std::ifstream::beg);

  char *buffer_content = new char[length];
  infile.read(buffer_content, length);
  const char *read_buff = buffer_content;
  auto innode = nbt::read_node(read_buff);
  delete[] buffer_content;

  return innode;
}

void write_to_file(const nbt_node &node, const string &filename) {
  std::vector<char> buffer;
  buffer.reserve(1000000);
  nbt::write_node(node, buffer);
  std::ofstream test_file{filename};
  test_file.write(buffer.data(), buffer.size());
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
} // namespace nbt

std::ostream &operator<<(std::ostream &os, const nbt::nbt_node &n) {
  os << n.pretty_print();
  return os;
}
