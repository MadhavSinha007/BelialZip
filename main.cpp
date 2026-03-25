#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <string>
#include<queue> //fifo data structure to store blocks in order of processing
#include<mutex> //locks for thread safety when accessing shared resources like the block queue
#include<condition_variable> //to synchronize threads and manage waiting for blocks to be processed

/* 
    SafeQueue is a thread-safe wrapper around st::queue.

    why we need this
    std::queue is not thread-safe by default
    if multiple threads push/pop simultaneously -> data corruption  will happen

    this class  solves that using
    mutex -> ensures  only one thread accesses queu at a time
    condition_variable -> makes threads wait efficently when queue is empty

*/

template <typename T>
class SafeQueue {
    private:
    std::queue<T> q; // underlying queue to store data
    std::mutex m; // mutex to protect access to queue
    std::condition_variable cv; // condition variable to manage waiting threads

    public:
    /* 
        push function

        adds data into queue safely
        notifies one waiting thread that new data is available
    */

    void push(T value){
        //lock scope starts here
        //lock_gurad automatically locks mutex on creation

        std::lock_guard<std::mutex> lock(m);
        //only one thread at a time can execute this blokc
        //critical section -> safe access to queue
        q.push(value); // add data to queue

    }

    //lock_guard goes aout of scope here -> mutex is automatically unlocked

    //notify one waiting thread that new data is svailable
    cv.notify_one();

/*
    pop function
    waits until queue has data
    removes and return front element safely
*/

T pop(){
    //unique_lock allows us to lock and unlock mutex manually
    std::unique_lock<std::mutex> lock(m);

    /*
        wait mechanism
        if queue is empty then thread sleeps
        when push() calls nofitfy_one thread wakes up
        lambda checks condition again to aviod spurious wakeups
    */
   cv.wait(lock, [&](){
    return !q.empty(); //only continue when queue has data
   });

   //at this point, queue is not empty, and thread own the lock again

   //get front elemtn
   T val = q.front();

   //remove it from queue
   q.pop();

   return val;


}

};


SafeQueue<Block> q1; //reader
SafeQueue<Block> q2; // writer

//reader block

void reader(const std::string& input){
    std::ifstream in(input, std::ios::binary);
    int id = 0;

    const size_t BLOCK_SIZE = 1024*1024; // 1Mb

    while(true)
}






// A block represents a chunk of the file in memory
struct Block {
    int id;                     // Block number (useful for debugging / ordering)
    std::vector<char> data;     // Actual file bytes stored in this block
    size_t size;                // Actual number of bytes read (important for last block)
};

// Function that copies file content using block-based reading
void copyFileContent(const std::string& input, const std::string& output) {

    // Open input file in binary mode (read raw bytes)
    std::ifstream in(input, std::ios::binary);

    // Open output file in binary mode (write raw bytes)
    std::ofstream out(output, std::ios::binary);

    


/*     File handling is done in binary mode to ensure that we read/write raw bytes without any transformation (like newline conversion on windows).
     This is crucial for accurate copying of all file types, especially non-text files like images, videos, or compressed archives, where any 
     alteration of byte data can corrup the file, by using binary mode we ensure that the file is copied exactly as it is, preserving its integrity
     regardless of its content. */

    // Check if input file opened successfully
    if (!in) {
        std::cerr << "Error: Cannot open input file\n";
        return;
    }

    // Check if output file opened successfully
    if (!out) {
        std::cerr << "Error: Cannot open output file\n";
        return;
    }

    // Vector to store all file blocks in memory
    std::vector<Block> blocks;

    int id = 0; // Block counter

    // Keep reading until end of file
    while (true) {

        Block block;
        block.id = id;

        // Allocate 1 MB buffer for reading file chunk
        block.data.resize(1024 * 1024);

        // Read up to 1 MB from file into buffer
        in.read(block.data.data(), block.data.size());

        // Get actual number of bytes read (important for last chunk)
        std::streamsize bytesRead = in.gcount();

        // If no bytes were read, file is fully consumed → exit loop
        if (bytesRead == 0)
            break;

        // Store actual size of valid data
        block.size = bytesRead;

        // Shrink buffer to actual data size (remove unused bytes)
        block.data.resize(bytesRead);

        // Save this block into vector
        blocks.push_back(block);

        // Move to next block
        id++;
    }

    // Write all stored blocks into output file in correct order
    for (const auto &block : blocks) {
        out.write(block.data.data(), block.size);
    }

    std::cout << "File copied successfully using block processing!\n";
}

int main(int argc, char* argv[]) {

    // Ensure user provides input and output file names
    if (argc != 3) {
        std::cout << "Usage: ./compressor input_file output_file\n";
        return 1;
    }

    // Create a thread to perform file copying
    std::thread t(copyFileContent, argv[1], argv[2]);

    // Wait for thread to finish before exiting program
    t.join();

    return 0;
}