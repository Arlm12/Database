#include <string.h>
#include <stdlib.h>
#include "btree_mgr.h"
#include "record_mgr.h"
#include "buffer_mgr.h"
#include "storage_mgr.h"
#include "h_needed.c"


/* 
 * Function: shutdownIndexManager
 * -------------------------------
 * Shuts down the index manager. Currently, it simply returns RC_OK.
 *
 * returns: RC_OK indicating successful shutdown.
 */
RC shutdownIndexManager()
{
    return RC_OK;
}

/* 
 * Dummy Function: dummyFunction1
 * --------------------------------
 * A placeholder function added as per request.
 */
void dummyFunction1() {
    // This is a dummy function.
}

/* 
 * Function: createBtree
 * ---------------------
 * Creates a new B-tree index file with the given index ID, key type, and order 'n'.
 *
 * idxId: Identifier for the index.
 * keyType: Data type of the keys.
 * n: Order of the B-tree.
 *
 * returns: RC_OK on success, appropriate error code otherwise.
 */
RC createBtree(char *idxId, DataType keyType, int n)
{
    SM_FileHandle fhandle;       // File handle for storage manager
    SM_PageHandle pageData;      // Page data buffer

    // Create a new page file for the B-tree index
    createPageFile(idxId);
    openPageFile(idxId, &fhandle);

    // Write initial page data based on key type and order
    writePageData(keyType, n, &pageData);

    if (pageData != NULL)
    {
        // Write the initial page to the current block
        writeCurrentBlock(&fhandle, pageData);
        // Close the page file after writing
        closePageFile(&fhandle);
    }

    // Free the allocated page data buffer
    freePageData(&pageData);

    return RC_OK;
}

/* 
 * Dummy Function: dummyFunction2
 * --------------------------------
 * A placeholder function added as per request.
 */
void dummyFunction2() {
    // This is another dummy function.
}

/* 
 * Function: initIndexManager
 * --------------------------
 * Initializes the index manager by setting root node and other management data to initial states.
 *
 * mgmtData: Pointer to management data (unused in current implementation).
 *
 * returns: RC_OK indicating successful initialization.
 */
RC initIndexManager(void *mgmtData)
{
    rootNode = NULL;            // Initialize root node to NULL
    numNodeValue = 0;           // Initialize number of nodes to 0
    sizeOfNode = 0;             // Initialize node size to 0
    empty.v.intV = 0;           // Initialize empty value
    empty.dt = DT_INT;           // Set data type of empty value to integer
    return RC_OK;
}

/* 
 * Function: openBtree
 * -------------------
 * Opens an existing B-tree index file and initializes the BTreeHandle structure.
 *
 * tree: Double pointer to BTreeHandle to be initialized.
 * idxId: Identifier for the index to open.
 *
 * returns: RC_OK on success, RC_IM_KEY_NOT_FOUND if opening fails.
 */
RC openBtree(BTreeHandle **tree, char *idxId)
{
    int type, n;
    int fourBytes = sizeof(int);

    // Allocate memory for BTreeHandle
    *tree = (BTreeHandle *)malloc(sizeof(BTreeHandle));
    if (tree != NULL)
    {
        BM_BufferPool *bm = MAKE_POOL(); // Create a new buffer pool
        initBufferPool(bm, idxId, 10, RS_CLOCK, NULL); // Initialize buffer pool

        BM_PageHandle *page = MAKE_PAGE_HANDLE(); // Create a page handle
        pinPage(bm, page, 0); // Pin the first page

        if (page != NULL)
        {
            // Read the key type from the first four bytes of the page
            memcpy(&type, (*page).data, fourBytes);
            (*tree)->keyType = (DataType)type;
            (*page).data += fourBytes; // Move the data pointer forward
        }

        if (page->data != NULL)
        {
            // Read the order 'n' from the next four bytes
            memcpy(&n, (*page).data, fourBytes);
            (*page).data -= fourBytes; // Reset the data pointer
        }

        // Update management data with the order and buffer pool
        (*tree)->mgmtData = updateMgmtData(n, bm);

        // Free the page handle
        freePage(page);
        if (page == NULL)
        {
            return RC_IM_KEY_NOT_FOUND; // Return error if page is NULL
        }
        return RC_OK;
    }

    return RC_IM_KEY_NOT_FOUND; // Return error if tree allocation fails
}

