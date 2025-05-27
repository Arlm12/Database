# Mini DBMS Engine

## 📦 Project Overview

This project is a lightweight implementation of a mini database management system (DBMS), designed to simulate how core components of real-world databases operate at a low level. It includes custom-built modules for storage management, buffer management, record handling, and indexing via B+ Trees.

This system is written in C and focuses on core DBMS internals—managing disk I/O, memory buffers, record layouts, and efficient data retrieval.

---

## 🧠 Modules Implemented

### 1. Storage Manager

* Handles creation, deletion, reading, and writing of database pages.
* Manages page-level I/O operations to simulate secondary storage behavior.

### 2. Buffer Manager

* Implements a page replacement strategy (e.g., FIFO or LRU).
* Controls in-memory buffer pool for fast access to disk pages.
* Tracks dirty pages and pin counts.

### 3. Record Manager

* Provides record-level operations such as insert, delete, update, and scan.
* Supports fixed-size records and page slot directory structures.

### 4. B+ Tree Index

* Implements an index structure for fast lookup and range queries.
* Includes insert and search functionality.
* Supports leaf and internal node management and page splits.

---

## 🔧 Compilation & Execution

### Prerequisites

* GCC or any standard C compiler
* POSIX-compliant environment (Linux/macOS recommended)

### Build

```bash
make all
```

### Run (example, replace with your driver code)

```bash
./storage_manager
```

---

## 🧪 Test Coverage

* Each module includes a corresponding test suite to validate its correctness.
* Test files are named as `test_assignX.c` where X = 1 to 4.
* Use `make test` or `./test_assignX` to run them individually.

---

## 🗂️ Directory Structure (should be created)

```
📁 MiniDBMS/
 ┣ 📁 assign1_storage_manager/
 ┣ 📁 assign2_buffer_manager/
 ┣ 📁 assign3_record_manager/
 ┗ 📁 assign4_b+tree_index/
```

---

## ✅ Learning Outcomes

* Deepened understanding of how database engines handle physical storage and memory.
* Gained experience in implementing data structures, managing pointers, and working with low-level I/O.
* Reinforced concepts in DBMS internals like page pinning, indexing, and page replacement.

---

## 👨‍💻 Author

Arunachalam B.

---

## 📄 License

MIT License (or specify your own)
