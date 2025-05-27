#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "record_mgr.h"
#include "buffer_mgr.h"
#include "storage_mgr.h"
#include <ctype.h>
#include <unistd.h>
#include <stddef.h> // For size_t

#define PAGE_SIZE 4096 // Example page size, adjust as needed

const int MAX_NUMBER_OF_PAGES = 100;
int ATTR_SIZE = 15;
RecManager *recMgr;

/* Dummy function to simulate initialization */
void dummyInitialization()
{
    // This function serves as a placeholder for future initialization logic
}

/* Dummy function to simulate cleanup */
void dummyCleanup()
{
    // Placeholder for cleanup operations
    //printf("Dummy cleanup executed.\n");
}

/* Dummy function to log actions */
void logAction(const char *action)
{
    // Logs the action taken by the system
    printf("Action logged: %s\n", action);
}

/**
 * Finds the first free slot in the data array.
 *
 * @param data Pointer to the data array representing the page.
 * @param recordSize Size of each record in bytes.
 * @return The index of the first free slot, or -1 if no free slot is found.
 */
int findFreeSlot(char *data, int recordSize)
{
    if (recordSize <= 0) {
        // Invalid record size
        return -1;
    }

    int maxSlotSize = PAGE_SIZE / recordSize;
    char *current = data; // Pointer to traverse the data array

    // Iterate through the slots to find a free one
    for (int slot = 0; slot < maxSlotSize; slot++, current += recordSize)
    {
        if (*current != '+')
        {
            return slot;
        }
    }

    // No free slot found
    return -1;
}

/**
 * Initializes the Record Manager.
 *
 * @param mgmtData Management data to initialize the record manager.
 * @return RC indicating success or failure.
 */
extern RC initRecordManager(void *mgmtData)
{
    // Perform dummy initialization
    dummyInitialization();

    // Initialize the storage manager and return its status
    return initStorageManager();
}

/**
 * Frees the record manager and its resources.
 */
void freeRecordManager()
{
    // Perform dummy cleanup before freeing resources
    dummyCleanup();

    // Free the record manager and set the pointer to NULL
    free(recMgr);
    recMgr = NULL;
}

/**
 * Shuts down the Record Manager and frees resources.
 *
 * @return RC indicating success or failure.
 */
extern RC shutdownRecordManager()
{
    // Shut down the buffer pool associated with the record manager
    shutdownBufferPool(&(recMgr->buffPool));

    // Free the record manager resources
    freeRecordManager();

    // Indicate successful shutdown
    return RC_OK;
}

/**
 * Checks if the return code indicates an error.
 *
 * @param retCode The return code to check.
 * @return True if the return code is not RC_OK.
 */
bool isRetCodeOk(RC retCode)
{
    // Check if the return code signifies an error
    return (retCode != RC_OK);
}

/**
 * Attempts to create and write to a page file.
 *
 * @param name Name of the file.
 * @param fileHandler File handler for the file.
 * @param data Data to write to the file.
 * @return RC indicating success or failure.
 */
RC getRetCode(char *name, SM_FileHandle fileHandler, char *data)
{
    RC retCode;

    // Attempt to create a new page file
    retCode = createPageFile(name);
    if (isRetCodeOk(retCode))
    {
        return retCode;
    }

    // Try opening the created page file
    retCode = openPageFile(name, &fileHandler);
    if (isRetCodeOk(retCode))
    {
        return retCode;
    }

    // Write data to the first block of the file
    retCode = writeBlock(0, &fileHandler, data);
    if (isRetCodeOk(retCode))
    {
        return retCode;
    }

    // Close the page file after writing
    retCode = closePageFile(&fileHandler);
    if (isRetCodeOk(retCode))
    {
        return retCode;
    }

    // If all operations are successful, return RC_OK
    return RC_OK;
}

/**
 * Sets an integer value at the current page handle position and advances the handle by 'buf' bytes.
 *
 * @param pageHandle Pointer to the page handle.
 * @param buf Number of bytes to advance the page handle.
 * @param value Value to set at the current position.
 */
void setPageHandle(char **pageHandle, int buf, int value)
{
    // Cast the current page handle to an integer pointer and assign the value
    *((int *)(*pageHandle)) = value;

    // Move the page handle forward by 'buf' bytes
    *pageHandle += buf;
}

/**
 * Assigns the number of attributes to the page handle and advances the handle.
 *
 * @param numOfAttr Number of attributes.
 * @param pageHandle Pointer to the page handle.
 */
void setNumOfAttr(int numOfAttr, char **pageHandle)
{
    // Store the number of attributes at the current page handle position
    *((int *)(*pageHandle)) = numOfAttr;

    // Advance the page handle by the size of an integer
    *pageHandle += sizeof(int);
}

/**
 * Sets the schema key size in the page handle and moves the handle forward.
 *
 * @param schemaKeySize Size of the schema key.
 * @param pageHandle Pointer to the page handle.
 */
void setSchemaKeySize(int schemaKeySize, char **pageHandle)
{
    // Assign the schema key size to the current position of the page handle
    *((int *)(*pageHandle)) = schemaKeySize;

    // Increment the page handle by the size of an integer
    *pageHandle += sizeof(int);
}

/**
 * Iterates through the attributes in the schema and writes their details to the page handle.
 *
 * @param model Schema model containing attributes.
 * @param pgHandle Pointer to the page handle.
 */
void loopThroughAttrs(Schema *model, char *pgHandle)
{
    int totalAttributes = model->numAttr;
    int index = 0;

    while (index < totalAttributes)
    {
        // Retrieve the attribute name
        char *attributeName = model->attrNames[index];

        // Copy the attribute name into the page handle with a fixed size
        strncpy(pgHandle, attributeName, ATTR_SIZE);
        pgHandle += ATTR_SIZE;

        // Store the attribute data type
        int attributeDataType = (int)model->dataTypes[index];
        *((int *)pgHandle) = attributeDataType;
        pgHandle += sizeof(int);

        // Store the attribute type length
        int attributeTypeLength = (int)model->typeLength[index];
        *((int *)pgHandle) = attributeTypeLength;
        pgHandle += sizeof(int);

        // Move to the next attribute
        index++;
    }
}

