#pragma once

#include <cstdint>
#include <vector>
#include <cstddef>

namespace belialzip::io {

    // A constant array for our exact magic bytes
    constexpr uint8_t MAGIC[4] = {'B', 'Z', 0x01, 0x00};
    constexpr uint8_t FORMAT_VERSION = 1;

    struct FileHeader {
        uint8_t magic[4];       // 4 bytes, array to prevent Endianness flipping
        uint8_t version;        // 1 byte, version of the file format
        uint8_t flags;          // 1 byte, switches for compression methods

        uint64_t original_size; // 8 bytes, strict 64-bit integer
        uint32_t block_size;    // 4 bytes, size of each uncompressed chunk (e.g., 256KB)
        uint32_t num_blocks;    // 4 bytes, how many chunks are in this file

        std::vector<uint64_t> block_sizes; // Dynamic, compressed size of every block
    };

    std::vector<uint8_t> serialize_header(const FileHeader& header);

    FileHeader parse_header(const uint8_t* data, size_t len);

}