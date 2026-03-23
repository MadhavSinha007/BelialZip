#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <string>

// Function to copy file content from input → output
void copyFileContent(const std::string& input, const std::string& output) {

    // Open input file in binary mode (for reading)
    std::ifstream in(input, std::ios::binary);

    // Open output file in binary mode (for writing)
    std::ofstream out(output, std::ios::binary);

    // Check if files opened successfully
    if (!in) {
        std::cerr << "Error: Cannot open input file\n";
        return;
    }
    if (!out) {
        std::cerr << "Error: Cannot open output file\n";
        return;
    }

    // Create a buffer of size 1MB (1024 * 1024 bytes)
    std::vector<char> buffer(1024 * 1024);

    // Read file in chunks until EOF (end of file)
    while (in.read(buffer.data(), buffer.size()) || in.gcount() > 0) {

        // Get number of bytes actually read
        std::streamsize bytesRead = in.gcount();

        // Write those bytes into output file
        out.write(buffer.data(), bytesRead);
    }

    // Files automatically close when going out of scope
    std::cout << "File copied successfully!\n";
}

int main(int argc, char* argv[]) {

    // Check if user provided correct number of arguments
    // Expected: ./compressor input_file output_file
    if (argc != 3) {
        std::cout << "Usage: ./compressor input output\n";
        return 1;
    }

    // Create a separate thread to perform file copying
    std::thread t(copyFileContent, argv[1], argv[2]);

    // Wait for the thread to finish execution
    t.join();

    return 0;
}