/**
 * Function to create a new table with the given name and schema.
 *
 * @param name Name of the table.
 * @param schema Schema of the table.
 * @return RC indicating success or failure.
 */
extern RC createTable(char *name, Schema *schema)
{
    char data[PAGE_SIZE];
    char *pageHandle = data;
    SM_FileHandle fileHandler;

    // Allocate memory for the Record Manager
    recMgr = (RecManager *)malloc(sizeof(RecManager));

    // Initialize the buffer pool with the specified parameters
    initBufferPool(&recMgr->buffPool, name, 100, RS_LRU, NULL);

    // Set initial values in the page handle
    setPageHandle(&pageHandle, sizeof(int), 0); // Tuple count
    setPageHandle(&pageHandle, sizeof(int), 1); // Free page number

    // Set the number of attributes and schema key size
    setNumOfAttr(schema->numAttr, &pageHandle);
    setSchemaKeySize(schema->keySize, &pageHandle);

    // Log action before populating attributes
    logAction("Populating attribute details in the schema");

    // Populate attribute details in the schema
    loopThroughAttrs(schema, pageHandle);

    // Finalize table creation and return the result code
    return getRetCode(name, fileHandler, data);
}

/**
 * Initializes the RM_TableData structure with the table name and management data.
 *
 * @param rel Pointer to the RM_TableData structure.
 * @param name Name of the table.
 */
void initOpenTable(RM_TableData **rel, char **name)
{
    // Assign the table name
    (*rel)->name = *name;

    // Link the Record Manager to the table
    (*rel)->mgmtData = recMgr;
}

/**
 * Initializes the page handle and retrieves attribute count from the Record Manager.
 *
 * @param rel Pointer to the RM_TableData structure.
 * @param pageHandle Pointer to the page handle.
 * @param attributeCount Pointer to store the number of attributes.
 */
void initOpenTablePageHandle(RM_TableData **rel, char **pageHandle, int *attributeCount)
{
    int bufferSize = sizeof(int);

    // Point the page handle to the beginning of the data
    *pageHandle = (char *)recMgr->pgHandle.data;

    // Extract tuple count from the page handle
    recMgr->tupCount = *((int *)(*pageHandle));
    *pageHandle += bufferSize;

    // Extract free page count from the page handle
    recMgr->freePg = *((int *)(*pageHandle));
    *pageHandle += bufferSize;

    // Extract the number of attributes from the page handle
    *attributeCount = *((int *)(*pageHandle));
    *pageHandle += bufferSize;
}

/**
 * Allocates memory for the schema based on the number of attributes.
 *
 * @param relSchema Pointer to the Schema structure.
 * @param attributeCount Number of attributes.
 */
void allocateMemoryForSchema(Schema **relSchema, int attributeCount)
{
    // Allocate memory for attribute names
    (*relSchema)->attrNames = (char **)malloc(sizeof(char *) * attributeCount);

    // Assign the number of attributes
    (*relSchema)->numAttr = attributeCount;

    // Allocate memory for data types
    (*relSchema)->dataTypes = (DataType *)malloc(sizeof(DataType) * attributeCount);

    // Allocate memory for type lengths
    (*relSchema)->typeLength = (int *)malloc(sizeof(int) * attributeCount);
}

/**
 * Resets the attribute count to zero.
 *
 * @param attrCount Pointer to the attribute count variable.
 */
void resetAttrCount(int *attrCount)
{
    *attrCount = 0;
}

/**
 * Processes each attribute and populates the schema with attribute details.
 *
 * @param attr Pointer to the attribute index.
 * @param numAttr Total number of attributes.
 * @param pageHandle Pointer to the page handle.
 * @param relSchema Pointer to the Schema structure.
 */
void whileAttrIsLessThanNumAttr(int *attr, int numAttr, char **pageHandle, Schema **relSchema)
{
    while (*attr < numAttr)
    {
        // Assign attribute name from the page handle
        (*relSchema)->attrNames[*attr] = *pageHandle;
        *pageHandle += ATTR_SIZE;

        // Assign attribute data type from the page handle
        (*relSchema)->dataTypes[*attr] = *((DataType *)(*pageHandle));
        *pageHandle += sizeof(int);

        // Assign attribute type length from the page handle
        (*relSchema)->typeLength[*attr] = *((int *)(*pageHandle));
        *pageHandle += sizeof(int);

        // Move to the next attribute
        (*attr)++;
    }
}

/**
 * Function to open an existing table with the given name and initialize its metadata.
 *
 * @param rel Pointer to the RM_TableData structure.
 * @param name Name of the table.
 * @return RC indicating success or failure.
 */
extern RC openTable(RM_TableData *rel, char *name)
{
    SM_PageHandle pageHandle;
    int attributeCount;
    int attr;
    const int ATTR_SIZE = 15; // Defined as a constant for clarity

    // Reset the attribute counter to zero
    resetAttrCount(&attr);

    // Initialize the RM_TableData structure with the table name and management data
    initOpenTable(&rel, &name);

    // Pin the first page (page number 0) of the buffer pool to access table metadata
    pinPage(&recMgr->buffPool, &recMgr->pgHandle, 0);

    // Initialize the page handle and retrieve the number of attributes from the pinned page
    initOpenTablePageHandle(&rel, &pageHandle, &attributeCount);

    // Allocate memory for the Schema structure based on the number of attributes
    Schema *relSchema = (Schema *)malloc(sizeof(Schema));
    allocateMemoryForSchema(&relSchema, attributeCount);

    // Allocate memory for each attribute name within the schema
    while (attr < attributeCount)
    {
        relSchema->attrNames[attr] = (char *)malloc(ATTR_SIZE);
        attr += 1;
    }

    // Reset the attribute counter before populating schema details
    resetAttrCount(&attr);

    // Log action before populating schema
    logAction("Populating schema details from the page handle");

    // Populate the schema with attribute names, data types, and type lengths from the page handle
    whileAttrIsLessThanNumAttr(&attr, attributeCount, &pageHandle, &relSchema);

    // Assign the populated schema to the RM_TableData structure
    rel->schema = relSchema;

    // Unpin the page from the buffer pool after accessing its data
    unpinPage(&recMgr->buffPool, &recMgr->pgHandle);

    // Force the changes on the pinned page to be written back to disk
    forcePage(&recMgr->buffPool, &recMgr->pgHandle);

    // Indicate successful table opening
    return RC_OK;
}