/* 
 * Dummy Function: dummyFunction3
 * --------------------------------
 * A placeholder function added as per request.
 */
void dummyFunction3() {
    // This is a third dummy function.
}

/* 
 * Function: closeBtree
 * --------------------
 * Closes an open B-tree index and frees associated resources.
 *
 * tree: Pointer to the BTreeHandle to be closed.
 *
 * returns: RC_OK indicating successful closure.
 */
RC closeBtree(BTreeHandle *tree)
{
    (*tree).idxId = NULL; // Reset the index ID
    shutdownBufferPoolAndFreeMgmt(tree); // Shutdown buffer pool and free management data
    freeTreeNode(tree); // Free tree nodes
    freeRootNode(); // Free the root node
    return RC_OK;
}

/* 
 * Function: deleteBtree
 * ---------------------
 * Deletes an existing B-tree index file.
 *
 * idxId: Identifier for the index to delete.
 *
 * returns: RC_OK on successful deletion.
 */
RC deleteBtree(char *idxId)
{
    return destroyPageFile(idxId); // Destroy the page file associated with the index
}

/* 
 * Dummy Function: dummyFunction4
 * --------------------------------
 * A placeholder function added as per request.
 */
void dummyFunction4() {
    // This is a fourth dummy function.
}

/* 
 * Function: getNumEntries
 * -----------------------
 * Retrieves the number of entries in the B-tree.
 *
 * tree: Pointer to the BTreeHandle.
 * result: Pointer to an integer where the number of entries will be stored.
 *
 * returns: RC_OK on success.
 */
RC getNumEntries(BTreeHandle *tree, int *result)
{
    (*result) = ((RM_bTree_mgmtData *)tree->mgmtData)->num; // Get the number of entries from management data
    return RC_OK;
}

/* 
 * Function: getNumNodes
 * ---------------------
 * Retrieves the number of nodes in the B-tree.
 *
 * tree: Pointer to the BTreeHandle.
 * result: Pointer to an integer where the number of nodes will be stored.
 *
 * returns: RC_OK on success.
 */
RC getNumNodes(BTreeHandle *tree, int *result)
{
    (*result) = numNodeValue; // Get the number of nodes
    return RC_OK;
}

/* 
 * Function: getKeyType
 * --------------------
 * Retrieves the data type of the keys in the B-tree.
 *
 * tree: Pointer to the BTreeHandle.
 * result: Pointer to a DataType where the key type will be stored.
 *
 * returns: RC_OK on success.
 */
RC getKeyType(BTreeHandle *tree, DataType *result)
{
    // Currently, this function does not set the result. It should be implemented.
    return RC_OK;
}

/* 
 * Dummy Function: dummyFunction5
 * --------------------------------
 * A placeholder function added as per request.
 */
void dummyFunction5() {
    // This is a fifth dummy function.
}

/* 
 * Function: findKey
 * -----------------
 * Searches for a given key in the B-tree and retrieves its associated RID.
 *
 * tree: Pointer to the BTreeHandle.
 * key: Pointer to the Value key to search for.
 * result: Pointer to RID where the result will be stored if found.
 *
 * returns: RC_OK if key is found, RC_IM_KEY_NOT_FOUND otherwise.
 */
RC findKey(BTreeHandle *tree, Value *key, RID *result)
{
    int i = 0;
    RM_BtreeNode *leaf = rootNode; // Start from the root node

    // Traverse the B-tree to find the appropriate leaf node
    while (!isLeafNode(leaf))
    {
        updateSerializedValue(leaf, i, key); // Update serialized value for comparison
        leaf = loopUntilIndexIsLessThanKeyCount(leaf, &i, key); // Move to the next node
        i = 0;
    }

    updateSerializedValue(leaf, i, key); // Update serialized value at leaf

    // Iterate through the keys in the leaf node to find the matching key
    while ((i < (*leaf).KeyCounts) && (strcmp(s, s_s) != 0))
    {
        free(s);
        if (++i < (*leaf).KeyCounts)
        {
            s = serializeValue(&leaf->keys[i]);
        }
        else
        {
            s = NULL;
        }
    }

    if (i >= (*leaf).KeyCounts)
    {
        freeSerialized(s);
        freeSerialized(s_s);
        return RC_IM_KEY_NOT_FOUND; // Key not found in the B-tree
    }

    if ((*result).page != -1)
    {
        (*result).page = ((RID *)(*leaf).ptr[i])->page; // Set the page number in result
        int check = (*result).page;
        printf("check %d\n", check);
        (*result).slot = ((RID *)(*leaf).ptr[i])->slot; // Set the slot number in result
    }

    if (s != NULL)
    {
        freeSerialized(s);
    }

    if (s_s != NULL)
    {
        freeSerialized(s_s);
    }

    return RC_OK;
}

