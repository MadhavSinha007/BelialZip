#include <belialzip/core/huffman.hpp>
#include <queue>
#include <algorithm>
#include <stdexcept>

namespace belialzip::core {

    std::array<uint32_t, 256> count_frequencies(const uint8_t* data, size_t size) {

        // array to hold the frequency (256 counters)
        std::array<uint32_t, 256> frequencies = {0};

        // counting how many times each byte appears
        for (size_t i = 0; i < size; ++i) {
            frequencies[data[i]]++;
        }

        return frequencies;
    }

    std::unique_ptr<HuffmanNode> build_tree(const std::array<uint32_t, 256>& freqs) {

        // min-heap to sort raw pointers by frequency
        auto cmp = [](const HuffmanNode* a, const HuffmanNode* b) {
            // logic for smallest frequency stays at the top
            return a->freq > b->freq;
        };

        std::priority_queue<
            HuffmanNode*,
            std::vector<HuffmanNode*>,
            decltype(cmp)
        > pq(cmp);

        // our storage vector to actually own the memory
        std::vector<std::unique_ptr<HuffmanNode>> storage;

        // leaf node for every char that appears
        for (int sym = 0; sym < 256; sym++) {

            // skip bytes that didn't appear
            if (freqs[sym] == 0)
                continue;

            storage.push_back(std::make_unique<HuffmanNode>(sym, freqs[sym]));

            // get the raw pointer for the queue
            pq.push(storage.back().get());
        }

        // empty file
        if (pq.empty())
            return nullptr;

        // file with only one unique char
        // add dummy right child so the tree doesn't crash later
        if (pq.size() == 1) {

            auto real_leaf = std::move(storage[0]);
            auto dummy = std::make_unique<HuffmanNode>(-2, 0);

            uint32_t root_freq = real_leaf->freq;

            return std::make_unique<HuffmanNode>(
                root_freq,
                std::move(real_leaf),
                std::move(dummy)
            );
        }

        // helper function to take a raw pointer, searches our storage, and hands over ownership
        auto take_ownership = [&](HuffmanNode* raw) -> std::unique_ptr<HuffmanNode> {

            for (auto& up : storage) {
                if (up.get() == raw)
                    return std::move(up);
            }

            return nullptr;
        };

        // keep combining the two smallest nodes until only one remains
        while (pq.size() > 1) {

            // smallest node
            HuffmanNode* first_raw = pq.top();
            pq.pop();

            // second smallest
            HuffmanNode* second_raw = pq.top();
            pq.pop();

            // taking ownership
            auto left = take_ownership(first_raw);
            auto right = take_ownership(second_raw);

            // combine freq
            uint32_t combined_freq = left->freq + right->freq;

            // parent node
            auto parent = std::make_unique<HuffmanNode>(
                combined_freq,
                std::move(left),
                std::move(right)
            );

            // queue only needs the raw pointer
            pq.push(parent.get());

            // storage owns the memory
            storage.push_back(std::move(parent));
        }

        // return the final node (root of the Huffman tree)
        return take_ownership(pq.top());
    }

    void generate_codes(
        const HuffmanNode* node,
        const std::string& prefix,
        std::unordered_map<uint8_t, std::string>& codes
    ) {

        // if the node is nullptr stop
        if (node == nullptr) {
            return;
        }

        // if we reached a leaf node
        if (node->is_leaf()) {

            // ignore the dummy node (symbol = -2)
            if (node->symbol >= 0) {

                // tree has only one real symbol then assign "0" instead of an empty string
                codes[static_cast<uint8_t>(node->symbol)] =
                    prefix.empty() ? "0" : prefix;
            }

            return;
        }

        // recursive step go left and append 0
        generate_codes(
            node->left.get(),
            prefix + "0",
            codes
        );

        // recursive step go right and append 1
        generate_codes(
            node->right.get(),
            prefix + "1",
            codes
        );
    }

    //serializer
    std::vector<uint8_t> serialize_freq_table(const std::array<uint32_t, 256>& freqs) {
        //store in 2 bytes
        uint16_t count = 0;
        for( int i = 0; i< 256; i++){
            if(freqs[i] > 0){
                count++;
            }
        }

        std::vector<uint8_t> out;

        //write the count
        out.push_back(count & 0xFF);
        out.push_back((count >> 8) & 0xFF);
        
        //write each symbol and its freq
        for(int sym = 0; sym < 256; sym++){

            //skip symbos that don't appear
            if(freqs[sym] == 0){
                continue;
            }

            //write the symbol 
            out.push_back(static_cast<uint8_t>(sym));

            //write the freq
            out.push_back(freqs[sym] & 0xFF);
            out.push_back((freqs[sym] >> 8) & 0xFF);
            out.push_back((freqs[sym] >> 16) & 0xFF);
            out.push_back((freqs[sym] >> 24) & 0xFF);
        }

        return out;
    
    }
    
    std::pair<std::array<uint32_t, 256>, size_t> deserialize_freq_table(const uint8_t* data, size_t len) {
        //at least 2 bytes just to read the count
        if (len < 2) {
            throw std::runtime_error("Freq table too short");
        }
        
        // Read the count (glue the first 2 bytes together)
        uint16_t count = static_cast<uint16_t>(data[0]) | (static_cast<uint16_t>(data[1]) << 8);
        
        //Calculate exact bytes needed = 2 (for count) + count * 5 (1 byte for symbol, 4 for freq)
        size_t needed = 2 + static_cast<size_t>(count) * 5;
        if (len < needed) {
            throw std::runtime_error("Freq table truncated");
        }
        
        std::array<uint32_t, 256> freqs = {0};
        
        //Read each entry
        for (uint16_t i = 0; i < count; i++) {
            // Calculate starting offset for this entry
            size_t off = 2 + (i * 5);
            
            uint8_t sym = data[off];
            
            uint32_t f = 0;
            f |= static_cast<uint32_t>(data[off+1]);
            f |= static_cast<uint32_t>(data[off+2]) << 8;
            f |= static_cast<uint32_t>(data[off+3]) << 16;
            f |= static_cast<uint32_t>(data[off+4]) << 24;
            
            freqs[sym] = f;
        }
        
        // return both the array and the number of bytes consumed
        return {freqs, needed};
    }
}