/**
 * Function to close an open table and perform necessary cleanup.
 *
 * @param rel Pointer to the RM_TableData structure.
 * @return RC indicating success or failure.
 */
extern RC closeTable(RM_TableData *rel)
{
    // Check if the provided table reference is NULL
    if (rel == NULL)
    {
        // If NULL, there's nothing to close; return success
        return RC_OK;
    }
    else
    {
        // Check if the management data associated with the table is not NULL
        if (rel->mgmtData != NULL)
        {
            // Cast the management data to RecManager pointer
            RecManager *temp_recMgr = rel->mgmtData;

            // Shutdown the buffer pool associated with the Record Manager
            shutdownBufferPool(&(temp_recMgr->buffPool));
        }

        // After cleanup, return success
        return RC_OK;
    }
}

/**
 * Function to delete an existing table by destroying its page file.
 *
 * @param name Name of the table.
 * @return RC indicating success or failure.
 */
extern RC deleteTable(char *name)
{
    // Attempt to destroy the page file with the given name and return the result code
    return destroyPageFile(name);
}

/**
 * Function to retrieve the number of tuples in a table.
 *
 * @param rel Pointer to the RM_TableData structure.
 * @return Number of tuples in the table.
 */
extern int getNumTuples(RM_TableData *rel)
{
    // Cast the management data to RecManager pointer
    RecManager *temp_recMgr = rel->mgmtData;

    // Return the tuple count from the Record Manager
    return temp_recMgr->tupCount;
}

/**
 * Function to check if both RM_TableData and Record pointers are not NULL.
 *
 * @param rel Pointer to the RM_TableData structure.
 * @param record Pointer to the Record structure.
 * @return True if both pointers are not NULL.
 */
bool isRelAndRecordNull(RM_TableData *rel, Record *record)
{
    // Return true only if both 'rel' and 'record' are not NULL
    return (rel != NULL) && (record != NULL);
}

/**
 * Function to set the page and slot information for a record.
 *
 * @param recID Pointer to the RID structure.
 * @param tempRecMgr Pointer to the Record Manager.
 * @param recordSize Size of each record.
 * @param data Pointer to the data buffer.
 */
void setRecordPageAndDataAndSlot(RID **recID, RecManager **tempRecMgr, int recordSize, char **data)
{
    // Assign the free page from RecManager to the record's page
    (**recID).page = (**tempRecMgr).freePg;

    // Pin the specified page in the buffer pool
    pinPage(&(*tempRecMgr)->buffPool, &(*tempRecMgr)->pgHandle, (*recID)->page);

    // Set the data pointer to the start of the pinned page's data
    *data = (**tempRecMgr).pgHandle.data;

    // Find a free slot within the page for the record and assign it to the record's slot
    (*recID)->slot = findFreeSlot(*data, recordSize);
}

/**
 * Function to handle scenarios where the record's slot is not found (-1).
 *
 * @param recID Pointer to the RID structure.
 * @param tempRecMgr Pointer to the Record Manager.
 * @param recordSize Size of each record.
 * @param data Pointer to the data buffer.
 */
void whileRecordIdSlotIsNegativeOne(RID **recID, RecManager **tempRecMgr, int recordSize, char **data)
{
    // Continue searching for a free slot as long as the current slot is -1
    while ((*recID)->slot == -1)
    {
        // Unpin the current page from the buffer pool
        unpinPage(&(*tempRecMgr)->buffPool, &(*tempRecMgr)->pgHandle);

        // Move to the next page
        (*recID)->page += 1;

        // Pin the new page in the buffer pool
        pinPage(&(*tempRecMgr)->buffPool, &(*tempRecMgr)->pgHandle, (*recID)->page);

        // Update the data pointer to the new page's data
        *data = (*tempRecMgr)->pgHandle.data;

        // Attempt to find a free slot in the new page
        (*recID)->slot = findFreeSlot(*data, recordSize);
    }
}

/**
 * Function to determine if the current page can be marked as dirty.
 *
 * @param tempRecMgr Pointer to the Record Manager.
 * @param recID Pointer to the RID structure.
 * @return True if the page can be marked as dirty.
 */
bool canMakeDirty(RecManager **tempRecMgr, RID **recID)
{
    // Return true if the free page in RecManager matches the record's current page
    return ((*tempRecMgr)->freePg == (*recID)->page);
}

/**
 * Function to insert a record into the table.
 *
 * @param rel Pointer to the RM_TableData structure.
 * @param record Pointer to the Record structure.
 * @return RC indicating success or failure.
 */
extern RC insertRecord(RM_TableData *rel, Record *record)
{
    char *data, *newData;

    // Check if both the table and record are not NULL
    bool isNotNull = isRelAndRecordNull(rel, record);
    if (isNotNull)
    {
        RID *recID = &record->id;
        RecManager *tempRecMgr = rel->mgmtData;
        int recordSize = getRecordSize(rel->schema);

        // Set the record's page, data pointer, and slot
        setRecordPageAndDataAndSlot(&recID, &tempRecMgr, recordSize, &data);

        // Continue searching for a free slot if the current slot is -1
        whileRecordIdSlotIsNegativeOne(&recID, &tempRecMgr, recordSize, &data);

        // Determine if data is valid and a slot was found
        newData = (data != NULL && recID->slot != -1) ? data : NULL;

        // Mark the page as dirty if applicable
        if (canMakeDirty(&tempRecMgr, &recID))
        {
            markDirty(&tempRecMgr->buffPool, &tempRecMgr->pgHandle);
        }

        // Calculate the exact position to insert the record
        newData = newData + recID->slot * recordSize;

        // Insert a '+' to indicate an occupied slot
        *newData = '+';

        // Copy the record's data into the calculated position, skipping the first byte
        memcpy(++newData, record->data + 1, recordSize - 1);

        // Increment the total tuple count
        tempRecMgr->tupCount += 1;
    }

    return RC_OK;
}

