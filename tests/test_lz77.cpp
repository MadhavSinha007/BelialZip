#include <belialzip/core/lz77.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <cassert>

bool lz77_roundtrip(const std::vector<uint8_t>& in, const std::string& name) {
    belialzip::core::LZ77Compressor lz;
    
    auto tokens = lz.encode(in.data(), in.size());
    auto ser = lz.serialize(tokens);
    auto dec = belialzip::core::LZ77Compressor::decode(ser.data(), ser.size());
    
    if (in != dec) {
        std::cout << "[FAIL] " << name << "\n";
        return false;
    }
    
    std::cout << "[PASS] " << name << " | " << in.size() << " -> " << ser.size() << " bytes\n";
    return true;
}

int main() {
    bool all = true;
    
    // 1. A highly repetitive string (LZ77's favorite food)
    std::string s;
    for (int i = 0; i < 100; i++) s += "hello world ";
    std::vector<uint8_t> repeated(s.begin(), s.end());
    
    all &= lz77_roundtrip(repeated, "Repeated 'hello world' x100");
    
    // 2. The Overlapping Run Extension trick!
    std::vector<uint8_t> run_ext(1000, 0xAA);
    all &= lz77_roundtrip(run_ext, "1000 * 0xAA (Run Extension)");

    if (all) std::cout << "\n🎉 LZ77 Compression validated!\n";
    return all ? 0 : 1;
}