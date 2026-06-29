#pragma once

#include <cstdint>
#include <array>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>

namespace belialzip::core
{
   /*
     HuffmanNode
     Leaf:     symbol >= 0 (byte value 0-255), left/right are nullptr
     Internal: symbol == -1, left/right own child subtrees
     */
    struct HuffmanNode {
        int symbol;
        uint32_t freq;
        std::unique_ptr<HuffmanNode> left;
        std::unique_ptr<HuffmanNode> right;

        // Constructor for a Leaf Node (has a symbol)
        HuffmanNode(int sym, uint32_t f) : symbol(sym), freq(f) {}

        // Constructor for an Internal Node (no symbol, joins two children)
        HuffmanNode(uint32_t f, std::unique_ptr<HuffmanNode> l, std::unique_ptr<HuffmanNode> r)
            : symbol(-1), freq(f), left(std::move(l)), right(std::move(r)) {}

        bool is_leaf() const { return symbol != -1; }
    };

    //count frequency of every byte(0 - 255)
    std::array<uint32_t, 256> count_frequencies(const uint8_t* data, size_t size);

    //building huffman tree from the frequency table
    std::unique_ptr<HuffmanNode> build_tree(const std::array<uint32_t, 256>& frequencies);

    //generate huffman codes
    void generate_codes(const HuffmanNode* node, const std::string& prefix, std::unordered_map<uint8_t, std::string>& codes);

    // Serialize frequency table: [count: 2 bytes] + count * [sym: 1 byte, freq: 4 bytes]
    std::vector<uint8_t> serialize_freq_table(const std::array<uint32_t, 256>& freqs);

    // Deserialize frequency table. Returns {table, bytes_consumed}.
    std::pair<std::array<uint32_t, 256>, size_t> deserialize_freq_table(const uint8_t* data, size_t len);
    
}
