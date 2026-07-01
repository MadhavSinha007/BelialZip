# 1. Create an executable program from our test file
add_executable(test_header test_header.cpp)

# 2. Link our BelialZip library to this test program so it knows what "FileHeader" is!
target_link_libraries(test_header PRIVATE belialzip_lib)

# 3. Register it as an official test
add_test(NAME test_header COMMAND test_header)

#same as above for the bitsream test
add_executable(test_bitstream test_bitstream.cpp)
target_link_libraries(test_bitstream PRIVATE belialzip_lib)
add_test(NAME test_bitstream COMMAND test_bitstream)

#for huffman
add_executable(test_huffman test_huffman.cpp)
target_link_libraries(test_huffman PRIVATE belialzip_lib)
add_test(NAME test_huffman COMMAND test_huffman)

    
#for rle
add_executable(test_rle test_rle.cpp)
target_link_libraries(test_rle PRIVATE belialzip_lib)
add_test(NAME test_rle COMMAND test_rle)

#for lz77
add_executable(test_lz77 test_lz77.cpp)
target_link_libraries(test_lz77 PRIVATE belialzip_lib)
add_test(NAME test_lz77 COMMAND test_lz77)