/**
 * Function to reset data pointers when current pointer is less than record size.
 *
 * @param recordSize Size of each record.
 * @param data Pointer to the data buffer.
 * @param id Record ID.
 */
void whileCurPointerIsLessThanRecordSize(int recordSize, char *data, RID id)
{
    for (int i = 0; i < recordSize; i++)
    {
        int index = id.slot * recordSize;
        data[index + i] = '-';
    }
}

/**
 * Function to mark a page as dirty, unpin it, and optionally force it to disk.
 *
 * @param tempRecMgr Pointer to the Record Manager.
 * @param shouldForce Boolean indicating whether to force the page to disk.
 */
void makeDirtyUnpinAndForcePage(RecManager *tempRecMgr, bool shouldForce)
{
    // Mark the current page as dirty
    markDirty(&tempRecMgr->buffPool, &tempRecMgr->pgHandle);

    // Unpin the current page from the buffer pool
    unpinPage(&tempRecMgr->buffPool, &tempRecMgr->pgHandle);

    // If required, force the page to be written to disk
    if (shouldForce)
    {
        forcePage(&tempRecMgr->buffPool, &tempRecMgr->pgHandle);
    }
}

/**
 * Function to delete a record from the table.
 *
 * @param rel Pointer to the RM_TableData structure.
 * @param id Record ID of the record to delete.
 * @return RC indicating success or failure.
 */
extern RC deleteRecord(RM_TableData *rel, RID id)
{
    // Retrieve the Record Manager from the table's management data
    RecManager *tempRecordManager = rel->mgmtData;

    // Pin the page where the record resides in the buffer pool
    pinPage(&tempRecordManager->buffPool, &tempRecordManager->pgHandle, id.page);

    // Update the free page pointer to the current page
    tempRecordManager->freePg = id.page;

    // Access the data within the pinned page
    char *recData = tempRecordManager->pgHandle.data;

    // Determine the size of the record based on the table's schema
    int recordSize = getRecordSize(rel->schema);

    // Reset the record's data pointers based on the record size and RID
    whileCurPointerIsLessThanRecordSize(recordSize, recData, id);

    // Mark the page as dirty, unpin it, and force it to disk
    makeDirtyUnpinAndForcePage(tempRecordManager, true);

    // Indicate successful record deletion
    return RC_OK;
}

/**
 * Function to copy data to a record at a specific position.
 *
 * @param data Pointer to the data buffer.
 * @param record Pointer to the Record structure.
 * @param recordSize Size of each record.
 * @param dataPos Position in the data buffer.
 */
void copyDataToRecord(char **data, Record **record, int recordSize, int dataPos)
{
    // Move the data pointer to the specified position
    *data += dataPos;

    // Mark the slot as occupied by setting it to '+'
    **data = '+';

    // Copy the record's data into the designated slot, excluding the first byte
    memcpy(++(*data), (*record)->data + 1, recordSize - 1);
}

/**
 * Function to update an existing record in the table.
 *
 * @param rel Pointer to the RM_TableData structure.
 * @param record Pointer to the Record structure.
 * @return RC indicating success or failure.
 */
extern RC updateRecord(RM_TableData *rel, Record *record)
{
    // Retrieve the Record Manager from the table's management data
    RecManager *tempRecMgr = rel->mgmtData;

    // Pin the page where the record resides in the buffer pool
    pinPage(&tempRecMgr->buffPool, &tempRecMgr->pgHandle, record->id.page);

    // Extract the Record ID and associated data
    RID id = record->id;
    char *data = tempRecMgr->pgHandle.data;

    // Determine the size of the record based on the table's schema
    int recordSize = getRecordSize(rel->schema);

    // Calculate the position within the data where the record should be updated
    int dataPos = id.slot * recordSize;

    // Copy the new data to the record's designated slot
    copyDataToRecord(&data, &record, recordSize, dataPos);

    // Mark the page as dirty, unpin it, and optionally force it to disk
    makeDirtyUnpinAndForcePage(tempRecMgr, false);

    // Indicate successful record update
    return RC_OK;
}

/**
 * Function to set the record's data and update its size.
 *
 * @param record Pointer to the Record structure.
 * @param recordSize Size of each record.
 * @param data Pointer to the data buffer.
 * @param id Record ID.
 */
void setRecordDataAndRecordSize(Record **record, int recordSize, char **data, RID id)
{
    // Assign the Record ID to the record
    (*record)->id = id;

    // Access the current data pointer within the record
    char *currData = (*record)->data;

    // Copy the data into the record's data buffer, excluding the first byte
    memcpy(++currData, (*data) + 1, recordSize - 1);
}

/**
 * Function to retrieve a record from the table based on its RID.
 *
 * @param rel Pointer to the RM_TableData structure.
 * @param id Record ID of the desired record.
 * @param record Pointer to the Record structure to populate.
 * @return RC indicating success or failure.
 */