/* 
 * Function: insertKey
 * -------------------
 * Inserts a new key-RID pair into the B-tree.
 *
 * tree: Pointer to the BTreeHandle.
 * key: Pointer to the Value key to insert.
 * rid: RID associated with the key.
 *
 * returns: RC_OK on successful insertion.
 */
RC insertKey(BTreeHandle *tree, Value *key, RID rid)
{
    int i = 0;
    RM_BtreeNode *leaf = rootNode; // Start from the root node

    if (leaf != NULL)
    {
        // Traverse to the appropriate leaf node
        while (!isLeafNode(leaf))
        {
            updateSerializedValue(leaf, i, key); // Update serialized value for comparison
            leaf = loopUntilIndexIsLessThanKeyCount(leaf, &i, key); // Move to the next node
            i = 0;
        }
    }

    RM_bTree_mgmtData *bTreeMgmt = (RM_bTree_mgmtData *)(*tree).mgmtData;
    bTreeMgmt->num++; // Increment the number of entries

    if (leaf)
    {
        int index = 0;

        updateSerializedValue(leaf, i, key); // Update serialized value at leaf

        // Find the correct position to insert the new key
        while ((index < (*leaf).KeyCounts) && strCompare(s, s_s))
        {
            free(s);
            index++;
            if (index < (*leaf).KeyCounts)
            {
                s = serializeValue(&leaf->keys[index]);
            }
            else
            {
                s = NULL;
            }
        }
        freeSerialized(s);
        freeSerialized(s_s);

        // Check if the leaf node is full
        if ((*leaf).KeyCounts >= sizeOfNode - 1)
        {
            RID **NodeRID = malloc(sizeOfNode * sizeof(RID *)); // Allocate array for RIDs
            Value *NodeKeys = malloc(sizeOfNode * sizeof(Value)); // Allocate array for keys
            int middleLoc = 0;

            // Split the node and redistribute keys and RIDs
            for (int j = 0; i < sizeOfNode; i++)
            {
                if (i == index)
                {
                    RID *newValue = (RID *)malloc(sizeof(RID));
                    updateRecordPageAndSlot(newValue, rid);
                    if (newValue != NULL)
                    {
                        NodeRID[i] = newValue;
                        NodeKeys[i] = *key;
                    }
                }
                else if (i < index)
                {
                    NodeRID[i] = (*leaf).ptr[i];
                    globalPos = NodeRID[i]->page;

                    if (sizeOfNode % 2 == 0)
                    {
                        middleLoc = true;
                    }
                    else
                    {
                        middleLoc = false;
                    }

                    NodeKeys[i] = (*leaf).keys[i];
                }
                else
                {
                    middleLoc = globalPos;
                    NodeRID[i] = leaf->ptr[i - 1];
                    if (i != -1)
                    {
                        globalPos = NodeRID[i]->page;
                        NodeKeys[i] = leaf->keys[i - 1];
                    }
                }
            }

            middleLoc = sizeOfNode / 2 + 1;

            // Update the original leaf with the first half of the keys and RIDs
            for (i = 0; i < middleLoc; i++)
            {
                (*leaf).ptr[i] = NodeRID[i];
                (*leaf).keys[i] = NodeKeys[i];
            }

            // Create a new leaf node and assign the second half of the keys and RIDs
            RM_BtreeNode *newLeafNode = createNewNode(newLeafNode);
            (*newLeafNode).isLeaf = true;
            if (isLeafNode(newLeafNode))
            {
                (*newLeafNode).pptr = (*leaf).pptr;
                (*newLeafNode).KeyCounts = sizeOfNode - middleLoc;
            }

            for (i = middleLoc; i < sizeOfNode; i++)
            {
                int idx = i - middleLoc;
                (*newLeafNode).ptr[idx] = NodeRID[i];
                (*newLeafNode).keys[idx] = NodeKeys[i];
            }

            (*newLeafNode).ptr[sizeOfNode - 1] = leaf->ptr[sizeOfNode - 1];
            if (leaf != NULL)
            {
                (*leaf).KeyCounts = middleLoc;
                (*leaf).ptr[sizeOfNode - 1] = newLeafNode;
            }

            // Free allocated memory for temporary arrays
            if (NodeRID != NULL)
            {
                free(NodeRID);
                NodeRID = NULL;
            }

            if (NodeKeys != NULL)
            {
                free(NodeKeys);
                NodeKeys = NULL;
            }

            // Insert the new parent node after splitting
            return insertParent(leaf, newLeafNode, newLeafNode->keys[0]);
        }
        else
        {
            // Shift keys and pointers to make space for the new key
            for (int i = (*leaf).KeyCounts; i > index; i--)
            {
                setNodeKeysAndPtr(leaf, i, -1, i, -1);
            }

            // Allocate and update RID for the new key
            RID *rec = (RID *)malloc(sizeof(RID));
            updateRecordPageAndSlot(rec, rid);
            updateLeafNodeKeysAndPtr(leaf, index, rec, key); // Insert the new key and RID
        }
    }
    else
    {
        {
            sizeOfNode = (*bTreeMgmt).maxKeyNum + 1; // Set the size of the node
            rootNode = createNewNode(rootNode); // Create a new root node
            RID *rec = (RID *)malloc(sizeof(RID));
            initNonLeafNode(rec, rid, rootNode, key); // Initialize the non-leaf node
        }
    }
    (*tree).mgmtData = bTreeMgmt; // Update the management data
    return RC_OK;
}

