#include <belialzip/core/lz77.hpp>
#include <algorithm>
#include <stdexcept>
#include <string>

namespace belialzip::core {

    // CONSTRUCTOR & RESET
    LZ77Compressor::LZ77Compressor(uint16_t lz_chain_depth)
        : chain_depth_(lz_chain_depth),
          head_(LZ_HASH_SIZE, 0xFFFFFFFF),  // 0xFFFFFFFF means "empty"
          prev_(LZ_WINDOW_SIZE, 0xFFFFFFFF) {}

    void LZ77Compressor::reset() {
        std::fill(head_.begin(), head_.end(), 0xFFFFFFFF);
        std::fill(prev_.begin(), prev_.end(), 0xFFFFFFFF);
    }


    // THE HASH CHAIN JUMPER (Find Match)
    std::pair<uint16_t, uint16_t> LZ77Compressor::find_longest_match(const uint8_t* data, size_t len, size_t pos) {
        // If we are too close to the end of the file to make a 3-byte match, abort!
        if (pos + LZ_MIN_MATCH > len) {
            return {0, 0};
        }

        // Get the hash of the current 3 bytes, and look up where we last saw it
        uint32_t h = hash3(data, pos);
        uint32_t candidate = head_[h];
        
        uint16_t best_len = 0;
        uint16_t best_dist = 0;
        int hops = 0;

        // Jump backward through the chain until we hit an empty spot (0xFFFFFFFF) or reach our max depth
        while (candidate != 0xFFFFFFFF && hops < chain_depth_) {
            
            // If the candidate is further back than our 32KB window, stop searching. It's too far!
            if (pos <= candidate || pos - candidate > LZ_WINDOW_SIZE) {
                break;
            }

            uint16_t dist = static_cast<uint16_t>(pos - candidate);
            
            // How many bytes are left in the file? (Cap it at our Max Match limit of 258)
            size_t max_len = std::min(static_cast<size_t>(LZ_MAX_MATCH), len - pos);
            uint16_t current_match_len = 0;

            // Byte-by-byte comparison! How many bytes actually match?
            while (current_match_len < max_len && data[candidate + current_match_len] == data[pos + current_match_len]) {
                current_match_len++;
            }

            // Is this the best match we've seen so far?
            if (current_match_len >= LZ_MIN_MATCH && current_match_len > best_len) {
                best_len = current_match_len;
                best_dist = dist;
                
                // If we found a perfect max-length match, stop searching!
                if (best_len == LZ_MAX_MATCH) break;
            }

            // Jump to the PREVIOUS time we saw this hash
            candidate = prev_[candidate % LZ_WINDOW_SIZE];
            hops++;
        }

        return {best_dist, best_len};
    }


    // ENCODE (Create Tokens)
    std::vector<LZToken> LZ77Compressor::encode(const uint8_t* data, size_t len) {
        reset();
        std::vector<LZToken> tokens;
        tokens.reserve(len); // Reserve memory for speed
        
        size_t pos = 0;
        
        while (pos < len) {
            // Ask our hash chain: "Have we seen this text recently?"
            auto [dist, match_len] = find_longest_match(data, len, pos);

            if (match_len >= LZ_MIN_MATCH) {
                // We found a profitable match! Create a token.
                tokens.push_back(LZToken::match(dist, match_len));
                
                // CRITICAL: We are skipping forward by `match_len` bytes.
                // We MUST update the hash table for every single byte we are skipping, 
                // otherwise our sliding window gets "blind spots".
                for (uint16_t k = 0; k < match_len && pos + k + 3 <= len; k++) {
                    uint32_t h2 = hash3(data, pos + k);
                    prev_[(pos + k) % LZ_WINDOW_SIZE] = head_[h2];
                    head_[h2] = static_cast<uint32_t>(pos + k);
                }
                pos += match_len;
            } 
            else {
                // No match. Just write it as a raw literal byte.
                tokens.push_back(LZToken::literal(data[pos]));
                
                // Update the hash table for this single byte
                if (pos + 3 <= len) {
                    uint32_t h2 = hash3(data, pos);
                    prev_[pos % LZ_WINDOW_SIZE] = head_[h2];
                    head_[h2] = static_cast<uint32_t>(pos);
                }
                pos++;
            }
        }
        return tokens;
    }


    // SERIALIZE (Tokens to Raw Bytes)
    std::vector<uint8_t> LZ77Compressor::serialize(const std::vector<LZToken>& tokens) {
        std::vector<uint8_t> out;
        out.reserve(tokens.size() * 2);
        
        size_t i = 0;
        while (i < tokens.size()) {
            // We group tokens in batches of 8. 
            // We reserve 1 byte to act as the "Flags" for the next 8 tokens.
            size_t flag_pos = out.size(); 
            out.push_back(0); // Placeholder for flags
            uint8_t flags = 0;
            
            size_t batch = std::min(tokens.size() - i, static_cast<size_t>(8));
            
            for (size_t k = 0; k < batch; k++) {
                const LZToken& t = tokens[i + k];
                
                if (t.is_literal) {
                    // Bit is 0. Just write the byte.
                    out.push_back(t.symbol);
                } else {
                    // Bit is 1. We have a match!
                    flags |= static_cast<uint8_t>(1u << (7 - k)); // Flip the bit
                    
                    // Write Distance (2 bytes, Little-Endian)
                    out.push_back(t.backref.distance & 0xFF);
                    out.push_back((t.backref.distance >> 8) & 0xFF);
                    
                    // Write Length (1 byte)
                    out.push_back(static_cast<uint8_t>(t.backref.length - LZ_MIN_MATCH));
                }
            }
            
            // Go back and fill in the real flags byte!
            out[flag_pos] = flags; 
            i += batch;
        }
        return out;
    }


    // DECODE (Raw Bytes back to File)
    std::vector<uint8_t> LZ77Compressor::decode(const uint8_t* data, size_t len) {
        std::vector<uint8_t> out;
        out.reserve(len * 3); // Decoding inflates the data
        
        size_t i = 0;
        while (i < len) {
            uint8_t flags = data[i++];
            
            for (int k = 7; k >= 0 && i < len; k--) {
                
                // Read the flag bit. Is it a 0 (literal) or 1 (match)?
                if (!((flags >> k) & 1)) {
                    // Literal! Just copy the byte.
                    out.push_back(data[i++]);
                } 
                else {
                    // Match! We need to read distance and length.
                    if (i + 2 >= len) {
                        throw std::runtime_error("Truncated LZ77 data at byte " + std::to_string(i));
                    }
                    
                    // Read Distance (2 bytes)
                    uint16_t dist = static_cast<uint16_t>(data[i]) | (static_cast<uint16_t>(data[i+1]) << 8);
                    
                    // Read Length (1 byte) + restore the MIN_MATCH
                    uint16_t match_len = static_cast<uint16_t>(data[i+2]) + LZ_MIN_MATCH; 
                    i += 3;
                    
                    if (dist == 0 || dist > out.size()) {
                        throw std::runtime_error("Invalid LZ77 distance: " + std::to_string(dist));
                    }
                    
                    // Go backward in our output and copy the bytes forward
                    size_t start = out.size() - dist;
                    for (uint16_t c = 0; c < match_len; c++) {
                        out.push_back(out[start + c]);
                    }
                }
            }
        }
        return out;
    }

}