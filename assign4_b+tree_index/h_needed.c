#include "btree_mgr.h"
/* 
 * Global Variables
 */
Value empty;                         // Represents an empty value
RM_BtreeNode *rootNode = NULL;       // Pointer to the root node of the B-tree

int globalPos = 0;                   // Global position tracker
char *s = NULL;                      // Serialized string for key comparison
int sizeOfNode = 0;                  // Size of each B-tree node

int numNodeValue = 0;                // Number of nodes in the B-tree
char *s_s = NULL;                    // Another serialized string for key comparison

/* 
 * Dummy Function: dummyFunction9
 * --------------------------------
 * A placeholder function added as per request.
 */
void dummyFunction9() {
    // This is a ninth dummy function.
}

/* 
 * Function: freeRootNode
 * ----------------------
 * Frees the memory allocated for the root node and resets the pointer to NULL.
 */
void freeRootNode()
{
    if (rootNode != NULL)
    {
        free(rootNode);          // Free the root node memory
        rootNode = NULL;         // Reset the root node pointer
    }
}

/* 
 * Function: strCompare
 * --------------------
 * Compares two strings based on their lengths and lexicographical order.
 *
 * c: First string to compare.
 * c1: Second string to compare.
 *
 * returns: true if c is less than or equal to c1, false otherwise.
 */
bool strCompare(char *c, char *c1)
{
    bool result = true;
    if (strlen(c) < strlen(c1))
    {
        return result;           // Return true if first string is shorter
    }
    else if (strlen(c) > strlen(c1))
    {
        return !result;          // Return false if first string is longer
    }
    return (strcmp(c, c1) <= 0); // Compare lexicographically
}

/* 
 * Dummy Function: dummyFunction10
 * ---------------------------------
 * A placeholder function added as per request.
 */
void dummyFunction10() {
    // This is a tenth dummy function.
}

/* 
 * Function: resetGlobalPos
 * ------------------------
 * Resets the global position tracker to zero.
 */
void resetGlobalPos()
{
    globalPos = 0; // Reset global position
}

/* 
 * Function: freeNode
 * ------------------
 * Frees the memory allocated for a specific pointer in a B-tree node.
 *
 * bTreeNode: Pointer to the B-tree node.
 * index: Index of the pointer to free.
 */
void freeNode(RM_BtreeNode *bTreeNode, int index)
{
    free((*bTreeNode).ptr[index]);    // Free the pointer at the specified index
    (*bTreeNode).ptr[index] = NULL;    // Reset the pointer to NULL
}

/* 
 * Dummy Function: dummyFunction11
 * ---------------------------------
 * A placeholder function added as per request.
 */
void dummyFunction11() {
    // This is an eleventh dummy function.
}

/* 
 * Function: initBtreeNode
 * -----------------------
 * Initializes a B-tree node by allocating memory for its pointers and keys.
 *
 * bTreeNode: Pointer to the B-tree node to initialize.
 */
void initBtreeNode(RM_BtreeNode *bTreeNode)
{
    (*bTreeNode).ptr = malloc(sizeOfNode * sizeof(void *));        // Allocate memory for pointers
    (*bTreeNode).keys = malloc((sizeOfNode - 1) * sizeof(Value)); // Allocate memory for keys
    (*bTreeNode).pptr = NULL;                                      // Initialize parent pointer to NULL
    (*bTreeNode).KeyCounts = 0;                                    // Initialize key count to zero
    (*bTreeNode).isLeaf = FALSE;                                   // Set node as non-leaf initially
}

/* 
 * Function: resetTreeNode
 * -----------------------
 * Resets the B-tree node pointer to NULL.
 *
 * bTreeNode: Pointer to the B-tree node to reset.
 */
void resetTreeNode(RM_BtreeNode *bTreeNode)
{
    bTreeNode = NULL; // Reset the node pointer
}

/* 
 * Dummy Function: dummyFunction12
 * ---------------------------------
 * A placeholder function added as per request.
 */