/* 
 * Dummy Function: dummyFunction6
 * --------------------------------
 * A placeholder function added as per request.
 */
void dummyFunction6() {
    // This is a sixth dummy function.
}

/* 
 * Function: deleteKey
 * -------------------
 * Deletes a key from the B-tree.
 *
 * tree: Pointer to the BTreeHandle.
 * ke: Pointer to the Value key to delete.
 *
 * returns: RC_OK on successful deletion, RC_IM_KEY_NOT_FOUND otherwise.
 */
RC deleteKey(BTreeHandle *tree, Value *ke)
{
    RM_BtreeNode *leaf = rootNode; // Start from the root node
    RC rc;
    int i = 0;

    RM_bTree_mgmtData *bTreeMgmt = (RM_bTree_mgmtData *)tree->mgmtData;

    if (leaf != NULL)
    {
        // Traverse to the appropriate leaf node
        while (!isLeafNode(leaf))
        {
            updateSerializedValue(leaf, i, ke); // Update serialized value for comparison
            leaf = loopUntilIndexIsLessThanKeyCount(leaf, &i, ke); // Move to the next node
            i = 0;
        }

        updateSerializedValue(leaf, i, ke); // Update serialized value at leaf

        // Iterate through the keys in the leaf node to find the matching key
        while ((i < (*leaf).KeyCounts) && (strcmp(s, s_s) != 0))
        {
            freeSerialized(s);
            if (++i < (*leaf).KeyCounts)
            {
                s = serializeValue(&leaf->keys[i]);
            }
            else
            {
                s = NULL;
            }
        }

        if (i < (*leaf).KeyCounts)
        {
            deleteNode(leaf, i); // Delete the node at index i
            return RC_OK;
        }

        freeSerialized(s);
        freeSerialized(s_s);
    }

    if (bTreeMgmt != NULL)
    {
        --(*bTreeMgmt).num; // Decrement the number of entries
        (*tree).mgmtData = bTreeMgmt;
    }
    return RC_OK;
}

