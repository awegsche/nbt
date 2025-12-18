//
// Tests for Region file loading and chunk access
//

#include <gtest/gtest.h>
#include <filesystem>

#include "region.h"

using namespace nbt;
namespace fs = std::filesystem;

// ---- Coordinate Conversion Tests ----

TEST(RegionCoords, ChunkToRegion)
{
    // Positive coordinates
    ASSERT_EQ(chunk_to_region(0, 0), std::make_pair(0, 0));
    ASSERT_EQ(chunk_to_region(31, 31), std::make_pair(0, 0));
    ASSERT_EQ(chunk_to_region(32, 32), std::make_pair(1, 1));
    ASSERT_EQ(chunk_to_region(63, 63), std::make_pair(1, 1));
    ASSERT_EQ(chunk_to_region(64, 0), std::make_pair(2, 0));
    
    // Negative coordinates
    ASSERT_EQ(chunk_to_region(-1, -1), std::make_pair(-1, -1));
    ASSERT_EQ(chunk_to_region(-32, -32), std::make_pair(-1, -1));
    ASSERT_EQ(chunk_to_region(-33, -33), std::make_pair(-2, -2));
    
    // Mixed coordinates
    ASSERT_EQ(chunk_to_region(10, -10), std::make_pair(0, -1));
    ASSERT_EQ(chunk_to_region(-10, 10), std::make_pair(-1, 0));
}

TEST(RegionCoords, ChunkToLocal)
{
    // Positive coordinates
    ASSERT_EQ(chunk_to_local(0, 0), std::make_pair(0, 0));
    ASSERT_EQ(chunk_to_local(31, 31), std::make_pair(31, 31));
    ASSERT_EQ(chunk_to_local(32, 32), std::make_pair(0, 0));
    ASSERT_EQ(chunk_to_local(33, 33), std::make_pair(1, 1));
    
    // Negative coordinates
    ASSERT_EQ(chunk_to_local(-1, -1), std::make_pair(31, 31));
    ASSERT_EQ(chunk_to_local(-32, -32), std::make_pair(0, 0));
    ASSERT_EQ(chunk_to_local(-33, -33), std::make_pair(31, 31));
}

TEST(RegionCoords, RegionFilename)
{
    ASSERT_EQ(region_filename(0, 0), "r.0.0.mca");
    ASSERT_EQ(region_filename(1, -2), "r.1.-2.mca");
    ASSERT_EQ(region_filename(-5, 3), "r.-5.3.mca");
}

TEST(RegionCoords, ChunkIndex)
{
    // First row
    ASSERT_EQ(Region::chunk_index(0, 0), 0);
    ASSERT_EQ(Region::chunk_index(1, 0), 1);
    ASSERT_EQ(Region::chunk_index(31, 0), 31);
    
    // Second row
    ASSERT_EQ(Region::chunk_index(0, 1), 32);
    ASSERT_EQ(Region::chunk_index(1, 1), 33);
    
    // Last chunk
    ASSERT_EQ(Region::chunk_index(31, 31), 1023);
}

// ---- ChunkEntry Tests ----

TEST(ChunkEntry, DefaultConstruction)
{
    ChunkEntry entry;
    ASSERT_EQ(entry.offset, 0);
    ASSERT_EQ(entry.sector_count, 0);
    ASSERT_EQ(entry.timestamp, 0);
    ASSERT_FALSE(entry.exists());
    ASSERT_FALSE(entry.data.has_value());
}

TEST(ChunkEntry, ExistsCheck)
{
    ChunkEntry entry;
    entry.offset = 0;
    ASSERT_FALSE(entry.exists());
    
    entry.offset = 2;  // Valid offset starts at sector 2 (after header)
    ASSERT_TRUE(entry.exists());
}

// ---- Region Structure Tests ----

TEST(Region, DefaultConstruction)
{
    Region region;
    ASSERT_EQ(region.region_x, 0);
    ASSERT_EQ(region.region_z, 0);
    ASSERT_EQ(region.count_chunks(), 0);
    ASSERT_EQ(region.count_loaded(), 0);
}