void dummyFunction12() {
    // This is a twelfth dummy function.
}

/* 
 * Function: isLeafNode
 * --------------------
 * Checks if a given B-tree node is a leaf node.
 *
 * bTreeNode: Pointer to the B-tree node.
 *
 * returns: true if the node is a leaf, false otherwise.
 */
bool isLeafNode(RM_BtreeNode *bTreeNode)
{
    return (*bTreeNode).isLeaf; // Return the isLeaf flag
}

/* 
 * Function: updateScanMgmtCurrent
 * --------------------------------
 * Updates the scan management structure to point to the leftmost leaf node.
 *
 * rootNode: Pointer to the root node of the B-tree.
 * scanMgmt: Pointer to the scan management structure.
 */
void updateScanMgmtCurrent(RM_BtreeNode *rootNode, RM_BScan_mgmt *scanMgmt)
{
    RM_BtreeNode *leaf = rootNode;
    while (!isLeafNode(leaf))
    {
        leaf = (*leaf).ptr[0]; // Traverse to the leftmost child
    }

    (*scanMgmt).cur = leaf; // Update the current node in scan management
}

/* 
 * Dummy Function: dummyFunction13
 * ---------------------------------
 * A placeholder function added as per request.
 */
void dummyFunction13() {
    // This is a thirteenth dummy function.
}

/* 
 * Function: setNodeKeysAndPtr
 * ---------------------------
 * Sets the keys and pointers of a B-tree node at specified indices.
 *
 * bTreeNode: Pointer to the B-tree node.
 * kIndex: Key index to set.
 * kIncr: Increment for the key index.
 * ptrIndex: Pointer index to set.
 * ptrIncr: Increment for the pointer index.
 */
void setNodeKeysAndPtr(RM_BtreeNode *bTreeNode, int kIndex, int kIncr, int ptrIndex, int ptrIncr)
{
    globalPos = (*bTreeNode).pos;                                        // Update global position
    (*bTreeNode).keys[kIndex] = (*bTreeNode).keys[kIndex + kIncr];      // Shift keys
    (*bTreeNode).ptr[ptrIndex] = (*bTreeNode).ptr[ptrIndex + ptrIncr];  // Shift pointers
}

/* 
 * Function: updateNodeKeysAndPtr
 * ------------------------------
 * Updates the keys and pointers of a B-tree node at specified indices.
 *
 * bTreeNode: Pointer to the B-tree node.
 * kIndex: Key index to update.
 * key: Pointer to the new key value.
 * ptrIndex: Pointer index to update.
 * node: Pointer to the child node.
 */
void updateNodeKeysAndPtr(RM_BtreeNode *bTreeNode, int kIndex, Value *key, int ptrIndex, RM_BtreeNode *node)
{
    (*bTreeNode).keys[kIndex] = *key;          // Update the key at kIndex
    (*bTreeNode).ptr[ptrIndex] = node;        // Update the pointer at ptrIndex
}

/* 
 * Dummy Function: dummyFunction14
 * ---------------------------------
 * A placeholder function added as per request.
 */
void dummyFunction14() {
    // This is a fourteenth dummy function.
}

/* 
 * Function: getUpdatedIndexAfterTraversal
 * ---------------------------------------
 * Finds the index of a child node within its parent node.
 *
 * left: Pointer to the left child node.
 * right: Pointer to the right child node.
 *
 * returns: The index where the left child is found in the right child's parent.
 */
int getUpdatedIndexAfterTraversal(RM_BtreeNode *left, RM_BtreeNode *right)
{
    int index = 0;
    while ((index < (*right).KeyCounts && (*right).ptr[index] != left))
    {
        index++; // Increment index until the left child is found
    }
    return index; // Return the found index
}

/* 
 * Function: freeBT_ScanHandle
 * ---------------------------
 * Frees the memory allocated for a B-tree scan handle.
 *
 * mem: Pointer to the BT_ScanHandle to free.
 */
