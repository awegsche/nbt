#include "region.h"
#include <fstream>

#include "common.h"

namespace nbt {

constexpr auto REGION_OFFSET = 4096;
constexpr auto CHUNKXZ = 32;
constexpr auto CHUNKS_IN_REGION = CHUNKXZ * CHUNKXZ;
constexpr auto UNC_CHUNKLENGTH = 2U << 17U;

std::vector<nbt_node> load_region(const char *filename)
{
    std::ifstream region(filename, std::ios::binary);
    if (!region) {
        // throw std::runtime_error("failed to load \"{}\"", filename);
        throw std::runtime_error("failed to load file");
    }
    region.seekg(0, std::ifstream::end);
    auto const length = region.tellg();
    region.seekg(0, std::ifstream::beg);

    std::vector<char> buffer;
    buffer.resize(length);
    region.read(buffer.data(), length);
    // debug("read {} bytes of region data.", length);

    std::vector<nbt_node> chunks;

    for (size_t i = 0; i < CHUNKS_IN_REGION; i++) {
        uint32_t location = __swap4(reinterpret_cast<uint32_t *>(buffer.data() + i * 4));
        auto offset = static_cast<size_t>(location >> 8);
        if (offset == 0) continue;

        auto chunkb = buffer.data() + offset * REGION_OFFSET;
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
            uncompress(reinterpret_cast<Bytef *>(unc_chunkb), &length_unc, reinterpret_cast<Bytef *>(chunkb), length);
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

    return chunks;
}
}// namespace nbt
