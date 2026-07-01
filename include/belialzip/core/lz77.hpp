#pragma once

#include <cstdint>
#include <vector>
#include <cstddef>

namespace belialzip::core { 

    //anything shorter not worth it
    constexpr uint16_t LZ_MIN_MATCH = 3;
    
    //standard zip maximum match
    constexpr uint16_t LZ_MAX_MATCH = 258;
    
    //32kb sliding window
    constexpr uint32_t LZ_WINDOW_SIZE = 32768;

    constexpr uint16_t LZ_HASH_BITS = 15;
    //32768 slots
    constexpr uint32_t LZ_HASH_SIZE = 1u << LZ_HASH_BITS;

    //lztoken struct
    struct  LZToken
    {
        bool is_literal;
        union
        {
            uint8_t symbol;
            struct
            {
                uint16_t distance; uint16_t length;
            } backref;
        };

        //static helpers to easily create token
        static LZToken literal(uint8_t sym){
            LZToken t; t.is_literal = true; t.symbol = sym; return t;
        }
        static LZToken match(uint16_t dist, uint16_t len){
            LZToken t; t.is_literal = false;
            t.backref.distance = dist; t.backref.length = len; return t;
        }  
    };

    class LZ77Compressor {
        public:

        //constructor to set how deep we want to search the chain!
        explicit LZ77Compressor(uint16_t lz_chain_depth = 32);


        //return a list of tokens representing the compressed file
        std::vector<LZToken> encode(const uint8_t* data, size_t len);

        //turns those token into raw bytes for the hard drive
        std::vector<uint8_t> serialize(const std::vector<LZToken>& tokens);

        // Reads raw bytes and recreates the original file
        static std::vector<uint8_t> decode(const uint8_t* data, size_t len);

        private:
        uint16_t chain_depth_;

        //we use 0XFFFFFFFF to represent empty or end of chain
        std::vector<uint32_t> head_;
        std::vector<uint32_t> prev_;
        

        //helper to clear the arrays beore a new file
        void reset();

        //super-fast hashing math formula for 3 bytes
        static uint32_t hash3(const uint8_t* data, size_t pos){
            uint32_t h = data[pos];
            h = (h << 5) ^ data[pos+1];
            h = (h << 5) ^ data[pos+2];
            return (h * 2654435761u) >> (32 - LZ_HASH_BITS);
        }

        // The function that will actually jump through the chain to find a match
        std::pair<uint16_t, uint16_t> find_longest_match(const uint8_t* data, size_t len, size_t pos);
    };
    
}
