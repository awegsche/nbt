# nbt ![release](https://github.com/awegsche/nbt/actions/workflows/release.yml/badge.svg)

Yet another nbt library, for my personal use and learning experience.

## Disclaimer

I created this project to practice C++ development.
If you are interested in an NBT library for your language, checkout
[this wiki entry](https://wiki.vg/NBT) ,
because "There are many, many libraries for manipulating NBT, written in several languages, and often several per language."

## Usage

The goal is to provide a modern C++ library with ergonomy as main focus (see [nlohmann-json](https://github.com/nlohmann/json) for an excellent example).

```cpp
using nbt::compound, nbt::nbt_node;

// create root object of type compound
compound root{};

// insert some elements. The tag types are deduced from the value types (int -> TAG_Int, std::vector<int> -> TAG_IntArray, etc.)
root.insert_node(1024, "width"); 

std::vector<int> data{};
/* ... setup the data ... */
root.insert_node(data, "data");

// write to a file
nbt::write_to_file(nbt_node{root}, "filename.nbt");
```

## Road map and known issues

- `std::map` like insertion

  ```cpp
  node["width"] = 1024;
  ```
  
- Fix region format loading

  Minecraft region files can be loaded. However, I didn't update this in a long time and it might not be compatible with current region formats.
  
- Add more testing and static analysis

  As I use this project to practice some best practices, it needs more testing and clang-tidy / warnings as errors etc.
