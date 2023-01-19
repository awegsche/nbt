#include "common.h"
// #include <spdlog/sinks/stdout_color_sinks.h>
#include "common.h"
#include "nbt.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

int main(int argc, char **argv) {

  // ---- setup logging
  // --------------------------------------------------------------------------
  auto console = spdlog::stdout_color_mt("console");
  spdlog::set_pattern("%^%5l%$ | %v");
  spdlog::set_level(spdlog::level::trace);

  info("hello {}", 42);
  std::cout << "hello cout" << std::endl;

  // ---- check writing and reading in again
  // -----------------------------------------------------
  std::vector<byte> field;
  field.push_back(255);
  field.push_back(255);
  field.push_back(255);
  field.push_back(255);
  field.push_back(255);
  field.push_back(255);
  field.push_back(255);
  nbt::nbt_node node{field};
  node.name = "Test int 32";

  // write node to file
  nbt::write_to_file(node, "test_file.nbt");

  // read back in
  auto innode = nbt::read_from_file("test_file.nbt");

  std::cout << "node (should be TAG_BYTE_ARRAY{255, ...}): " << node
            << std::endl;
  std::cout << "innode : " << innode << std::endl;

  return 0;

#ifdef __MINGW32__
  std::cout << "mingw32" << std::endl;
  char filepath[] = "K:/cpp/nbt/region/region/r.0.0.mca";
#else
  char filepath[] = "K:/projects/nbt/region/region/r.0.0.mca";
#endif

  // std::filesystem::path test_path = std::filesystem::path{"K:"} / "projects"
  // / "nbt" / "region" / "region" / "r.0.0.mca"; info(test_path.c_str());

  auto chunks = nbt::load_region(filepath);
  if (chunks.size() > 0) {
    std::cout << chunks[0] << std::endl;

    auto chunk0 = chunks[0];
  } else {
    return 1;
  }

  return 0;
}