/* 
 * Function: openTreeScan
 * ----------------------
 * Initializes a scan operation on the B-tree.
 *
 * tree: Pointer to the BTreeHandle.
 * handle: Double pointer to BT_ScanHandle to be initialized.
 *
 * returns: RC_OK on successful initialization, RC_IM_KEY_NOT_FOUND otherwise.
 */
RC openTreeScan(BTreeHandle *tree, BT_ScanHandle **handle)
{
    if (tree != NULL)
    {
        // Allocate memory for BT_ScanHandle
        *handle = (BT_ScanHandle *)malloc(sizeof(BT_ScanHandle));
        (*handle)->tree = tree;

        // Allocate and initialize scan management data
        RM_BScan_mgmt *scanMgmt = (RM_BScan_mgmt *)malloc(sizeof(RM_BScan_mgmt));
        (*handle)->mgmtData = scanMgmt;

        if (scanMgmt != NULL)
        {
            (*scanMgmt).cur = NULL;       // Initialize current node to NULL
            (*scanMgmt).index = 0;        // Initialize index to 0
            (*scanMgmt).tot_Scan = 0;     // Initialize total scanned entries to 0
        }
        return RC_OK;
    }
    return RC_IM_KEY_NOT_FOUND; // Return error if tree is NULL
}

/* 
 * Dummy Function: dummyFunction7
 * --------------------------------
 * A placeholder function added as per request.
 */
void dummyFunction7() {
    // This is a seventh dummy function.
}

/* 
 * Function: nextEntry
 * -------------------
 * Retrieves the next entry in the B-tree scan.
 *
 * handle: Pointer to the BT_ScanHandle.
 * result: Pointer to RID where the result will be stored.
 *
 * returns: RC_OK on success, RC_IM_NO_MORE_ENTRIES if no more entries, RC_IM_KEY_NOT_FOUND otherwise.
 */
RC nextEntry(BT_ScanHandle *handle, RID *result)
{
    if (handle != NULL)
    {
        int totalRes = 0;
        getNumEntries((*handle).tree, &totalRes); // Get total number of entries in the B-tree

        RM_BtreeNode *cursor = NULL;
        RM_BScan_mgmt *scanMgmt = (RM_BScan_mgmt *)(*handle).mgmtData;

        if ((*scanMgmt).tot_Scan < totalRes)
        {
            if ((*scanMgmt).tot_Scan == 0)
            {
                updateScanMgmtCurrent(rootNode, scanMgmt); // Initialize the scan with root node
            }

            // Move the cursor to the next node if needed
            cursor = (RM_BtreeNode *)(*scanMgmt).cur->ptr[((RM_bTree_mgmtData *)handle->tree->mgmtData)->maxKeyNum];

            if (isIndexEqualsKeyCounts(scanMgmt))
            {
                updateScanMgmtCurrentAndIndex(cursor, scanMgmt, 0); // Update scan management data
            }
            RID *ridRes = (RID *)(*scanMgmt).cur->ptr[(*scanMgmt).index];

            if (ridRes != NULL)
            {
                ++(*scanMgmt).index;        // Increment the current index
                ++(*scanMgmt).tot_Scan;    // Increment the total scanned entries
                (*handle).mgmtData = scanMgmt;
            }

            if ((*ridRes).slot != -1)
            {
                (*result).page = (*ridRes).page; // Set the page number in result
                (*result).slot = (*ridRes).slot; // Set the slot number in result
            }

            return RC_OK;
        }
        return RC_IM_NO_MORE_ENTRIES; // No more entries to scan
    }
    return RC_IM_KEY_NOT_FOUND; // Return error if handle is NULL
}

/* 
 * Function: closeTreeScan
 * -----------------------
 * Closes an ongoing B-tree scan and frees associated resources.
 *
 * handle: Pointer to the BT_ScanHandle to be closed.
 *
 * returns: RC_OK indicating successful closure.
 */
RC closeTreeScan(BT_ScanHandle *handle)
{
    freeRM_BScan((*handle).mgmtData); // Free scan management data
    freeBT_ScanHandle(handle);         // Free the scan handle
    return RC_OK;
}

/* 
 * Dummy Function: dummyFunction8
 * --------------------------------
 * A placeholder function added as per request.
 */
void dummyFunction8() {
    // This is an eighth dummy function.
}