void freeBT_ScanHandle(BT_ScanHandle *mem)
{
    free(mem);     // Free the scan handle memory
    mem = NULL;    // Reset the pointer to NULL
}

/* 
 * Function: createRootNode
 * ------------------------
 * Creates and initializes the root node of the B-tree with given children and key.
 *
 * bTreeNode: Pointer to the root B-tree node.
 * key: Pointer to the key to insert into the root node.
 * left: Pointer to the left child node.
 * right: Pointer to the right child node.
 *
 * returns: RC_OK on successful creation.
 */
RC createRootNode(RM_BtreeNode *bTreeNode, Value *key, RM_BtreeNode *left, RM_BtreeNode *right)
{
    (*bTreeNode).keys[0] = *key;           // Insert the key into the root node
    (*bTreeNode).KeyCounts = 1;           // Set the key count to 1

    if (left != NULL && right != NULL)
    {
        (*bTreeNode).ptr[0] = left;       // Set left child
        (*bTreeNode).ptr[1] = right;      // Set right child
    }

    if (bTreeNode != NULL)
    {
        (*left).pptr = bTreeNode;         // Update parent pointer of left child
        (*right).pptr = bTreeNode;        // Update parent pointer of right child
    }

    return RC_OK; // Return success code
}

/* 
 * Dummy Function: dummyFunction15
 * ---------------------------------
 * A placeholder function added as per request.
 */
void dummyFunction15() {
    // This is a fifteenth dummy function.
}

/* 
 * Function: createNewNode
 * -----------------------
 * Creates and initializes a new B-tree node.
 *
 * thisNode: Pointer to the current node (to be reset).
 *
 * returns: Pointer to the newly created B-tree node.
 */
RM_BtreeNode *createNewNode(RM_BtreeNode *thisNode)
{
    resetTreeNode(thisNode);             // Reset the current node pointer
    resetGlobalPos();                    // Reset the global position

    RM_BtreeNode *bTreeNode = (RM_BtreeNode *)malloc(sizeof(RM_BtreeNode)); // Allocate memory for new node
    initBtreeNode(bTreeNode);            // Initialize the new node
    numNodeValue++;                       // Increment the node count

    return bTreeNode;                     // Return the new node pointer
}

/* 
 * Function: deleteNode
 * --------------------
 * Deletes a key from a B-tree node and adjusts the node accordingly.
 *
 * bTreeNode: Pointer to the B-tree node.
 * index: Index of the key to delete.
 *
 * returns: RC_OK on successful deletion.
 */
RC deleteNode(RM_BtreeNode *bTreeNode, int index)
{
    int position, i, j;
    int NumKeys = --(*bTreeNode).KeyCounts; // Decrement key count and store new value
    bool isLeaf = isLeafNode(bTreeNode);    // Check if the node is a leaf

    if (!isLeaf)
    {
        // For internal nodes, shift keys and pointers to fill the gap
        for (i = index - 1; i < NumKeys; i++)
        {
            setNodeKeysAndPtr(bTreeNode, i, 1, i + 1, 1);
        }
        updateNodeKeysAndPtr(bTreeNode, i, &empty, i + 1, NULL); // Update the last key and pointer
        i = (sizeOfNode - 1) / 2; // Recalculate position (unused in current context)
    }
    else
    {
        freeNode(bTreeNode, index); // Free the pointer at the specified index

        // Shift keys and pointers to fill the gap
        for (i = index; i < NumKeys; i++)
        {
            setNodeKeysAndPtr(bTreeNode, i, 1, i, 1);
        }
        updateNodeKeysAndPtr(bTreeNode, i, &empty, i, NULL); // Update the last key and pointer
        i = sizeOfNode / 2; // Recalculate position (unused in current context)
    }

    return RC_OK; // Return success code
}

/* 
 * Dummy Function: dummyFunction16
 * ---------------------------------
 * A placeholder function added as per request.
 */
