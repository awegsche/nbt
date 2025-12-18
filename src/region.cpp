#include "region.h"
#include "common.h"
#include <fstream>
#include <cstring>
#include <stdexcept>

namespace nbt {

// Internal helper to decompress chunk data
static std::vector<char> decompress_chunk(
    const char* compressed_data,
    size_t compressed_size,
    CompressionType compression)
{
    std::vector<char> result;
    
    switch (compression) {
    case CompressionType::ZLIB: {
        // Zlib decompression
        constexpr size_t INITIAL_SIZE = 1 << 18;  // 256 KiB initial buffer
        result.resize(INITIAL_SIZE);
        
        z_stream strm{};
        strm.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(compressed_data));
        strm.avail_in = static_cast<uInt>(compressed_size);
        strm.next_out = reinterpret_cast<Bytef*>(result.data());
        strm.avail_out = static_cast<uInt>(result.size());
        
        if (inflateInit(&strm) != Z_OK) {
            throw std::runtime_error("Failed to initialize zlib decompression");
        }
        
        int ret;
        size_t total_out = 0;
        
        do {
            ret = inflate(&strm, Z_NO_FLUSH);
            if (ret == Z_STREAM_ERROR || ret == Z_DATA_ERROR || ret == Z_MEM_ERROR) {
                inflateEnd(&strm);
                throw std::runtime_error("Zlib decompression error");
            }
            
            total_out = strm.total_out;
            
            if (ret != Z_STREAM_END && strm.avail_out == 0) {
                // Need more output space
                size_t old_size = result.size();
                result.resize(old_size * 2);
                strm.next_out = reinterpret_cast<Bytef*>(result.data() + old_size);
                strm.avail_out = static_cast<uInt>(old_size);
            }
        } while (ret != Z_STREAM_END);
        
        inflateEnd(&strm);
        result.resize(total_out);
        break;
    }
    
    case CompressionType::GZIP: {
        // Gzip decompression (zlib with gzip header)
        constexpr size_t INITIAL_SIZE = 1 << 18;
        result.resize(INITIAL_SIZE);
        
        z_stream strm{};
        strm.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(compressed_data));
        strm.avail_in = static_cast<uInt>(compressed_size);
        strm.next_out = reinterpret_cast<Bytef*>(result.data());
        strm.avail_out = static_cast<uInt>(result.size());
        
        // 15 + 16 enables gzip decoding
        if (inflateInit2(&strm, 15 + 16) != Z_OK) {
            throw std::runtime_error("Failed to initialize gzip decompression");
        }
        
        int ret;
        size_t total_out = 0;
        
        do {
            ret = inflate(&strm, Z_NO_FLUSH);
            if (ret == Z_STREAM_ERROR || ret == Z_DATA_ERROR || ret == Z_MEM_ERROR) {
                inflateEnd(&strm);
                throw std::runtime_error("Gzip decompression error");
            }
            
            total_out = strm.total_out;
            
            if (ret != Z_STREAM_END && strm.avail_out == 0) {
                size_t old_size = result.size();
                result.resize(old_size * 2);
                strm.next_out = reinterpret_cast<Bytef*>(result.data() + old_size);
                strm.avail_out = static_cast<uInt>(old_size);
            }
        } while (ret != Z_STREAM_END);
        
        inflateEnd(&strm);
        result.resize(total_out);
        break;
    }
    
    case CompressionType::UNCOMPRESSED: {
        result.assign(compressed_data, compressed_data + compressed_size);
        break;
    }
    
    case CompressionType::LZ4: {
        // LZ4 support would require the lz4 library
        // For now, throw an error indicating it's not supported
        throw std::runtime_error("LZ4 compression is not yet supported");
    }
    
    case CompressionType::CUSTOM: {
        throw std::runtime_error("Custom compression is not supported");
    }
    }
    
    return result;
}

