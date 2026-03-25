#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <string>
#include <queue> 
#include <mutex> 
#include <condition_variable>
#include <unordered_map>

/*
    Block structure

    This is the core unit of data in our system.
    Each block represents a chunk of the file.
*/
struct Block {
    int id;                      // unique block ID (used to maintain order)
    std::vector<char> data;      // actual data
    size_t size;                 // size of valid data
};

   // huffman sructure
   struct Node
   {
    /* data */
    char ch;
    int freq;
    Node *left, *right;

    //constructor
    Node(char c, int f): ch(c), freq(f), left(nullptr), right(nullptr);
   };
   


/* 
    SafeQueue is a thread-safe wrapper around std::queue.

    WHY WE NEED THIS:
    std::queue is NOT thread-safe.
    If multiple threads push/pop at the same time → data corruption.

    This class solves that using:
    - mutex → ensures only one thread accesses queue at a time
    - condition_variable → makes threads wait efficiently when queue is empty
*/

template <typename T>
class SafeQueue {
private:
    std::queue<T> q;                 
    std::mutex m;                    
    std::condition_variable cv;      

public:

    /*
        PUSH FUNCTION

        Adds data into queue safely.
        Notifies one waiting thread that new data is available.
    */
    void push(const T& value) {
        {
            std::lock_guard<std::mutex> lock(m);  // lock acquired
            q.push(value);                        // critical section
        }                                         // lock released automatically

        cv.notify_one(); // wake up one waiting thread
    }


    /*
        POP FUNCTION

        Waits until queue has data.
        Removes and returns front element safely.
    */
    T pop() {
        std::unique_lock<std::mutex> lock(m);

        /*
            WAIT MECHANISM

            If queue is empty → thread sleeps
            When push() calls notify → thread wakes up

            Lambda prevents spurious wakeups
        */
        cv.wait(lock, [&]() {
            return !q.empty();
        });

        T val = q.front(); // get front element
        q.pop();           // remove it

        return val;
    }
};


/*
    GLOBAL QUEUES

    q1: Reader → Worker
    q2: Worker → Writer
*/
SafeQueue<Block> q1;
SafeQueue<Block> q2;


/*
    READER FUNCTION

    Reads file in chunks (blocks)
    Pushes blocks into q1
*/
void reader(const std::string& input) {

    std::ifstream in(input, std::ios::binary);

    if (!in) {
        std::cerr << "Error: Cannot open input file\n";
        return;
    }

    int id = 0;
    const size_t BLOCK_SIZE = 1024 * 1024; // 1MB blocks

    while (true) {

        Block block;
        block.id = id++;
        block.data.resize(BLOCK_SIZE);

        // read data into buffer
        in.read(block.data.data(), BLOCK_SIZE);

        // actual bytes read
        std::streamsize bytesRead = in.gcount();

        if (bytesRead <= 0) break;

        block.size = bytesRead;

        // resize to actual size
        block.data.resize(bytesRead);

        // send to worker
        q1.push(block);
    }

    // send end signal
    q1.push(Block{-1, {}, 0});
}


/*
    WORKER FUNCTION

    Takes blocks from q1
    (currently does nothing → later compression will be added)
    Pushes blocks to q2
*/
void worker() {
    while (true) {

        Block b = q1.pop();

        // check for end signal
        if (b.id == -1) {
            q2.push(b);
            break;
        }

        // NO PROCESSING (yet)
        // future: compression happens here

        q2.push(b);
    }
}


/*
    WRITER FUNCTION

    Takes processed blocks from q2
    Writes them to output file
*/
void writer(const std::string& output) {

    std::ofstream out(output, std::ios::binary);

    if (!out) {
        std::cerr << "Error: Cannot open output file\n";
        return;
    }

    while (true) {

        Block b = q2.pop();

        // check end signal
        if (b.id == -1) break;

        // write block data
        out.write(b.data.data(), b.size);
    }
}


/*
    MAIN FUNCTION

    Starts 3 threads:
    - reader
    - worker
    - writer
*/
int main(int argc, char* argv[]) {

    if (argc != 3) {
        std::cout << "Usage: ./compressor input output\n";
        return 1;
    }

    std::thread t1(reader, argv[1]);   // reader thread
    std::thread t2(worker);            // worker thread
    std::thread t3(writer, argv[2]);   // writer thread

    t1.join();
    t2.join();
    t3.join();

    return 0;
}