# nbt
![release](https://github.com/awegsche/nbt/actions/workflows/release.yml/badge.svg)
[![Quality gate](https://sonarcloud.io/api/project_badges/quality_gate?project=awegsche1_nbt)](https://sonarcloud.io/summary/new_code?id=awegsche1_nbt)

A modern C++ library for reading and writing Minecraft's Named Binary Tag (NBT) format.

## About the NBT Format

**Named Binary Tag (NBT)** is a tree-based binary data structure used by Minecraft to store game data including world saves, player data, and item information. The format is designed to be simple, compact, and efficient for serialization.

### Tag Types

NBT supports 13 tag types:

| ID | Tag Type | Description | C++ Type |
|----|----------|-------------|----------|
| 0 | `TAG_End` | Marks the end of a compound tag | - |
| 1 | `TAG_Byte` | Signed 8-bit integer (-128 to 127) | `byte` |
| 2 | `TAG_Short` | Signed 16-bit integer | `int16_t` |
| 3 | `TAG_Int` | Signed 32-bit integer | `int32_t` |
| 4 | `TAG_Long` | Signed 64-bit integer | `int64_t` |
| 5 | `TAG_Float` | 32-bit IEEE 754 floating point | `float` |
| 6 | `TAG_Double` | 64-bit IEEE 754 floating point | `double` |
| 7 | `TAG_Byte_Array` | Array of signed bytes | `std::vector<byte>` |
| 8 | `TAG_String` | UTF-8 string (length-prefixed) | `std::string` |
| 9 | `TAG_List` | List of unnamed tags of the same type | `nbt_list` |
| 10 | `TAG_Compound` | Collection of named tags | `compound` |
| 11 | `TAG_Int_Array` | Array of signed 32-bit integers | `std::vector<int32_t>` |
| 12 | `TAG_Long_Array` | Array of signed 64-bit integers | `std::vector<int64_t>` |

### Binary Structure

Each tag in the binary format consists of:
1. **Tag type** (1 byte) - The tag type ID
2. **Name** (for named tags) - A length-prefixed UTF-8 string
3. **Payload** - The tag's data (format depends on tag type)

NBT files are typically compressed with gzip or zlib compression. This library handles both compressed and uncompressed files.

For more details on the NBT specification, see the [Minecraft Wiki - NBT format](https://minecraft.fandom.com/wiki/NBT_format).

## Usage

The library provides an ergonomic, modern C++ API inspired by [nlohmann-json](https://github.com/nlohmann/json).

### Creating NBT Data

```cpp
using nbt::compound, nbt::nbt_node;

// Create root object of type compound
compound root{};

// Insert elements - tag types are automatically deduced from value types
root.insert_node(1024, "width");              // TAG_Int
root.insert_node(768, "height");              // TAG_Int
root.insert_node(3.14159f, "pi");             // TAG_Float
root.insert_node("Hello NBT", "greeting");    // TAG_String

std::vector<int> data{1, 2, 3, 4, 5};
root.insert_node(data, "data");               // TAG_Int_Array

// Write to file (automatically gzip compressed)
nbt::write_to_file(nbt_node{root}, "output.nbt");
```

### Reading NBT Data

```cpp
// Read from file
nbt::nbt_node node = nbt::read_from_file("level.dat");

// Access compound children by name
if (auto* data = node.at("Data")) {
    // Get typed values
    auto& level_name = data->get_field<nbt::NbtTagType::TAG_String>("LevelName");
    auto& spawn_x = data->get_field<nbt::NbtTagType::TAG_Int>("SpawnX");
}

// Pretty print the structure
std::cout << node << std::endl;
```

### Nested Compounds and Lists

```cpp
compound player{};
player.insert_node("Steve", "Name");

compound position{};
position.insert_node(100.5, "X");
position.insert_node(64.0, "Y");
position.insert_node(-200.3, "Z");
player.insert_node(std::move(position), "Pos");

nbt::write_to_file(nbt_node{player}, "player.dat");
```

## Features

- **Type-safe access** via `std::variant` and templated getters
- **Automatic type deduction** when inserting values
- **Gzip compression** support for reading and writing
- **Pretty printing** for debugging and inspection
- **Region file support** for Minecraft world data (experimental)

## Building

This project uses CMake and vcpkg for dependency management:

```bash
cmake --preset=debug
cmake --build builds/debug
```

### Dependencies

- **zlib** - For gzip compression/decompression
- **Google Test** - For unit testing (optional)

## Road Map

- [ ] `std::map`-like insertion syntax: `node["width"] = 1024;`
- [ ] Update region format support for latest Minecraft versions
- [ ] SNBT (Stringified NBT) parsing and output

## References

- [Minecraft Wiki - NBT format](https://minecraft.fandom.com/wiki/NBT_format)
- [Minecraft Wiki - Region file format](https://minecraft.fandom.com/wiki/Region_file_format)
- [wiki.vg - NBT](https://wiki.vg/NBT)