Region load_region_header(const std::string& filename)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open region file: " + filename);
    }
    
    Region region;
    
    // Parse region coordinates from filename (r.X.Z.mca)
    auto base = std::filesystem::path(filename).stem().string();
    if (base.starts_with("r.")) {
        size_t first_dot = 2;
        size_t second_dot = base.find('.', first_dot);
        if (second_dot != std::string::npos) {
            try {
                region.region_x = std::stoi(base.substr(first_dot, second_dot - first_dot));
                region.region_z = std::stoi(base.substr(second_dot + 1));
            } catch (...) {
                // Ignore parsing errors, leave coordinates as 0
            }
        }
    }
    
    // Read location table (4096 bytes = 1024 entries * 4 bytes)
    std::array<char, SECTOR_SIZE> location_table;
    file.read(location_table.data(), SECTOR_SIZE);
    if (!file) {
        throw std::runtime_error("Failed to read location table");
    }
    
    // Read timestamp table (4096 bytes = 1024 entries * 4 bytes)
    std::array<char, SECTOR_SIZE> timestamp_table;
    file.read(timestamp_table.data(), SECTOR_SIZE);
    if (!file) {
        throw std::runtime_error("Failed to read timestamp table");
    }
    
    // Parse tables
    for (size_t i = 0; i < CHUNKS_PER_REGION; i++) {
        const char* loc_ptr = location_table.data() + i * 4;
        const char* ts_ptr = timestamp_table.data() + i * 4;
        
        // Location: 3 bytes offset (big-endian) + 1 byte sector count
        uint32_t location = 0;
        location |= static_cast<uint32_t>(static_cast<uint8_t>(loc_ptr[0])) << 16;
        location |= static_cast<uint32_t>(static_cast<uint8_t>(loc_ptr[1])) << 8;
        location |= static_cast<uint32_t>(static_cast<uint8_t>(loc_ptr[2]));
        
        region.chunks[i].offset = location;
        region.chunks[i].sector_count = static_cast<uint8_t>(loc_ptr[3]);
        
        // Timestamp: 4 bytes big-endian
        uint32_t timestamp = 0;
        std::memcpy(&timestamp, ts_ptr, 4);
        region.chunks[i].timestamp = from_big_endian(timestamp);
    }
    
    return region;
}

Region load_region(const std::string& filename)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open region file: " + filename);
    }
    
    // Get file size
    file.seekg(0, std::ios::end);
    size_t file_size = static_cast<size_t>(file.tellg());
    file.seekg(0, std::ios::beg);
    
    // Read entire file
    std::vector<char> file_data(file_size);
    file.read(file_data.data(), file_size);
    if (!file) {
        throw std::runtime_error("Failed to read region file");
    }
    
    Region region;
    
    // Parse region coordinates from filename
    auto base = std::filesystem::path(filename).stem().string();
    if (base.starts_with("r.")) {
        size_t first_dot = 2;
        size_t second_dot = base.find('.', first_dot);
        if (second_dot != std::string::npos) {
            try {
                region.region_x = std::stoi(base.substr(first_dot, second_dot - first_dot));
                region.region_z = std::stoi(base.substr(second_dot + 1));
            } catch (...) {
                // Ignore parsing errors
            }
        }
    }
    
    // Parse location and timestamp tables
    for (size_t i = 0; i < CHUNKS_PER_REGION; i++) {
        const char* loc_ptr = file_data.data() + i * 4;
        const char* ts_ptr = file_data.data() + SECTOR_SIZE + i * 4;
        
        uint32_t location = 0;
        location |= static_cast<uint32_t>(static_cast<uint8_t>(loc_ptr[0])) << 16;
        location |= static_cast<uint32_t>(static_cast<uint8_t>(loc_ptr[1])) << 8;
        location |= static_cast<uint32_t>(static_cast<uint8_t>(loc_ptr[2]));
        
        region.chunks[i].offset = location;
        region.chunks[i].sector_count = static_cast<uint8_t>(loc_ptr[3]);
        
        uint32_t timestamp = 0;
        std::memcpy(&timestamp, ts_ptr, 4);
        region.chunks[i].timestamp = from_big_endian(timestamp);
    }
    
    // Load all existing chunks
    for (size_t i = 0; i < CHUNKS_PER_REGION; i++) {
        auto& entry = region.chunks[i];
        if (!entry.exists()) continue;
        
        size_t chunk_offset = static_cast<size_t>(entry.offset) * SECTOR_SIZE;
        if (chunk_offset + 5 > file_size) {
            warn("Chunk {} has invalid offset, skipping", i);
            continue;
        }
        
        const char* chunk_ptr = file_data.data() + chunk_offset;
        
        // Read chunk header: 4 bytes length + 1 byte compression type
        uint32_t chunk_length = 0;
        std::memcpy(&chunk_length, chunk_ptr, 4);
        chunk_length = from_big_endian(chunk_length);
        chunk_ptr += 4;
        
        auto compression = static_cast<CompressionType>(*reinterpret_cast<const uint8_t*>(chunk_ptr));
        entry.compression = compression;
        chunk_ptr++;
        
        // Validate length
        if (chunk_offset + 5 + chunk_length - 1 > file_size) {
            warn("Chunk {} has invalid length, skipping", i);
            continue;
        }
        
        try {
            // Decompress chunk data (length includes compression byte, so subtract 1)
            auto decompressed = decompress_chunk(chunk_ptr, chunk_length - 1, compression);
            
            // Parse NBT
            const char* nbt_ptr = decompressed.data();
            entry.data = read_node(nbt_ptr);
        } catch (const std::exception& e) {
            warn("Failed to decompress/parse chunk {}: {}", i, e.what());
        }
    }
    
    return region;
}