extern RC getRecord(RM_TableData *rel, RID id, Record *record)
{
    // Retrieve the Record Manager from the table's management data
    RecManager *tempRecMgr = rel->mgmtData;

    // Pin the specific page in the buffer pool where the record resides
    RC status = pinPage(&tempRecMgr->buffPool, &tempRecMgr->pgHandle, id.page);
    if (status != RC_OK) {
        // Handle error
        return status;
    }

    // Determine the size of the record based on the table's schema
    int recordSize = getRecordSize(rel->schema);

    // Access the data buffer of the pinned page
    char *data = tempRecMgr->pgHandle.data;

    // Calculate the exact position of the record within the data buffer
    data += (id.slot * recordSize);

    // Check if the slot is marked as occupied ('+')
    if (*data != '+')
    {
        // Unpin the page before returning
        unpinPage(&tempRecMgr->buffPool, &tempRecMgr->pgHandle);
        return RC_RM_NO_TUPLE_WITH_GIVEN_RID;
    }

    // Populate the Record structure with data from the data buffer
    setRecordDataAndRecordSize(&record, recordSize, &data, id);

    // Unpin the page after accessing its data
    unpinPage(&tempRecMgr->buffPool, &tempRecMgr->pgHandle);

    // Indicate successful retrieval of the record
    return RC_OK;
}

/**
 * Function to initialize the Table Manager and Scan Manager with relevant data.
 *
 * @param tableManager Pointer to the Table Manager.
 * @param scanManager Pointer to the Scan Manager.
 * @param rel Pointer to the RM_TableData structure.
 * @param scan Pointer to the RM_ScanHandle structure.
 * @param cond Pointer to the scan condition expression.
 */
void setTableManagerAndScanManager(RecManager **tableManager, RecManager **scanManager, RM_TableData **rel, RM_ScanHandle **scan, Expr **cond)
{
    // Initialize the total tuple count in the Table Manager
    (*tableManager)->tupCount = ATTR_SIZE;

    // Initialize the Record ID slot and scan count in the Scan Manager
    (*scanManager)->recID.slot = 0;
    (*scanManager)->scnCount = 0;

    // Associate the Scan Manager with the Scan Handle
    (*scan)->mgmtData = *scanManager;

    // Assign the page number to the Record ID in the Scan Manager
    (*scanManager)->recID.page = 1; // Simplified from 1 + 0 + 0 to 1

    // Set the scan condition in the Scan Manager
    (*scanManager)->cond = *cond;

    // Link the Scan Handle to the table's metadata
    (*scan)->rel = *rel;
}


//
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Function Prototypes
extern RC startScan(RM_TableData *rel, RM_ScanHandle *scan, Expr *cond);
extern RC next(RM_ScanHandle *scan, Record *record);
RC freeScanPtr(RM_ScanHandle **scan);

// Helper Functions
void setScanManagerRecordIdAndScanCount(RecManager **scanManager, int page, int slot, int count);
void setScanManagerForNext(RecManager **scanManager, int *scanCount, int totalSlots);
void setDataPointerPageAndSlotForNext(char **dataPointer, RecManager **scanManager, Record **record);
void copyToMemoryAndEval(Record **record, Schema **schema, Expr **cond, Value **result, char **dataPointer, char **data, int *schemaRecordSize, RecManager **scanManager);

// Initialize a scan on the given table with a condition
extern RC startScan(RM_TableData *rel, RM_ScanHandle *scan, Expr *cond)
{
    // Check if condition is provided
    if (cond == NULL)
    {
        return RC_SCAN_CONDITION_NOT_FOUND;
    }

    RecManager *scanManager = NULL;
    RecManager *tableManager = NULL;

    // Attempt to open the table
    int openStatus = openTable(rel, "ScanTable");
    if (openStatus == RC_OK)
    {
        // Allocate memory for scanManager if table opens successfully
        scanManager = (RecManager *)malloc(sizeof(RecManager));
    }

    // Retrieve the table manager from the table data
    tableManager = rel->mgmtData;

    // Set the table and scan managers with the provided parameters
    setTableManagerAndScanManager(&tableManager, &scanManager, &rel, &scan, &cond);

    return RC_OK;
}

// Set the record ID and scan count in the scan manager
void setScanManagerRecordIdAndScanCount(RecManager **scanManager, int page, int slot, int count)
{
    (**scanManager).recID.page = page;
    (**scanManager).recID.slot = slot;
    (**scanManager).scnCount = count;
}

// Update the scan manager for the next record
void setScanManagerForNext(RecManager **scanManager, int *scanCount, int totalSlots)
{
    if (*scanCount <= 0)
    {
        // Reset to the first page and slot if scanCount is non-positive
        (**scanManager).recID.page = 1;
        (*scanManager)->recID.slot = 0;
    }
    else
    {
        // Move to the next slot
        (*scanManager)->recID.slot += 1;

        // If slot exceeds total slots, move to the next page
        if ((**scanManager).recID.slot >= totalSlots)
        {
            (**scanManager).recID.slot = 0;
            (**scanManager).recID.page += 1;
        }
    }
}

// Set data pointer and update record's page and slot
void setDataPointerPageAndSlotForNext(char **dataPointer, RecManager **scanManager, Record **record)
{
    // Update the record's page and slot based on scanManager
    (**record).id.page = (**scanManager).recID.page;
    (**record).id.slot = (**scanManager).recID.slot;

    // Set the data pointer to the record's data
    *dataPointer = (*record)->data;

    // Modify the first character of dataPointer as per original logic
    **dataPointer = '-';
}

// Copy data to memory and evaluate the condition expression
void copyToMemoryAndEval(Record **record, Schema **schema, Expr **cond, Value **result, char **dataPointer, char **data, int *schemaRecordSize, RecManager **scanManager)
{
    // Copy the record data excluding the first byte
    memcpy(++(*dataPointer), (*data) + 1, (*schemaRecordSize) - 1);

    // Increment the scan count
    (**scanManager).scnCount++;

    // Evaluate the expression condition
    evalExpr(*record, *schema, *cond, result);
}

