#include <belialzip/io/header.hpp>
#include <cstring>

namespace belialzip::io {
    //chop a 32 bit integer into 4 bytes and pushes then to vector
    static void write_u32(std::vector<uint8_t>& buf, uint32_t v){
        for(int i = 0; i<4; i++){
            buf.push_back((v >> (i*8)) & 0xFF);
        }
    }

    //chpos a 64 bit integer into 8 bytes and pushes then to vector
    static void write_u64(std::vector<uint8_t>& buf, uint64_t v){
        for(int i = 0; i<8; i++){
            buf.push_back((v >> (i*8)) & 0xFF);
        }
    }



    std::vector<uint8_t> serialize_header(const FileHeader& hdr){
        std::vector<uint8_t> out_buffer;
        
        //writing magic bytes
        out_buffer.insert(
            out_buffer.end(),
            std::begin(hdr.magic),
            std::end(hdr.magic)
        );


        //writing version and flags
        out_buffer.push_back(hdr.version);
        out_buffer.push_back(hdr.flags);

        //writing fixed-size fields
        write_u64(out_buffer, hdr.original_size);
        write_u32(out_buffer, hdr.block_size);
        write_u32(out_buffer, hdr.num_blocks);


        //writing all compressed blocks sizes
        for(uint64_t block_size : hdr.block_sizes){
            write_u64(out_buffer, block_size);
        }

        return out_buffer;
    }
}