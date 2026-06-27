#include <belialzip/core/bitstream.hpp>

namespace belialzip::core {

    // BIT WRITER IMPLEMENTATION

    void BitWriter::write_bit(bool bit) {
        if (bit) {
            // Glue the 1 into current_byte at the correct empty slot
            current_byte |= (1 << (7 - bit_count));
        }

        // Increment our slot counter
        bit_count++;

        // If our carton (byte) is exactly full (8 bits)
        if (bit_count == 8) {
            buffer.push_back(current_byte); // Ship it to the vector
            current_byte = 0;               // Get a fresh, empty byte (00000000)
            bit_count = 0;                  // Reset our slot counter to 0
        }
    }

    void BitWriter::write_bits(const std::string& code) {
        for (char c : code) {
            // Write each bit in the string to the BitWriter.
            write_bit(c == '1'); 
        }
    }

    int BitWriter::flush() {
        // If we have no bits in the current byte, the last byte was perfectly full.
        if (bit_count == 0) {
            return 8; 
        }

        // Save how many real bits are in here before we reset it.
        int valid_bits = bit_count; 
        
        // Ship the partially filled byte (empty slots stay 0 automatically). 
        buffer.push_back(current_byte); 
        
        // Resetting the writer state
        current_byte = 0;
        bit_count = 0;
        
        return valid_bits; 
    }

    const std::vector<uint8_t>& BitWriter::data() const {
        return buffer;
    }


    // BIT READER IMPLEMENTATION

    // Constructor with the proper Initializer List
    BitReader::BitReader(const uint8_t* data, size_t byte_count, int valid_bits_in_last_byte) 
        : data(data), 
          byte_count(byte_count), 
          valid_bits_in_last_byte(valid_bits_in_last_byte) {
        // Body is empty because the list above did the setup!
    }

    bool BitReader::is_end() const {
        // Check whether we have read past the total amount of available bytes
        if (current_byte_index >= byte_count) {
            return true;
        }

        // Check if we are currently reading the very last byte
        if (current_byte_index == byte_count - 1) {
            // Number of bits already read in this last byte
            int bits_read = 7 - current_bit_index;

            if (bits_read >= valid_bits_in_last_byte) {
                return true;
            }
        }

        return false;
    }

    int BitReader::read_bit() {
        // No more bits left! Return error state.
        if (is_end()) {
            return -1; 
        }

        // Extract current bit using shift and bitwise AND
        int bit = (data[current_byte_index] >> current_bit_index) & 1;
        
        // Move to the next bit
        current_bit_index--;

        // If we just finished reading the 0th bit...
        if (current_bit_index < 0) {
            current_byte_index++;  // Move to the next BYTE!
            current_bit_index = 7; // Reset to the biggest bit of the new byte
        }

        return bit;
    }

}