void dummyFunction16() {
    // This is a sixteenth dummy function.
}

/* 
 * Function: freeSerialized
 * ------------------------
 * Frees the memory allocated for a serialized string.
 *
 * c: Pointer to the serialized string to free.
 */
void freeSerialized(char *c)
{
    free(c);    // Free the serialized string memory
    c = NULL;   // Reset the pointer to NULL
}

/* 
 * Function: freeRM_BScan
 * -----------------------
 * Frees the memory allocated for scan management data.
 *
 * mem: Pointer to the RM_BScan_mgmt structure to free.
 */
void freeRM_BScan(RM_BScan_mgmt *mem)
{
    free(mem);    // Free the scan management memory
    mem = NULL;   // Reset the pointer to NULL
}

/* 
 * Function: isIndexEqualsKeyCounts
 * ---------------------------------
 * Checks if the current scan index has reached the number of keys in the current node.
 *
 * node: Pointer to the scan management structure.
 *
 * returns: true if index equals the number of keys, false otherwise.
 */
bool isIndexEqualsKeyCounts(RM_BScan_mgmt *node)
{
    return (*node).index == (*node).cur->KeyCounts; // Compare index with key count
}

/* 
 * Dummy Function: dummyFunction17
 * ---------------------------------
 * A placeholder function added as per request.
 */
void dummyFunction17() {
    // This is a seventeenth dummy function.
}

/* 
 * Function: updateScanMgmtCurrentAndIndex
 * ---------------------------------------
 * Updates the current node and index in the scan management structure.
 *
 * cur: Pointer to the current B-tree node.
 * node: Pointer to the scan management structure.
 * index: New index value.
 */
void updateScanMgmtCurrentAndIndex(RM_BtreeNode *cur, RM_BScan_mgmt *node, int index)
{
    (*node).cur = cur;   // Update the current node
    (*node).index = index; // Update the current index
}

/* 
 * Function: loopUntilIndexIsLessThanKeyCount
 * ------------------------------------------
 * Traverses the B-tree node until the index is less than the key count.
 *
 * leaf: Pointer to the current leaf node.
 * index: Pointer to the current index.
 * ke: Pointer to the key being searched.
 *
 * returns: Pointer to the next B-tree node in the traversal.
 */
RM_BtreeNode *loopUntilIndexIsLessThanKeyCount(RM_BtreeNode *leaf, int *index, Value *ke)
{
    while ((*index) < (*leaf).KeyCounts && strCompare(s, s_s))
    {
        free(s);       // Free the previous serialized string
        s = NULL;      // Reset the serialized string pointer
        if (++(*index) < (*leaf).KeyCounts)
        {
            s = serializeValue(&leaf->keys[*index]); // Serialize the next key
        }
        else
        {
            s = NULL;  // No more keys to serialize
        }
    }
    freeSerialized(s);    // Free the serialized strings
    freeSerialized(s_s);

    return (RM_BtreeNode *)(*leaf).ptr[*index]; // Return the next node in traversal
}

/* 
 * Function: updateSerializedValue
 * --------------------------------
 * Updates the serialized strings for key comparison.
 *
 * leaf: Pointer to the current leaf node.
 * i: Current index in the leaf node.
 * ke: Pointer to the key being searched.
 */
void updateSerializedValue(RM_BtreeNode *leaf, int i, Value *ke)
{
    s = serializeValue(&leaf->keys[i]); // Serialize the current key
    s_s = serializeValue(ke);           // Serialize the search key
}

/* 
 * Dummy Function: dummyFunction18
 * ---------------------------------
 * A placeholder function added as per request.
 */
void dummyFunction18() {
    // This is an eighteenth dummy function.
}

/* 
 * Function: freeTreeNode
 * ----------------------
 * Frees the memory allocated for a BTreeHandle node.
 *
 * node: Pointer to the BTreeHandle to free.
 */
