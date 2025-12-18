#ifndef REGION_H_
#define REGION_H_

#include "nbt.h"
#include <array>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>

namespace nbt {

/// Number of chunks per region dimension (32x32 = 1024 chunks per region)
constexpr int REGION_DIMENSION = 32;
constexpr int CHUNKS_PER_REGION = REGION_DIMENSION * REGION_DIMENSION;

/// Size of a sector in bytes (4 KiB)
constexpr size_t SECTOR_SIZE = 4096;

/// Header size in bytes (location table + timestamp table)
constexpr size_t HEADER_SIZE = 2 * SECTOR_SIZE;

/// Compression types used in Minecraft region files
enum class CompressionType : uint8_t {
    GZIP = 1,           // GZip (RFC1952) - rarely used
    ZLIB = 2,           // Zlib (RFC1950) - most common
    UNCOMPRESSED = 3,   // Uncompressed (since 1.15.1)
    LZ4 = 4,            // LZ4 (since 24w04a) - not yet widely used
    CUSTOM = 127        // Custom compression (external)
};

/// Entry for a single chunk in the region
struct ChunkEntry {
    /// Offset in 4KiB sectors from start of file (0 = chunk doesn't exist)
    uint32_t offset = 0;
    
    /// Number of sectors the chunk occupies
    uint8_t sector_count = 0;
    
    /// Timestamp of last modification (Unix timestamp)
    uint32_t timestamp = 0;
    
    /// Compression type used for this chunk
    CompressionType compression = CompressionType::ZLIB;
    
    /// The actual chunk data (loaded on demand)
    std::optional<nbt_node> data;
    
    /// Returns true if this chunk exists in the region
    [[nodiscard]] bool exists() const { return offset != 0; }
};

/// Represents a Minecraft region file (Anvil format)
/// 
/// Region files contain up to 1024 chunks arranged in a 32x32 grid.
/// Each chunk is independently compressed and stored at a sector-aligned offset.
///
/// File structure:
/// - Bytes 0-4095: Location table (1024 entries, 4 bytes each)
/// - Bytes 4096-8191: Timestamp table (1024 entries, 4 bytes each)
/// - Bytes 8192+: Chunk data in sectors
struct Region {
    /// All 1024 chunk entries (indexed by z * 32 + x)
    std::array<ChunkEntry, CHUNKS_PER_REGION> chunks;
    
    /// Region coordinates (derived from filename)
    int region_x = 0;
    int region_z = 0;
    
    /// Calculate chunk index from local coordinates (0-31, 0-31)
    [[nodiscard]] static constexpr size_t chunk_index(int local_x, int local_z) {
        return static_cast<size_t>(local_z * REGION_DIMENSION + local_x);
    }
    
    /// Get a chunk by local coordinates (0-31, 0-31)
    /// Returns nullptr if chunk doesn't exist or isn't loaded
    [[nodiscard]] const nbt_node* get_chunk(int local_x, int local_z) const {
        if (local_x < 0 || local_x >= REGION_DIMENSION || 
            local_z < 0 || local_z >= REGION_DIMENSION) {
            return nullptr;
        }
        auto& entry = chunks[chunk_index(local_x, local_z)];
        if (entry.data.has_value()) {
            return &entry.data.value();
        }
        return nullptr;
    }
    
    /// Get a mutable chunk by local coordinates
    [[nodiscard]] nbt_node* get_chunk(int local_x, int local_z) {
        return const_cast<nbt_node*>(std::as_const(*this).get_chunk(local_x, local_z));
    }
    
    /// Get chunk entry by local coordinates
    [[nodiscard]] const ChunkEntry& get_entry(int local_x, int local_z) const {
        return chunks[chunk_index(local_x, local_z)];
    }
    
    /// Get mutable chunk entry by local coordinates
    [[nodiscard]] ChunkEntry& get_entry(int local_x, int local_z) {
        return chunks[chunk_index(local_x, local_z)];
    }
    
    /// Count how many chunks exist in this region
    [[nodiscard]] size_t count_chunks() const {
        size_t count = 0;
        for (const auto& entry : chunks) {
            if (entry.exists()) count++;
        }
        return count;
    }
    
    /// Count how many chunks are loaded
    [[nodiscard]] size_t count_loaded() const {
        size_t count = 0;
        for (const auto& entry : chunks) {
            if (entry.data.has_value()) count++;
        }
        return count;
    }
};

/// Load a region file and all its chunks
/// @param filename Path to the .mca region file
/// @return Fully loaded Region with all existing chunks parsed
Region load_region(const std::string& filename);

/// Load a region file, parsing only the header (no chunk data)
/// @param filename Path to the .mca region file
/// @return Region with header info but no chunk data loaded
Region load_region_header(const std::string& filename);

/// Load a specific chunk from a region file
/// @param filename Path to the .mca region file
/// @param local_x Local X coordinate (0-31)
/// @param local_z Local Z coordinate (0-31)
/// @return The chunk data, or nullopt if chunk doesn't exist
std::optional<nbt_node> load_chunk(const std::string& filename, int local_x, int local_z);

/// Calculate the region file coordinates from world chunk coordinates
/// @param chunk_x World chunk X coordinate
/// @param chunk_z World chunk Z coordinate
/// @return Pair of (region_x, region_z)
[[nodiscard]] constexpr std::pair<int, int> chunk_to_region(int chunk_x, int chunk_z) {
    // Integer division that rounds toward negative infinity
    auto floor_div = [](int a, int b) -> int {
        return a / b - (a % b != 0 && (a ^ b) < 0);
    };
    return {floor_div(chunk_x, REGION_DIMENSION), floor_div(chunk_z, REGION_DIMENSION)};
}

/// Calculate local chunk coordinates within a region from world chunk coordinates
/// @param chunk_x World chunk X coordinate
/// @param chunk_z World chunk Z coordinate
/// @return Pair of (local_x, local_z) in range [0, 31]
[[nodiscard]] constexpr std::pair<int, int> chunk_to_local(int chunk_x, int chunk_z) {
    // Proper modulo that always returns positive
    auto pos_mod = [](int a, int b) -> int {
        return ((a % b) + b) % b;
    };
    return {pos_mod(chunk_x, REGION_DIMENSION), pos_mod(chunk_z, REGION_DIMENSION)};
}

/// Build the region filename from region coordinates
/// @param region_x Region X coordinate
/// @param region_z Region Z coordinate
/// @return Filename like "r.0.-1.mca"
[[nodiscard]] inline std::string region_filename(int region_x, int region_z) {
    return "r." + std::to_string(region_x) + "." + std::to_string(region_z) + ".mca";
}

/// Load a chunk by world coordinates from a world's region folder
/// @param region_folder Path to the region folder (e.g., "world/region")
/// @param chunk_x World chunk X coordinate  
/// @param chunk_z World chunk Z coordinate
/// @return The chunk data, or nullopt if chunk doesn't exist
std::optional<nbt_node> load_chunk_from_world(
    const std::filesystem::path& region_folder,
    int chunk_x,
    int chunk_z);

// Legacy function - kept for backwards compatibility
std::vector<nbt_node> load_region_legacy(const char* filename);

}  // namespace nbt

#endif  // REGION_H_
