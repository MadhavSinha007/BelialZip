#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <unordered_map>

// Block represents a chunk of file data
// We split large files into smaller chunks (blocks)
// Each block is processed independently in the pipeline
struct Block {
    int id;                     // used to maintain correct order
    std::vector<char> data;    // actual bytes of the file
    size_t size;               // valid data size
    int validBits; //for last byte to check how many valid bits are in last byte
};

// SafeQueue is a thread-safe wrapper around std::queue
// WHY we need this:
// - Multiple threads access the queue simultaneously
// - std::queue is NOT thread-safe
// - mutex ensures only one thread accesses it at a time
// - condition_variable allows threads to wait efficiently
template <typename T>
class SafeQueue {
private:
    std::queue<T> q;           // internal queue
    std::mutex m;              // mutex for locking
    std::condition_variable cv; // used for waiting/wakeup

public:

    // Push adds data into the queue safely
    // It locks the queue, inserts data, then notifies a waiting thread
    void push(const T& value) {
        {
            std::lock_guard<std::mutex> lock(m); // acquire lock
            q.push(value);                       // critical section
        }
        cv.notify_one(); // wake up one waiting thread
    }

    // Pop removes and returns data from the queue
    // If queue is empty → thread sleeps (efficient waiting)
    // When push() is called → thread wakes up
    T pop() {
        std::unique_lock<std::mutex> lock(m); // acquire lock

        cv.wait(lock, [&]() {
            return !q.empty(); // wait until queue has data
        });

        T val = q.front(); // read front element
        q.pop();           // remove it

        return val;        // return data
    }
};

// Global queues forming the pipeline
// q1: Reader → Worker
// q2: Worker → Writer
SafeQueue<Block> q1;
SafeQueue<Block> q2;

// HuffmanNode represents a node in the Huffman Tree
// Leaf nodes store actual characters
// Internal nodes store combined frequency
struct HuffmanNode {
    char ch;                  // character
    int freq;                 // frequency
    HuffmanNode* left;        // left child
    HuffmanNode* right;       // right child

    HuffmanNode(char c, int f) {
        ch = c;
        freq = f;
        left = right = nullptr;
    }
};

// Comparator for priority queue (min heap)
// Nodes with smaller frequency get higher priority
struct Compare {
    bool operator()(HuffmanNode* a, HuffmanNode* b) {
        return a->freq > b->freq;
    }
};

// Builds Huffman Tree using greedy approach
// Steps:
// 1. Insert all characters into min heap
// 2. Take two smallest nodes
// 3. Merge them into a new node
// 4. Repeat until one node remains (root)
HuffmanNode* buildTree(std::unordered_map<char, int>& freq) {

    std::priority_queue<HuffmanNode*, std::vector<HuffmanNode*>, Compare> pq;

    for (auto& pair : freq) {
        pq.push(new HuffmanNode(pair.first, pair.second));
    }

    while (pq.size() > 1) {
        HuffmanNode* left = pq.top(); pq.pop();
        HuffmanNode* right = pq.top(); pq.pop();

        HuffmanNode* merged = new HuffmanNode('\0', left->freq + right->freq);
        merged->left = left;
        merged->right = right;

        pq.push(merged);
    }
    return pq.top(); // root of tree
}

// Generates Huffman codes by traversing the tree
// Left edge → add '0'
// Right edge → add '1'
// When we reach a leaf node → assign the code
void generateCodes(HuffmanNode* root,
                   std::string code,
                   std::unordered_map<char, std::string>& huffmanCode) {

    if (!root) return;

    if (!root->left && !root->right) {
        huffmanCode[root->ch] = code; // assign code to character
    }

    generateCodes(root->left, code + "0", huffmanCode);
    generateCodes(root->right, code + "1", huffmanCode);
}


//bitpacketFunction
//converts string of '0' ans '1' into bytes
// 0100001 = > 65 stored as 1 byte

std::vector<char> packBits(const std::string& bits){
    std::vector<char> packed;
    for(size_t i = 0; i<bits.size(); i += 8){
        char byte = 0;
        for(int j = 0; j<8; j++){
            byte <<= 1;

            if(i + j<bits.size() && bits[i+j] == '1'){
                byte |= 1;
            }
        }

        packed.push_back(byte);
    }

    return packed;
}




// Compress a block using Huffman encoding
// Steps:
// 1. Count frequency of each character
// 2. Build Huffman Tree
// 3. Generate codes
// 4. Replace characters with binary codes
Block compressBlock(Block& input) {

    std::unordered_map<char, int> freq;

    for (char c : input.data) {
        freq[c]++; // count frequency
    }

    HuffmanNode* root = buildTree(freq);

    std::unordered_map<char, std::string> huffmanCode;
    generateCodes(root, "", huffmanCode);

    //crete bit string
    std::string bitString;

    for(char c: input.data){
        bitString += huffmanCode[c];
    }


    //packing bits into bytes
    std::vector<char> packedData = packBits(bitString);

    Block output;
    output.id = input.id;
    output.data = packedData;
    output.size  = packedData.size();

    output.validBits = bitString.size() %8;
    if(output.validBits == 0) {
        output.validBits = 8;
    }

    return output;
}

// Reader thread reads file in chunks and pushes to q1
// It ensures streaming instead of loading entire file into memory
void reader(const std::string& input) {

    std::ifstream in(input, std::ios::binary);

    if (!in) {
        std::cerr << "Error: Cannot open input file\n";
        return;
    }

    int id = 0;
    const size_t BLOCK_SIZE = 1024 * 1024; // 1MB per block

    while (true) {
        Block block;
        block.id = id++;
        block.data.resize(BLOCK_SIZE);

        in.read(block.data.data(), BLOCK_SIZE);
        std::streamsize bytesRead = in.gcount();

        if (bytesRead <= 0) break;

        block.size = bytesRead;
        block.data.resize(bytesRead);

        q1.push(block);
    }

    // Send termination signal
    q1.push(Block{-1, {}, 0});
}

// Worker thread processes blocks
// Currently applies compression
void worker() {
    while (true) {
        Block b = q1.pop();

        if (b.id == -1) {
            q2.push(b);
            break;
        }

        Block compressed = compressBlock(b);
        q2.push(compressed);
    }
}

// Writer thread writes processed blocks to output file
// It runs continuously until termination signal is received
void writer(const std::string& output) {

    std::ofstream out(output, std::ios::binary);

    if (!out) {
        std::cerr << "Error: Cannot open output file\n";
        return;
    }

    while (true) {
        Block b = q2.pop();

        if (b.id == -1) break;

        out.write(b.data.data(), b.size);
    }
}

// Main function initializes pipeline threads
// Reader → Worker → Writer
// Each runs independently but communicates via queues
int main(int argc, char* argv[]) {

    if (argc != 3) {
        std::cout << "Usage: ./compressor input output\n";
        return 1;
    }

    std::thread t1(reader, argv[1]);   // start reader
    std::thread t2(worker);            // start worker
    std::thread t3(writer, argv[2]);   // start writer

    t1.join(); // wait for reader
    t2.join(); // wait for worker
    t3.join(); // wait for writer

    return 0;
}