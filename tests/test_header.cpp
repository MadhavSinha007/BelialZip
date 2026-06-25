#include <belialzip/io/header.hpp>
#include <iostream>
#include <cassert>
#include <cstring>

int main() {
    // 1. Construct the initial state
    belialzip::io::FileHeader hdr;
    std::memcpy(hdr.magic, belialzip::io::MAGIC, 4);
    hdr.version = belialzip::io::FORMAT_VERSION;
    hdr.flags   = 0x03; 
    hdr.original_size = 1234567890ULL;
    hdr.block_size = 256 * 1024;
    hdr.num_blocks = 3;
    hdr.block_sizes = {1024, 2048, 512};

    // 2. The Transformation Leap
    auto bytes = belialzip::io::serialize_header(hdr);
    auto hdr2 = belialzip::io::parse_header(bytes.data(), bytes.size());

    // 3. Validation assertions
    assert(hdr2.original_size == 1234567890ULL);
    assert(hdr2.block_sizes[1] == 2048);
    assert(hdr2.num_blocks == 3);
    
    std::cout << "[PASS] Serialize/parse round-trip validated.\n";
    return 0;
}