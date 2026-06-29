#include <belialzip/core/huffman.hpp>
#include <belialzip/core/bitstream.hpp>
#include <iostream>
#include <cassert>
#include <vector>
#include <string>

int main() {
    std::string test_data = "hello world! this is belialzip. hello again!";
    std::vector<uint8_t> input(test_data.begin(), test_data.end());

    std::cout << "Original size: " << input.size() << " bytes\n";

    // 1. Analyze and Build
    auto freqs = belialzip::core::count_frequencies(input.data(), input.size());
    auto tree  = belialzip::core::build_tree(freqs);
    
    std::unordered_map<uint8_t, std::string> codes;
    belialzip::core::generate_codes(tree.get(), "", codes);

    // 2. Compress the data using our BitWriter
    belialzip::core::BitWriter bw;
    for (uint8_t b : input) {
        bw.write_bits(codes.at(b));
    }
    int valid_bits = bw.flush();

    std::cout << "Compressed size: " << bw.data().size() << " bytes\n";
    std::cout << "Compression ratio: " << (float)bw.data().size() / input.size() * 100 << "%\n";

    // 3. Decompress the data
    belialzip::core::BitReader br(bw.data().data(), bw.data().size(), valid_bits);
    std::vector<uint8_t> output;
    
    // We start at the top of the tree
    const belialzip::core::HuffmanNode* current_node = tree.get();
    
    while (!br.is_end()) {
        int bit = br.read_bit();
        if (bit < 0) break; // End of stream
        
        // Navigate the tree
        current_node = (bit == 1) ? current_node->right.get() : current_node->left.get();
        
        // If we hit a leaf, we found the character!
        if (current_node->is_leaf()) {
            output.push_back(static_cast<uint8_t>(current_node->symbol));
            current_node = tree.get(); // Reset to top of the tree for the next character
        }
    }

    // 4. Validate!
    assert(output == input);
    
    std::cout << "[PASS] Huffman tree compression/decompression validated!\n";
    return 0;
}