#include <belialzip/core/bitstream.hpp>
#include <iostream>
#include <cassert>
#include <vector>
#include <string>

int main() {
    // 1. We will write the binary string "10110100" (which is the number 180)
    std::string test_bits = "10110100";
    
    belialzip::core::BitWriter writer;
    writer.write_bits(test_bits);
    
    // Flush the writer. Since we wrote exactly 8 bits, valid_bits should be 8.
    int valid = writer.flush();
    assert(valid == 8);

    // Get the packed bytes
    const auto& bytes = writer.data();
    assert(bytes.size() == 1);
    assert(bytes[0] == 180); // 10110100 in binary is 180 in decimal!

    // 2. Now let's read it back!
    belialzip::core::BitReader reader(bytes.data(), bytes.size(), valid);
    
    std::string read_back = "";
    while (!reader.is_end()) {
        int bit = reader.read_bit();
        if (bit == 1) read_back += "1";
        if (bit == 0) read_back += "0";
    }

    // 3. Validation
    assert(read_back == test_bits);
    assert(reader.read_bit() == -1); // Should safely return -1 when exhausted

    std::cout << "[PASS] BitStream write/read round-trip validated.\n";
    return 0;
}