// Retrieve the next record that satisfies the scan condition
extern RC next(RM_ScanHandle *scan, Record *record)
{
    if (scan == NULL)
    {
        return RC_SCAN_CONDITION_NOT_FOUND;
    }

    // Allocate memory for the result of the expression evaluation
    Value *result = (Value *)malloc(sizeof(Value));

    RecManager *scanManager = scan->mgmtData;
    if (scanManager->cond == NULL)
    {
        return RC_SCAN_CONDITION_NOT_FOUND;
    }

    RecManager *tableManager = scan->rel->mgmtData;

    // If there are no tuples in the table, return no more tuples
    if (tableManager->tupCount == 0)
        return RC_RM_NO_MORE_TUPLES;

    Schema *schema = scan->rel->schema;

    int schemaRecordSize = getRecordSize(schema);
    int totalSlots = PAGE_SIZE / schemaRecordSize;
    int scanCount = scanManager->scnCount;

    // Iterate through the records until the condition is met or no more tuples
    while (scanCount <= tableManager->tupCount)
    {
        // Update scan manager for the next record
        setScanManagerForNext(&scanManager, &scanCount, totalSlots);

        // Pin the relevant page in the buffer pool
        pinPage(&tableManager->buffPool, &scanManager->pgHandle, scanManager->recID.page);

        char *dataPointer = NULL;

        // Update data pointer and record's page and slot
        setDataPointerPageAndSlotForNext(&dataPointer, &scanManager, &record);

        // Calculate the data address based on current page and slot
        char *data = scanManager->pgHandle.data + (scanManager->recID.slot * schemaRecordSize);

        // Copy data to memory and evaluate the condition
        copyToMemoryAndEval(&record, &schema, &scanManager->cond, &result, &dataPointer, &data, &schemaRecordSize, &scanManager);

        // Check if the evaluated condition is true
        bool isResultTrue = ((*result).v.boolV == TRUE);
        if (isResultTrue)
        {
            // Unpin the page as the record satisfies the condition
            unpinPage(&tableManager->buffPool, &scanManager->pgHandle);
            return RC_OK;
        }

        // Increment scan count if condition not met
        scanCount++;
    }

    // Unpin the last accessed page
    unpinPage(&tableManager->buffPool, &scanManager->pgHandle);

    // Reset the scan manager's record ID and scan count
    setScanManagerRecordIdAndScanCount(&scanManager, 1, 0, 0);

    // Indicate that there are no more tuples to scan
    return RC_RM_NO_MORE_TUPLES;
}

// Free the scan handle and its associated resources
RC freeScanPtr(RM_ScanHandle **scan)
{
    if (*scan != NULL)
    {
        // Free the management data if allocated
        if ((*scan)->mgmtData != NULL)
        {
            free((*scan)->mgmtData);
            (*scan)->mgmtData = NULL;
        }
        // Optionally, free the scan handle itself if it was dynamically allocated
        // free(*scan);
        // *scan = NULL;
    }
    return RC_OK;
}

extern RC closeScan(RM_ScanHandle *scan)
{
    if (scan != NULL)
    {
        RecManager *scanManager = scan->mgmtData;
        RecManager *recManager = scan->rel->mgmtData;

        bool exp = scanManager->scnCount > 0;
        if (exp)
        {
            unpinPage(&recManager->buffPool, &scanManager->pgHandle);
            setScanManagerRecordIdAndScanCount(&scanManager, 1, 0, 0);
        }

        return freeScanPtr(scan);
    }
    else
    {
        return RC_SCAN_CONDITION_NOT_FOUND;
    }
}

void whileAttrIsLessThanNumOfAttr(int attr, int numOfAttr, int *totalSize, Schema **schema)
{
    while (attr < numOfAttr)
    {
        int dataType = (*schema)->dataTypes[attr];

        switch (dataType)
        {
            case DT_INT:
                *totalSize += sizeof(int);
                break;
            case DT_BOOL:
                *totalSize += sizeof(bool);
                break;
            case DT_STRING:
                *totalSize += (*schema)->typeLength[attr];
                break;
            case DT_FLOAT:
                *totalSize += sizeof(float);
                break;
            default:
                // Handle unexpected data types if necessary
                break;
        }

        attr++;
    }
}

extern int getRecordSize(Schema *schema)
{
    int numOfAttr = 0; // Initialize number of attributes
    int totalSize = 0; // Start with total size zero
    int attr = 0;      // Attribute counter

    // Check if schema is not null
    if (schema != NULL)
    {
        numOfAttr = schema->numAttr; // Get number of attributes from schema
    }

    // Calculate total size by iterating attributes
    whileAttrIsLessThanNumOfAttr(attr, numOfAttr, &totalSize, &schema);

    // Increase total size by one and return
    totalSize += 1;
    return totalSize;
}

#include <stdlib.h>

// Assuming necessary typedefs and enums are defined elsewhere
// typedef enum { DT_STRING, DT_INT, DT_FLOAT, DT_BOOL } DataType;
// typedef struct {
//     char **attrNames;
//     int numAttr;
//     DataType *dataTypes;
//     int *typeLength;
//     int keySize;
//     int *keyAttrs;
// } Schema;

/**
 * @brief Sets the attribute names, number of attributes, and data types for the schema.
 *
 * @param schema Pointer to the Schema pointer.
 * @param attrNames Array of attribute names.
 * @param numAttr Number of attributes.
 * @param dataTypes Array of data types corresponding to each attribute.
 */
void setSchemaAttrs(Schema **schema, char **attrNames, int numAttr, DataType *dataTypes)
{
    if (schema == NULL || *schema == NULL) {
        // Handle error: schema is not initialized
        return;
    }

    (*schema)->attrNames = attrNames;
    (*schema)->numAttr = numAttr;
    (*schema)->dataTypes = dataTypes;
}

/**
 * @brief Sets the key attributes, key size, and type lengths for the schema.
 *
 * @param schema Pointer to the Schema pointer.
 * @param keySize Number of key attributes.
 * @param keys Array of key attribute indices.
 * @param typeLength Array of type lengths for each attribute.
 */
void setSchemaKeyAttrs(Schema **schema, int keySize, int *keys, int *typeLength)
{
    if (schema == NULL || *schema == NULL) {
        // Handle error: schema is not initialized
        return;
    }

    (*schema)->keySize = keySize;
    (*schema)->keyAttrs = keys;
    (*schema)->typeLength = typeLength;
}


