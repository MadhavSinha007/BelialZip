#include <belialzip/io/header.hpp>
#include <cstring>
#include <stdexcept>

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

    //glueing 4 bytes back into a 32 bit integer
    static uint32_t read_u32(const uint8_t* p){
        return static_cast<uint32_t>(p[0])
            | (static_cast<uint32_t>(p[1]) << 8)
            | (static_cast<uint32_t>(p[2]) << 16)
            | (static_cast<uint32_t>(p[3]) << 24);  
    }

    //glueing 8 bytes back into 64 bit integer
    static uint64_t read_u64(const uint8_t* p){
        uint64_t v = 0;
        for(int i = 0; i<8; i++){
            v |= (static_cast<uint64_t>(p[i]) << (i*8));
        }
        return v;
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


    //parse_header
    FileHeader parse_header(const uint8_t* data, size_t len){

        if(len < 22){
            throw std::runtime_error("File too small");
        }

        FileHeader hdr;
        std::memcpy(hdr.magic, data, 4);
        hdr.version = data[4];
        hdr.flags = data[5];

        hdr.original_size = read_u64(data + 6);
        hdr.block_size = read_u32(data + 14);
        hdr.num_blocks = read_u32(data + 18);

        if(len < 22 + hdr.num_blocks*8){
            throw std::runtime_error("File truncated");
        }

        for(uint32_t i = 0; i < hdr.num_blocks; i++){
            hdr.block_sizes.push_back(read_u64(data + 22 + (i * 8)));
        }
        return hdr;
    }
}