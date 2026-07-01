#include <belialzip/core/rle.hpp>
#include <iostream>
#include <vector>
#include <cassert>
#include <string>

bool rle_roundtrip(const std::vector<uint8_t>& in, const std::string& name) {
    // Encode and Decode
    auto enc = belialzip::core::rle_encode(in.data(), in.size());
    auto dec = belialzip::core::rle_decode(enc.data(), enc.size());
    
    // Validate
    if (in != dec) {
        std::cout << "[FAIL] " << name << "\n";
        return false;
    }
    std::cout << "[PASS] " << name << " | Size: " << in.size() << " -> " << enc.size() << " bytes\n";
    return true;
}

int main() {
    bool all = true;
    
    // Test 1: Highly compressible (512 zeros)
    all &= rle_roundtrip(std::vector<uint8_t>(512, 0x00), "512 Zeros");
    
    // Test 2: Run of 3 (Should NOT compress - stays 3 bytes)
    all &= rle_roundtrip({0xAA, 0xAA, 0xAA}, "Run of 3");
    
    // Test 3: Run of 4 (Should compress - drops to 3 bytes)
    all &= rle_roundtrip({0xAA, 0xAA, 0xAA, 0xAA}, "Run of 4");
    
    // Test 4: The Fake Escape! (Should inflate slightly to protect the data)
    all &= rle_roundtrip({0xFE, 0xFE, 0xFE}, "Fake Escapes");
    
    // Test 5: Max Run (255 bytes)
    all &= rle_roundtrip(std::vector<uint8_t>(255, 0xBB), "Max Run (255)");
    
    // Test 6: Over Max Run (300 bytes - should split into 2 tokens)
    all &= rle_roundtrip(std::vector<uint8_t>(300, 0xCC), "Over Max Run (300)");

    if (all) {
        std::cout << "\n🎉 All RLE tests passed successfully!\n";
    }
    return all ? 0 : 1;
}