#include <stdlib.h>

// Assuming necessary typedefs and enums are defined elsewhere
// typedef enum { DT_STRING, DT_INT, DT_FLOAT, DT_BOOL } DataType;
// typedef struct {
//     char **attrNames;
//     int numAttr;
//     DataType *dataTypes;
//     int *typeLength;
//     int keySize;
//     int *keyAttrs;
// } Schema;

// typedef enum { RC_OK, RC_ERROR } RC;

/**
 * @brief Creates a new Schema by allocating memory and setting its attributes.
 *
 * @param attributeNum Number of attributes in the schema.
 * @param aNames Array of attribute names.
 * @param dataTypes Array of data types corresponding to each attribute.
 * @param typeLen Array of type lengths for each attribute.
 * @param keySize Number of key attributes.
 * @param keys Array of key attribute indices.
 * @return Pointer to the newly created Schema, or NULL if allocation fails.
 */
extern Schema *createSchema(int attributeNum, char **aNames, DataType *dataTypes, int *typeLen, int keySize, int *keys)
{
    // Allocate memory for the Schema
    Schema *schema = (Schema *)malloc(sizeof(Schema));
    if (schema == NULL) {
        // Memory allocation failed
        return NULL;
    }

    // Initialize Schema attributes
    setSchemaAttrs(&schema, aNames, attributeNum, dataTypes);
    setSchemaKeyAttrs(&schema, keySize, keys, typeLen);

    return schema;
}

/**
 * @brief Frees the memory allocated for a Schema.
 *
 * @param schema Pointer to the Schema to be freed.
 * @return RC_OK on success, RC_ERROR if schema is NULL.
 */
extern RC freeSchema(Schema *schema)
{
    if (schema == NULL) {
        // Cannot free a NULL pointer
        return RC_ERROR;
    }

    // Assuming that attrNames, dataTypes, typeLength, and keyAttrs are managed elsewhere
    // If they are dynamically allocated within the Schema, they should be freed here as well
    // Example:
    // for (int i = 0; i < schema->numAttr; i++) {
    //     free(schema->attrNames[i]);
    // }
    // free(schema->attrNames);
    // free(schema->dataTypes);
    // free(schema->typeLength);
    // free(schema->keyAttrs);

    // Free the Schema structure itself
    free(schema);

    return RC_OK;
}

void setNewRecordData(Record **newRecord, Schema **schema)
{
    (*newRecord)->data = (char *)malloc(getRecordSize(*schema));
    (*newRecord)->id.page = -1;
    (*newRecord)->id.slot = -1;
}

void updateNewRecordData(Record **newRecord)
{
    char *dataPointer = (**newRecord).data;
    *dataPointer = '-';
    *(++dataPointer) = '\0';
}

extern RC createRecord(Record **record, Schema *schema)
{
    // Allocate memory for the new Record structure
    Record *newRecord = (Record *)malloc(sizeof(Record));
    if (newRecord == NULL)
    {
        // Memory allocation failed
        return RC_ERROR;
    }

    // Verify that the provided schema is valid
    if (schema == NULL)
    {
        // Free the allocated memory before returning to prevent memory leak
        free(newRecord);
        return RC_ERROR;
    }

    // Initialize the Record's data and ID fields using the Schema
    setNewRecordData(&newRecord, &schema);
    updateNewRecordData(&newRecord);

    // Assign the newly created Record to the output parameter
    *record = newRecord;

    return RC_OK;
}

/**
 * @brief Updates the result offset based on the attribute's data type.
 *
 * This function increments the result pointer by the size of the specified
 * attribute's data type. It handles different DataTypes by adding the appropriate
 * byte sizes to the result offset.
 *
 * @param result Double pointer to the integer offset to be updated.
 * @param schema Double pointer to the Schema containing attribute information.
 * @param attr Index of the attribute whose type is used to update the result.
 */
void updateResult(int **result, Schema **schema, int attr)
{
    // Ensure that all pointers are valid before proceeding
    if (result == NULL || *result == NULL || schema == NULL || *schema == NULL)
    {
        return; // Exit gracefully if any pointer is invalid
    }

    // Determine the size to add based on the DataType of the attribute
    if ((*schema)->dataTypes[attr] == DT_STRING)
    {
        // For string types, increment by the defined type length
        **result += (*schema)->typeLength[attr];
    }
    else if ((*schema)->dataTypes[attr] == DT_FLOAT)
    {
        // For float types, increment by the size of a float
        **result += sizeof(float);
    }
    else if ((*schema)->dataTypes[attr] == DT_BOOL)
    {
        // For boolean types, increment by the size of a bool
        **result += sizeof(bool);
    }
    else if ((*schema)->dataTypes[attr] == DT_INT)
    {
        // For integer types, increment by the size of an int
        **result += sizeof(int);
    }
}

RC attrOffset(Schema *schema, int attrNum, int *res)
{
    // Validate that the schema and result pointer are not NULL
    if (schema == NULL)
        return RC_ERROR;

    // Initialize the offset to zero
    *res = 0;

    // Iterate over each attribute up to attrNum to accumulate the offset
    for (int attr = 0; attr < attrNum; attr++)
    {
        updateResult(&res, &schema, attr);
    }

    // Increment the offset by 1 to account for additional padding or metadata
    (*res) += 1;

    return RC_OK;
}

/**
 * @brief Frees the memory allocated for a Record.
 *
 * This function deallocates the memory associated with a Record structure.
 * It first checks if the record pointer is not NULL, frees the allocated memory,
 * and then sets the pointer to NULL to prevent dangling references.
 *
 * @param rec Pointer to the Record to be freed.
 * @return RC_OK after successful deallocation.
 */