std::optional<nbt_node> load_chunk(const std::string& filename, int local_x, int local_z)
{
    if (local_x < 0 || local_x >= REGION_DIMENSION ||
        local_z < 0 || local_z >= REGION_DIMENSION) {
        return std::nullopt;
    }
    
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        return std::nullopt;
    }
    
    // Read just the location entry we need
    size_t index = Region::chunk_index(local_x, local_z);
    file.seekg(index * 4);
    
    char loc_data[4];
    file.read(loc_data, 4);
    if (!file) return std::nullopt;
    
    uint32_t offset = 0;
    offset |= static_cast<uint32_t>(static_cast<uint8_t>(loc_data[0])) << 16;
    offset |= static_cast<uint32_t>(static_cast<uint8_t>(loc_data[1])) << 8;
    offset |= static_cast<uint32_t>(static_cast<uint8_t>(loc_data[2]));
    
    if (offset == 0) return std::nullopt;
    
    // Seek to chunk data
    file.seekg(static_cast<size_t>(offset) * SECTOR_SIZE);
    
    // Read chunk header
    char header[5];
    file.read(header, 5);
    if (!file) return std::nullopt;
    
    uint32_t chunk_length = 0;
    std::memcpy(&chunk_length, header, 4);
    chunk_length = from_big_endian(chunk_length);
    
    auto compression = static_cast<CompressionType>(static_cast<uint8_t>(header[4]));
    
    // Read compressed data
    std::vector<char> compressed(chunk_length - 1);
    file.read(compressed.data(), chunk_length - 1);
    if (!file) return std::nullopt;
    
    // Decompress and parse
    auto decompressed = decompress_chunk(compressed.data(), compressed.size(), compression);
    const char* nbt_ptr = decompressed.data();
    return read_node(nbt_ptr);
}

std::optional<nbt_node> load_chunk_from_world(
    const std::filesystem::path& region_folder,
    int chunk_x,
    int chunk_z)
{
    auto [region_x, region_z] = chunk_to_region(chunk_x, chunk_z);
    auto [local_x, local_z] = chunk_to_local(chunk_x, chunk_z);
    
    auto region_path = region_folder / region_filename(region_x, region_z);
    return load_chunk(region_path.string(), local_x, local_z);
}

// Legacy function for backwards compatibility
std::vector<nbt_node> load_region_legacy(const char* filename)
{
    auto region = load_region(filename);
    
    std::vector<nbt_node> result;
    result.reserve(region.count_loaded());
    
    for (auto& entry : region.chunks) {
        if (entry.data.has_value()) {
            result.push_back(std::move(entry.data.value()));
        }
    }
    
    return result;
}

}  // namespace nbt
