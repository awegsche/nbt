#ifndef REGION_H_
#define REGION_H_

#include "nbt.h"

namespace nbt {
constexpr unsigned int HEADER_SIZE = 4;

#pragma region I /O

/// Load region in anvil format
/// \param filename
/// \return
std::vector<nbt_node> load_region(const char *filename);

} // namespace nbt

#endif // REGION_H_