extern RC freeRecord(Record *rec)
{
    // Check if the record pointer is valid
    if (rec != NULL) {
        // Free the memory allocated for the record's data if it exists
        if (rec->data != NULL) {
            free(rec->data);
            rec->data = NULL;
        }
        // Free the Record structure itself
        free(rec);
        rec = NULL; // This has no effect outside the function since rec is passed by value
    }
    return RC_OK;
}
extern RC getAttr(Record *rec, Schema *model, int attriNum, Value **value)
{
    // Validate input pointers to ensure they are not NULL
    if (model == NULL || rec == NULL || value == NULL)
    {
        return RC_ERROR;
    }

    // Initialize position to calculate the byte offset of the attribute
    int position = 0;
    RC rc = attrOffset(model, attriNum, &position);
    if (rc != RC_OK) {
        return rc;
    }

    // Pointer to the start of the attribute's data within the record
    char *dataPointer = rec->data + position;

    // Allocate memory for the Value structure to hold the attribute's value
    Value *attri = (Value *)malloc(sizeof(Value));
    if (attri == NULL) {
        return RC_ERROR;
    }

    // **Note:** The following line modifies the schema's dataTypes array, which is typically
    // intended to be immutable. If this modification is intentional, it can remain.
    // Otherwise, consider removing or revising it to prevent unintended side effects.
    model->dataTypes[attriNum] = (attriNum == 1) ? attriNum : model->dataTypes[attriNum];

    // Determine the attribute's data type and extract its value accordingly
    switch (model->dataTypes[attriNum])
    {
        case DT_STRING:
        {
            attri->dt = DT_STRING;

            // Allocate memory for the string value, including space for the null terminator
            size_t string_length = model->typeLength[attriNum];
            attri->v.stringV = (char *)malloc(string_length + 1);
            if (attri->v.stringV == NULL) {
                free(attri); // Free previously allocated memory to prevent leaks
                return RC_ERROR;
            }

            // Copy the string data from the record to the Value structure
            strncpy(attri->v.stringV, dataPointer, string_length);
            attri->v.stringV[string_length] = '\0'; // Ensure null termination
            break;
        }

        case DT_INT:
        {
            attri->dt = DT_INT;
            int num = 0;

            // Copy the integer value byte by byte from the record to the variable 'num'
            char *numPtr = (char *)&num;
            for (int i = 0; i < sizeof(int); i++) {
                *numPtr++ = *dataPointer++;
            }

            attri->v.intV = num;
            break;
        }

        case DT_FLOAT:
        {
            attri->dt = DT_FLOAT;
            float floatVal = 0.0f;

            // Copy the float value from the record to the variable 'floatVal'
            memcpy(&floatVal, dataPointer, sizeof(float));
            attri->v.floatV = floatVal;
            break;
        }

        case DT_BOOL:
        {
            attri->dt = DT_BOOL;
            bool boolVal = false;

            // Copy the boolean value from the record to the variable 'boolVal'
            memcpy(&boolVal, dataPointer, sizeof(bool));
            attri->v.boolV = boolVal;
            break;
        }

        default:
            // Handle unknown data types by freeing allocated memory and returning an error
            free(attri);
            return RC_ERROR;
    }

    // Assign the populated Value structure to the output parameter
    *value = attri;
    return RC_OK;
}
extern RC setAttr(Record *record, Schema *model, int attrNum, Value *value)
{
    // Validate input pointers to ensure they are not NULL
    if (model == NULL || record == NULL || value == NULL)
    {
        return RC_ERROR; // Return error if any pointer is invalid
    }

    // Initialize offset to calculate the byte position of the attribute
    int offset = 0;

    // Calculate the offset for the specified attribute within the record
    RC rc = attrOffset(model, attrNum, &offset);
    if (rc != RC_OK) {
        return rc; // Return error if offset calculation fails
    }

    // Ensure that attrNum is within the valid range of attributes
    if (attrNum < 0 || attrNum >= model->numAttr) {
        return RC_ERROR; // Return error if attribute index is out of bounds
    }

    // Pointer to the start of the attribute's data within the record
    char *dataPointer = record->data + offset;

    // Determine the attribute's data type and set its value accordingly
    switch (model->dataTypes[attrNum])
    {
        case DT_STRING:
        {
            // Set the data type of the Value structure to DT_STRING
            value->dt = DT_STRING;

            // Retrieve the length of the string from the Schema
            int len = model->typeLength[attrNum];
            char *stringV = value->v.stringV;

            // Check if the string value is not NULL
            if (stringV == NULL) {
                return RC_ERROR; // Return error if string value is invalid
            }

            // Copy the string data into the record's data buffer
            for (int i = 0; i < len; i++) {
                // If the source string is shorter than len, pad with null characters
                if (stringV[i] != '\0') {
                    dataPointer[i] = stringV[i];
                } else {
                    dataPointer[i] = '\0';
                }
            }

            // No need to increment dataPointer after the loop as it already points to the correct position
            break;
        }

        case DT_FLOAT:
        {
            // Set the data type of the Value structure to DT_FLOAT
            value->dt = DT_FLOAT;

            // Copy the float value into the record's data buffer using memcpy for accuracy
            memcpy(dataPointer, &(value->v.floatV), sizeof(float));

            // Increment the dataPointer by the size of float (optional, as it's not used further)
            dataPointer += sizeof(float);
            break;
        }

        case DT_INT:
        {
            // Set the data type of the Value structure to DT_INT
            value->dt = DT_INT;

            // Copy the integer value into the record's data buffer using memcpy for accuracy
            memcpy(dataPointer, &(value->v.intV), sizeof(int));

            // Increment the dataPointer by the size of int (optional, as it's not used further)
            dataPointer += sizeof(int);
            break;
        }

        case DT_BOOL:
        {
            // Set the data type of the Value structure to DT_BOOL
            value->dt = DT_BOOL;

            // Copy the boolean value into the record's data buffer using memcpy for accuracy
            memcpy(dataPointer, &(value->v.boolV), sizeof(bool));

            // Increment the dataPointer by the size of bool (optional, as it's not used further)
            dataPointer += sizeof(bool);
            break;
        }

        default:
            // Handle unsupported data types by returning an error
            return RC_ERROR;
    }

    return RC_OK; // Return success after setting the attribute
}