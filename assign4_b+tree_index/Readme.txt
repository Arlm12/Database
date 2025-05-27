# CS 525: Advanced Database Organization Fall 2024
Assignment 4: B+ Tree

# Information about the team:
Group 22: Arunachalam Barathidasan(abarathidasan@hawk.iit.edu)
          Sejal Srivastav (ssrivastav@hawk.iit.edu)
          Prathiksha Shirsat (pshirsat@hawk.iit.edu)

---

## Overview

In this assignment, we developed a B+-tree index, structured to store and manage data efficiently in a
 sorted order using a buffer manager to optimize memory usage. Key functions include `insertKey()` 
 to add entries, `findKey()` to search for specific keys, and `deleteKey()` to remove them, all while 
 maintaining the tree’s balance. Additionally, `openTreeScan()` and `nextEntry()` allow for sequentially
  scanning entries in sorted order. The B+-tree is built to handle edge cases like duplicate keys and 
  underflows, ensuring robust and reliable performance.

## Table of Contents
1. Contents of the folder
2. Compilation and Execution
3. Functional Description
    - Index Initialization and Shutdown
    - B+-Tree Index Management
    - Accessing B+-Tree Information
    - Key Operations
    - Tree Scanning

1. Contents of the folder
Below is the list of files that are present in the Assignment folder.
.c files:
btree_mgr.c, buffer_mgr_stat.c, buffer_mgr.c, dberror.c, expr.c, h_needed.c, record_mgr.c, rm_serializer.c, storage_mgr.c, test_assign4_1.c and test_expr.c

.h or header files:
btree_mgr.h, buffer_mgr_stat.h, buffer_mgr.h, dberror.h, dt.h, expr.h, record_mgr.h, storage_mgr.h, tables.h, test_helper.h 

2. Compilation and Execution

Compilation Steps:
1. Intially execute "make clean" command to remove any execution files if present.
2. Give "make test_assign4" command to create the executable files for test_assign4_1.
3. Give "make run" command to run test_assign4_1
4. Give "make test_expr" command to create the executable files for test_expr
5. Give "make run_expr" command to run test_expr


3. Functional Description

Index Initialization and Termination

initIndexManager(): Initializes the necessary resources for managing the B+-tree index.

shutdownIndexManager(): Releases resources once the index manager is no longer required.

B+-Tree Index Operations

createBtree(): Initializes a new B+-tree index and stores it in a file managed by the storage manager.

openBtree(): Opens an existing B+-tree index for both reading and writing operations.

closeBtree(): Commits any changes and closes the currently open B+-tree index.

deleteBtree(): Completely removes an existing B+-tree index from the system.

Retrieving B+-Tree Details

getNumNodes(): Retrieves the total count of nodes (pages) present in the B+-tree.

getNumEntries(): Returns the total number of key-value pairs stored in the B+-tree.

getKeyType(): Indicates the data type of keys used within the B+-tree (e.g., integer).

Key Management Operations

findKey(): Looks up a specific key and returns its associated Record ID (RID) if it exists.

insertKey(): Adds a new key and its corresponding Record ID to the B+-tree; returns an error if the key is already present.

deleteKey(): Removes a key and its associated Record ID from the B+-tree.

Tree Traversal

openTreeScan(): Starts a scan over B+-tree entries that meet a defined condition.

nextEntry(): Fetches the next entry in the ongoing scan; signals completion when no further entries match.

closeTreeScan(): Terminates the scan and frees any resources that were allocated for it.