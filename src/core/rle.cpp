#include <belialzip/core/rle.hpp>

#include <stdexcept>

namespace belialzip::core{
    std::vector<uint8_t> rle_encode(const uint8_t* data, size_t len){
        std::vector<uint8_t> out;

        //reserve memory so the vector dosen't have to resize constantly
        out.reserve(len);

        size_t i = 0;

        while(i < len){
            uint8_t sym = data[i];
            size_t run_start = i;

            //count the run 
            while( i < len && data[i] == sym && (i - run_start) < RLE_MAX_RUN){
                i++;
            }

            size_t run_len = i - run_start;

            if(sym == RLE_ESCAPE){
                for (size_t j = 0; j < run_len; j++) {
                    out.push_back(RLE_ESCAPE);
                    out.push_back(RLE_ESCAPE);
                    out.push_back(1);
                }
            }
            else if (run_len >= 4){
                out.push_back(RLE_ESCAPE);
                out.push_back(sym);
                out.push_back(static_cast<uint8_t>(run_len));
            }
            else{
                //uprofitable run 
                for(size_t j = 0; j < run_len; j++){
                    out.push_back(sym);
                }
            }
            
        }

    return out;
    }

    //rle decode function
    std::vector<uint8_t> rle_decode(const uint8_t* data, size_t len){
        std::vector<uint8_t> out;


        out.reserve(len *3);
        size_t i = 0;
        while(i < len){
            if(data[i] == RLE_ESCAPE){
                if(i + 2 >= len){
                    throw std::runtime_error("Corrupt RLE Data");
                }

                //extract sumbo and count
                uint8_t sym = data[i + 1];
                uint8_t count = data[i + 2];

                //write the symbo count time
                for(uint8_t j = 0; j < count; j++){
                    out.push_back(sym);
                }

                i += 3; 
        }

        else{
            //normal byte
            out.push_back(data[i]);
            i++;
        }

    }

            return out;

}

}