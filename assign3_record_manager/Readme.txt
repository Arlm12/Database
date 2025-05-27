# CS 525: Advanced Database Organization Fall 2024
Assignment 3: Record Manager

# Information about the team:
Group 22: Arunachalam Barathidasan(abarathidasan@hawk.iit.edu)
          Sejal Srivastav (ssrivastav@hawk.iit.edu)
          Prathiksha Shirsat (pshirsat@hawk.iit.edu)

---

## Overview

This README provides an overview of the implementation for the Record Manager, which manages database tables, records, and scans for CS 525: Advanced Database Organization. 
The Record Manager utilizes both the buffer manager and storage manager for handling table operations, ensuring efficient storage and retrieval of data. 


## Table of Contents
1. File Structure
2. Compilation and Execution
3. Functional Description
    - Tables and Manager
    - Record Operations
    - Scanning Records
    - Schema Management
    - Attribute Operations
4. Memory Management
5. Optional Extensions

---

## 1. Structure of File

Before running the `make` command, the file structure is as follows:

```bash
.
|____buffer_mgr.h
|____buffer_mgr_stat.c
|____buffer_mgr_stat.h
|____dberror.c
|____dberror.h
|____expr.c
|____expr.h
|____record_mgr.h
|____rm_serializer.c
|____storage_mgr.h
|____tables.h
|____test_assign3_1.c
|____test_expr.c
|____test_helper.h
|____Makefile
```

After running the `make` command, the compiled object files and executables will be created:

```bash
.
|____buffer_mgr.c
|____buffer_mgr.o
|____buffer_mgr_stat.c
|____buffer_mgr_stat.h
|____buffer_mgr_stat.o
|____dberror.c
|____dberror.h
|____dberror.o
|____dt.h
|____expr.c
|____expr.h
|____expr.o
|____Makefile
|____Readme.md
|____record_mgr.c
|____record_mgr.h
|____record_mgr.o
|____recordmgr.exe
|____rm_serializer.c
|____rm_serializer.o
|____storage_mgr.c
|____storage_mgr.h
|____storage_mgr.o
|____tables.h
|____test_assign3_1.c
|____test_assign3_1.o
|____test_expr.c
|____test_expr.exe
|____test_expr.o
|____test_helper.h
|____test_table_t

```

---

## 2. Compilation and Execution
   Steps to Compile:

   1. Give " make clean " command to clear all the files that are present. 
   Note: We used Windows so del command is used for clean. For mac del should be replaced with rm.

   2. Give " make outresult1 " to generate the .o binary files to execute test_assign3_1
   
   3. Give " make outresult2 " to generate the .o binary files to execute test_expr 
   
   4. Alternatively " make " command can be given to generate all files required
   
   5. Give " make run " to execute test_assign3_1
   
   6. Give " make run_expr " to execute test_expr

## 3. Functional Description

### Tables and Manager
- initRecordManager(void *mgmtData): Initializes the Record Manager.
- shutdownRecordManager(): Releases allocated resources and shuts down the Record Manager.
- createTable(char *name, Schema *schema): Creates a new table, initializes a buffer pool with FIFO replacement strategy, and serializes the schema into the table's first page.
- openTable(RM_TableData *rel, char *name): Opens an existing table by loading schema and other metadata.
- closeTable(RM_TableData *rel): Closes the table and unpins all associated pages.
- deleteTable(char *name): Deletes the table file and deallocates resources.
- getNumTuples(RM_TableData *rel): Returns the number of tuples (records) in the table.

### Record Operations
- insertRecord(RM_TableData *rel, Record *record): Inserts a new record into the table, pinning and unpinning pages as needed.
- deleteRecord(RM_TableData *rel, RID id): Deletes a record by clearing its data and marking the page dirty.
- updateRecord(RM_TableData *rel, Record *record): Updates an existing record's data in the table.
- getRecord(RM_TableData *rel, RID id, Record *record): Retrieves a record by its ID from the table.

### Scanning Records
- startScan(RM_TableData *rel, RM_ScanHandle *scan, Expr *cond): Initializes a scan with a specified condition.
- next(RM_ScanHandle *scan, Record *record): Retrieves the next record that meets the scan condition.
- closeScan(RM_ScanHandle *scan): Ends the scan, releasing associated resources.

### Schema Management
- getRecordSize(Schema *schema): Returns the size (in bytes) of a record based on the schema.
- createSchema(int numAttr, char **attrNames, DataType *dataTypes, int *typeLength, int keySize, int *keys): Defines the schema for a table, including attributes and keys.
- freeSchema(Schema *schema): Frees memory allocated for the schema.

### Attribute Operations
- getAttr(Record *record, Schema *schema, int attrNum, Value **value): Retrieves a specific attribute value from a record.
- setAttr(Record *record, Schema *schema, int attrNum, Value *value): Updates an attribute value in a record.
