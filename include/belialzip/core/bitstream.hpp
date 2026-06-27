#pragma once

#include <cstdint>
#include <vector>
#include <string>

namespace belialzip::core {

    class BitWriter {
    public:
        void write_bit(bool bit); // true = 1 and false = 0
        void write_bits(const std::string& code); // takes string like "1011" and writes each bit
        
        int flush(); // flushes the buffer to disk and returns the number of valid bits in the last byte
        
        const std::vector<uint8_t>& data() const; // returns the buffer

    private:
        std::vector<uint8_t> buffer; // holds finished bytes
        uint8_t current_byte = 0;    // current carton
        int bit_count = 0;           // how many bits are currently in the carton
    };

    class BitReader {
    public:
        // constructor
        BitReader(const uint8_t* data, size_t byte_count, int valid_bits_in_last_byte);
        
        // returns 0 or 1, or -1 if we ran out of data
        int read_bit();
        
        // returns true if we have read all valid bits
        bool is_end() const;

    private:
        const uint8_t* data;
        size_t byte_count;
        int valid_bits_in_last_byte;
        size_t current_byte_index = 0;
        int current_bit_index = 7; // starts from the most significant bit
    };

}