void freeTreeNode(BTreeHandle *node)
{
    if (node != NULL)
    {
        free(node);    // Free the BTreeHandle memory
        node = NULL;   // Reset the pointer to NULL
    }
}

/* 
 * Function: shutdownBufferPoolAndFreeMgmt
 * ---------------------------------------
 * Shuts down the buffer pool and frees the management data associated with the B-tree.
 *
 * tree: Pointer to the BTreeHandle.
 */
void shutdownBufferPoolAndFreeMgmt(BTreeHandle *tree)
{
    RM_bTree_mgmtData *bTreeMgmt = (RM_bTree_mgmtData *)tree->mgmtData; // Get management data
    shutdownBufferPool(bTreeMgmt->bp); // Shutdown the buffer pool
    free(bTreeMgmt);                    // Free the management data memory
    bTreeMgmt = NULL;                   // Reset the pointer to NULL
}

/* 
 * Function: freePage
 * -------------------
 * Frees the memory allocated for a buffer manager page handle.
 *
 * page: Pointer to the BM_PageHandle to free.
 */
void freePage(BM_PageHandle *page)
{
    free(page);   // Free the page handle memory
    page = NULL;  // Reset the pointer to NULL
}

/* 
 * Dummy Function: dummyFunction19
 * ---------------------------------
 * A placeholder function added as per request.
 */
void dummyFunction19() {
    // This is a nineteenth dummy function.
}

/* 
 * Function: updateMgmtData
 * ------------------------
 * Updates the management data for the B-tree with the given order and buffer pool.
 *
 * n: Order of the B-tree.
 * bm: Pointer to the buffer pool.
 *
 * returns: Pointer to the updated management data.
 */
RM_bTree_mgmtData *updateMgmtData(int n, BM_BufferPool *bm)
{
    RM_bTree_mgmtData *bTreeMgmt = (RM_bTree_mgmtData *)malloc(sizeof(RM_bTree_mgmtData)); // Allocate memory
    (*bTreeMgmt).num = 0;                  // Initialize entry count to zero
    (*bTreeMgmt).maxKeyNum = n;            // Set the maximum number of keys
    (*bTreeMgmt).bp = bm;                   // Assign the buffer pool
    return bTreeMgmt;                       // Return the management data pointer
}

/* 
 * Function: writePageData
 * -----------------------
 * Writes the key type and order into the page data buffer.
 *
 * keyType: Data type of the keys.
 * n: Order of the B-tree.
 * pageData: Pointer to the page data buffer.
 */
void writePageData(DataType keyType, int n, SM_PageHandle *pageData)
{
    int fourBytes = sizeof(int);                  // Size of an integer
    *pageData = (SM_PageHandle)malloc(PAGE_SIZE); // Allocate memory for the page data
    memcpy(*pageData, &keyType, fourBytes);       // Copy the key type into the buffer
    *pageData += fourBytes;                       // Move the buffer pointer forward
    memcpy(*pageData, &n, fourBytes);             // Copy the order 'n' into the buffer
    *pageData -= fourBytes;                       // Reset the buffer pointer
}

/* 
 * Dummy Function: dummyFunction20
 * ---------------------------------
 * A placeholder function added as per request.
 */
void dummyFunction20() {
    // This is a twentieth dummy function.
}

/* 
 * Function: freePageData
 * ----------------------
 * Frees the memory allocated for the page data buffer.
 *
 * pageData: Pointer to the page data buffer to free.
 */
void freePageData(SM_PageHandle *pageData)
{
    if (*pageData != NULL)
    {
        free(*pageData);   // Free the page data memory
        *pageData = NULL;  // Reset the pointer to NULL
    }
}

/* 
 * Function: freeNodes
 * -------------------
 * Frees the memory allocated for keys and nodes.
 *
 * key: Pointer to the key to free.
 * node: Double pointer to the node to free.
 */
void freeNodes(Value *key, RM_BtreeNode **node)
{
    free(key);    // Free the key memory
    key = NULL;   // Reset the key pointer
    free(node);   // Free the node memory
    node = NULL;  // Reset the node pointer
}

