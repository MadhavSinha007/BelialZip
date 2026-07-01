#pragma once

#include <vector>
#include <cstdint>
#include <cstddef>

namespace belialzip::core {

    // Escape value for RLE (0xFE is less common than 0xFF in typical binary data)
    constexpr uint8_t RLE_ESCAPE = 0xFE;

    // Max count for RLE (1 byte can hold up to 255)
    constexpr uint8_t RLE_MAX_RUN = 255;

    // Encoding function for RLE
    std::vector<uint8_t> rle_encode(const uint8_t* data, size_t len);

    // Function to decode RLE data
    std::vector<uint8_t> rle_decode(const uint8_t* data, size_t len);

}