TEST(Region, ChunkAccess)
{
    Region region;
    
    // Add a chunk
    region.chunks[Region::chunk_index(5, 10)].offset = 2;
    region.chunks[Region::chunk_index(5, 10)].data = nbt_node{compound{}};
    
    // Access by coordinates
    auto* chunk = region.get_chunk(5, 10);
    ASSERT_NE(chunk, nullptr);
    
    // Non-existent chunk
    auto* missing = region.get_chunk(0, 0);
    ASSERT_EQ(missing, nullptr);
    
    // Out of bounds
    auto* oob = region.get_chunk(-1, 0);
    ASSERT_EQ(oob, nullptr);
    oob = region.get_chunk(32, 0);
    ASSERT_EQ(oob, nullptr);
}

TEST(Region, CountChunks)
{
    Region region;
    
    ASSERT_EQ(region.count_chunks(), 0);
    
    region.chunks[0].offset = 2;
    region.chunks[100].offset = 5;
    region.chunks[500].offset = 10;
    
    ASSERT_EQ(region.count_chunks(), 3);
}

TEST(Region, CountLoaded)
{
    Region region;
    
    ASSERT_EQ(region.count_loaded(), 0);
    
    region.chunks[0].data = nbt_node{compound{}};
    region.chunks[100].data = nbt_node{compound{}};
    
    ASSERT_EQ(region.count_loaded(), 2);
}

TEST(Region, GetEntry)
{
    Region region;
    
    region.chunks[Region::chunk_index(3, 7)].offset = 15;
    region.chunks[Region::chunk_index(3, 7)].sector_count = 3;
    region.chunks[Region::chunk_index(3, 7)].timestamp = 1234567890;
    
    const auto& entry = region.get_entry(3, 7);
    ASSERT_EQ(entry.offset, 15);
    ASSERT_EQ(entry.sector_count, 3);
    ASSERT_EQ(entry.timestamp, 1234567890);
}

// ---- Coordinate System Integration Test ----

TEST(RegionIntegration, WorldToRegionToLocal)
{
    // Test that world coordinates correctly map through the system
    
    // World chunk (100, -50)
    int chunk_x = 100;
    int chunk_z = -50;
    
    auto [region_x, region_z] = chunk_to_region(chunk_x, chunk_z);
    auto [local_x, local_z] = chunk_to_local(chunk_x, chunk_z);
    
    // Verify region coordinates
    ASSERT_EQ(region_x, 3);   // 100 / 32 = 3
    ASSERT_EQ(region_z, -2);  // -50 / 32 = -2 (floor division)
    
    // Verify local coordinates
    ASSERT_EQ(local_x, 4);    // 100 % 32 = 4
    ASSERT_EQ(local_z, 14);   // (-50 % 32 + 32) % 32 = 14
    
    // Verify filename
    ASSERT_EQ(region_filename(region_x, region_z), "r.3.-2.mca");
    
    // Verify index calculation
    size_t index = Region::chunk_index(local_x, local_z);
    ASSERT_EQ(index, 14 * 32 + 4);  // z * 32 + x = 452
}

TEST(RegionIntegration, NegativeCoordinates)
{
    // Test corner cases with negative coordinates
    
    // Chunk at exactly -32, -32 (should be in region -1, -1 at local 0, 0)
    auto [r1x, r1z] = chunk_to_region(-32, -32);
    auto [l1x, l1z] = chunk_to_local(-32, -32);
    ASSERT_EQ(r1x, -1);
    ASSERT_EQ(r1z, -1);
    ASSERT_EQ(l1x, 0);
    ASSERT_EQ(l1z, 0);
    
    // Chunk at -1, -1 (should be in region -1, -1 at local 31, 31)
    auto [r2x, r2z] = chunk_to_region(-1, -1);
    auto [l2x, l2z] = chunk_to_local(-1, -1);
    ASSERT_EQ(r2x, -1);
    ASSERT_EQ(r2z, -1);
    ASSERT_EQ(l2x, 31);
    ASSERT_EQ(l2z, 31);
}

// ---- Compression Type Tests ----

TEST(Compression, CompressionTypeValues)
{
    ASSERT_EQ(static_cast<uint8_t>(CompressionType::GZIP), 1);
    ASSERT_EQ(static_cast<uint8_t>(CompressionType::ZLIB), 2);
    ASSERT_EQ(static_cast<uint8_t>(CompressionType::UNCOMPRESSED), 3);
    ASSERT_EQ(static_cast<uint8_t>(CompressionType::LZ4), 4);
    ASSERT_EQ(static_cast<uint8_t>(CompressionType::CUSTOM), 127);
}