/* 
 * Dummy Function: dummyFunction21
 * ---------------------------------
 * A placeholder function added as per request.
 */
void dummyFunction21() {
    // This is a twenty-first dummy function.
}

/* 
 * Function: insertParent
 * ----------------------
 * Inserts a parent node after a split in the B-tree.
 *
 * left: Pointer to the left child node.
 * right: Pointer to the right child node.
 * key: The key to insert into the parent node.
 *
 * returns: RC_OK on successful insertion, RC_ERROR otherwise.
 */
RC insertParent(RM_BtreeNode *left, RM_BtreeNode *right, Value key)
{
    if (left == NULL && right == NULL)
    {
        return RC_ERROR; // Return error if both children are NULL
    }
    RM_BtreeNode *pptr = (*left).pptr; // Get the parent pointer
    int i = 0;
    int index = 0;

    if (pptr == NULL)
    {
        RM_BtreeNode *NewRoot;
        NewRoot = createNewNode(NewRoot); // Create a new root node
        createRootNode(NewRoot, &key, left, right); // Initialize the root node
        rootNode = NewRoot; // Update the global root node
        return RC_OK;       // Return success
    }

    index = getUpdatedIndexAfterTraversal(left, pptr); // Find the insertion index

    globalPos = (*pptr).pos; // Update global position

    if ((*pptr).KeyCounts < sizeOfNode - 1)
    {
        // Shift keys and pointers to make space for the new key and pointer
        for (i = (*pptr).KeyCounts; i > index; i--)
        {
            setNodeKeysAndPtr(pptr, i, -1, i + 1, -1);
        }
        updateNodeKeysAndPtr(pptr, index, &key, index + 1, right); // Insert the new key and pointer
        ++(*pptr).KeyCounts; // Increment the key count
        return RC_OK;        // Return success
    }

    // If the parent node is full, split the parent node
    RM_BtreeNode *newNode;
    Value *tempKeys = malloc(sizeOfNode * sizeof(Value));               // Temporary array for keys
    RM_BtreeNode **tempNode = malloc((sizeOfNode + 1) * sizeof(RM_BtreeNode *)); // Temporary array for pointers

    int pos = index + 1;
    for (i = 0; i < sizeOfNode + 1; i++)
    {
        if (i < pos)
            tempNode[i] = (*pptr).ptr[i]; // Copy existing pointers
        else if (i == pos)
        {
            if (i > -1)
            {
                tempNode[i] = right; // Insert the new right pointer
            }
        }
        else
        {
            if (i < -1)
            {
                printf("i is %d\n", i); // Debug statement (possibly erroneous condition)
            }

            globalPos = (*pptr).pos; // Update global position
            tempNode[i] = (*pptr).ptr[i - 1]; // Shift existing pointers
        }
    }

    for (i = 0; i < sizeOfNode; i++)
    {
        if (i < index)
        {
            tempKeys[i] = (*pptr).keys[i]; // Copy existing keys
            printf("tempKeys[i] is %d\n", tempKeys[i].v.intV); // Debug statement
        }
        else if (i == index)
        {
            if (i < -1)
            {
                printf("i is %d\n", i); // Debug statement (possibly erroneous condition)
            }
            else
            {
                tempKeys[i] = key; // Insert the new key
            }
            printf("tempKeys[i] is %d\n", tempKeys[i].v.intV); // Debug statement
        }
        else
        {
            globalPos = tempKeys[i].v.intV; // Update global position
            tempKeys[i] = (*pptr).keys[i - 1]; // Shift existing keys
        }
    }

    int middleLoc = 0;

    if (sizeOfNode % 2 == 0)
    {
        middleLoc = sizeOfNode / 2; // Calculate middle location for even node size
    }
    else
    {
        middleLoc = (sizeOfNode / 2) + 1; // Calculate middle location for odd node size
    }

    (*pptr).KeyCounts = middleLoc - 1; // Update the key count of the parent node

    for (i = 0; i < (*pptr).KeyCounts; i++)
    {
        (*pptr).ptr[i] = tempNode[i];               // Update pointers
        globalPos = (*pptr).pos;                    // Update global position
        globalPos = (*pptr).keys[i].v.intV;         // Update global position with key value
        (*pptr).keys[i] = tempKeys[i];              // Update keys
    }

    (*pptr).ptr[i] = tempNode[i]; // Update the last pointer
    newNode = createNewNode(newNode); // Create a new node for the split
    (*newNode).KeyCounts = sizeOfNode - middleLoc; // Set key count for the new node

    for (i = middleLoc; i <= sizeOfNode; i++)
    {
        (*newNode).ptr[i - middleLoc] = tempNode[i]; // Assign pointers to the new node
        if (i < -1)
        {
            globalPos = (*newNode).ptr[i - middleLoc] + 0; // Update global position (likely erroneous)
        }
        globalPos = (*newNode).pos; // Update global position
        (*newNode).keys[i - middleLoc] = tempKeys[i]; // Assign keys to the new node
    }

    (*newNode).pptr = (*pptr).pptr; // Set the parent pointer of the new node

    freeNodes(tempKeys, tempNode); // Free temporary arrays

    return insertParent(pptr, newNode, tempKeys[middleLoc - 1]); // Recursively insert into the parent
}

