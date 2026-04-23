# 🗜️ BellZip

### A Hybrid Parallel Compression Engine in Modern C++

> Lossless compression for any file type — designed to scale with CPU cores

---

## 📌 Overview

**BellZip** is a from-scratch lossless compression engine written in **C++17**, built to explore compression algorithms, systems programming, and concurrency.

Unlike traditional tools that wrap existing libraries, BellZip implements its own compression pipeline using:

- **Run-Length Encoding (RLE)** for redundancy reduction  
- **Huffman Coding** for entropy-based compression  
- **Block-level parallelism** for multi-core scalability  

It is designed to work on **any file type** — text, binaries, images, executables, and databases — at the byte level.

---