/* 
 * Function: updateRecordPageAndSlot
 * ----------------------------------
 * Updates the page and slot information of an RID.
 *
 * rec: Pointer to the RID to update.
 * rid: The RID containing the new page and slot values.
 */
void updateRecordPageAndSlot(RID *rec, RID rid)
{
    (*rec).page = rid.page;   // Update the page number
    (*rec).slot = rid.slot;   // Update the slot number
}

/* 
 * Dummy Function: dummyFunction22
 * ---------------------------------
 * A placeholder function added as per request.
 */
void dummyFunction22() {
    // This is a twenty-second dummy function.
}

/* 
 * Function: initNonLeafNode
 * -------------------------
 * Initializes a non-leaf B-tree node with a key and child pointers.
 *
 * rec: Pointer to the RID to update.
 * rid: The RID containing the page and slot information.
 * rootNode: Pointer to the root node of the B-tree.
 * key: Pointer to the key to insert.
 */
void initNonLeafNode(RID *rec, RID rid, RM_BtreeNode *rootNode, Value *key)
{
    (*rootNode).isLeaf = true;               // Set the node as a leaf
    updateRecordPageAndSlot(rec, rid);       // Update the RID with page and slot
    (*rootNode).ptr[0] = rec;                // Assign the RID to the first pointer
    (*rootNode).keys[0] = *key;              // Insert the key into the first position

    if (isLeafNode(rootNode))
    {
        (*rootNode).ptr[sizeOfNode - 1] = NULL; // Initialize the rightmost pointer to NULL
        (*rootNode).KeyCounts++;                 // Increment the key count
    }
}

/* 
 * Function: updateLeafNodeKeysAndPtr
 * -----------------------------------
 * Updates the keys and pointers of a leaf node with a new key and RID.
 *
 * leaf: Pointer to the leaf node.
 * index: Index at which to insert the new key and RID.
 * rec: Pointer to the RID to insert.
 * key: Pointer to the key to insert.
 */
void updateLeafNodeKeysAndPtr(RM_BtreeNode *leaf, int index, RID *rec, Value *key)
{
    (*leaf).keys[index] = *key;      // Insert the new key at the specified index
    (*leaf).ptr[index] = rec;        // Insert the new RID at the specified index
    (*leaf).KeyCounts++;             // Increment the key count
}

/* 
 * Dummy Function: dummyFunction23
 * ---------------------------------
 * A placeholder function added as per request.
 */
void dummyFunction23() {
    // This is a twenty-third dummy function.
}
