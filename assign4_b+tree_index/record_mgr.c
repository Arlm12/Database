#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "record_mgr.h"
#include "buffer_mgr.h"
#include "storage_mgr.h"
#include <ctype.h>
#include <unistd.h>

const int MAX_NUMBER_OF_PAGES = 100;
int ATTR_SIZE = 15;
RecManager *recMgr;

int findFreeSlot(char *data, int recordSize)
{
    int index = 0;
    int maxSlotSize = PAGE_SIZE / recordSize;
    int slot = 0;

    while (slot < maxSlotSize)
    {
        index = slot * recordSize;
        if (data[index] != '+')
        {
            return slot;
        }
        slot++;
    }

    return -1;
}

extern RC initRecordManager(void *mgmtData)
{
    initStorageManager();
}

void freeRecordManager()
{
    recMgr = NULL;
    free(recMgr);
}

extern RC shutdownRecordManager()
{
    shutdownBufferPool(&recMgr->buffPool);
    freeRecordManager();
    return RC_OK;
}

bool isRetCodeOk(RC retCode)
{
    return retCode != RC_OK;
}

RC getRetCode(char *name, SM_FileHandle fileHandler, char *data)
{
    int retCode = createPageFile(name);
    if (isRetCodeOk(retCode))
        return retCode;

    retCode = openPageFile(name, &fileHandler);
    if (isRetCodeOk(retCode))
        return retCode;

    retCode = writeBlock(0, &fileHandler, data);
    if (isRetCodeOk(retCode))
        return retCode;

    retCode = closePageFile(&fileHandler);
    if (isRetCodeOk(retCode))
        return retCode;

    return RC_OK;
}

void setPageHandle(char **pageHandle, int buf, int value)
{
    *(int *)*pageHandle = value;
    *pageHandle = *pageHandle + buf;
}

void setNumOfAttr(int numOfAttr, char **pageHandle)
{
    *(int *)*pageHandle = numOfAttr;
    *pageHandle = *pageHandle + sizeof(int);
}

void setSchemaKeySize(int schemaKeySize, char **pageHandle)
{
    *(int *)*pageHandle = schemaKeySize;
    *pageHandle = *pageHandle + sizeof(int);
}

void loopThroughAttrs(Schema *schema, char *pageHandle)
{
    int totalNoOfAttrs = schema->numAttr;
    for (int attr = 0; attr < totalNoOfAttrs; attr++)
    {
        char *attrName = schema->attrNames[attr];
        strncpy(pageHandle, attrName, ATTR_SIZE);
        pageHandle += ATTR_SIZE;
        int attrDataType = (int)schema->dataTypes[attr];
        *(int *)pageHandle = attrDataType;
        pageHandle += sizeof(int);
        int attrTypeLength = (int)schema->typeLength[attr];
        *(int *)pageHandle = attrTypeLength;
        pageHandle += sizeof(int);
    }
}

extern RC createTable(char *name, Schema *schema)
{

    char data[PAGE_SIZE];
    char *pageHandle = data;
    SM_FileHandle fileHandler;
    recMgr = (RecManager *)malloc(sizeof(RecManager));
    initBufferPool(&recMgr->buffPool, name, 100, RS_LRU, NULL);
    setPageHandle(&pageHandle, sizeof(int), 0);
    setPageHandle(&pageHandle, sizeof(int), 1);
    setNumOfAttr(schema->numAttr, &pageHandle);
    setSchemaKeySize(schema->keySize, &pageHandle);
    loopThroughAttrs(schema, pageHandle);
    return getRetCode(name, fileHandler, data);
}

void initOpenTable(RM_TableData **rel, char **name)
{
    (*rel)->name = *name;
    (*rel)->mgmtData = recMgr;
}

void initOpenTablePageHandle(RM_TableData **rel, char **pageHandle, int *attributeCount)
{
    int buf = sizeof(int);
    *pageHandle = (char *)recMgr->pgHandle.data;
    (*recMgr).tupCount = *(int *)*pageHandle;
    *pageHandle += buf;
    (*recMgr).freePg = *(int *)*pageHandle;
    *pageHandle = *pageHandle + buf;
    *attributeCount = *(int *)*pageHandle;
    *pageHandle += buf;
}

void allocateMemoryForSchema(Schema **relSchema, int attributeCount)
{
    (**relSchema).attrNames = (char **)malloc(sizeof(char *) * attributeCount);
    (*relSchema)->numAttr = attributeCount;
    (**relSchema).dataTypes = (DataType *)malloc(sizeof(DataType) * attributeCount);
    (*relSchema)->typeLength = (int *)malloc(sizeof(int) * attributeCount);
}

void resetAttrCount(int *attrCount)
{
    *attrCount = 0;
}

void whileAttrIsLessThanNumAttr(int *attr, int numAttr, char **pageHandle, Schema **relSchema)
{
    while (*attr < numAttr)
    {
        (*relSchema)->attrNames[*attr] = *pageHandle;
        *pageHandle += ATTR_SIZE;
        (*relSchema)->dataTypes[*attr] = *(int *)*pageHandle;
        *pageHandle += sizeof(int);
        (*relSchema)->typeLength[*attr] = *(int *)*pageHandle;
        *pageHandle += sizeof(int);
        *attr += 1;
    }
}

extern RC openTable(RM_TableData *rel, char *name)
{
    SM_PageHandle pageHandle;
    int attributeCount;
    int attr;
    int ATTR_SIZE = 15;

    resetAttrCount(&attr);

    initOpenTable(&rel, &name);

    pinPage(&recMgr->buffPool, &recMgr->pgHandle, 0);
    initOpenTablePageHandle(&rel, &pageHandle, &attributeCount);

    Schema *relSchema = (Schema *)malloc(sizeof(Schema));
    allocateMemoryForSchema(&relSchema, attributeCount);

    while (attr < attributeCount)
    {
        relSchema->attrNames[attr] = (char *)malloc(ATTR_SIZE);
        attr += 1;
    }

    resetAttrCount(&attr);

    whileAttrIsLessThanNumAttr(&attr, attributeCount, &pageHandle, &relSchema);

    (*rel).schema = relSchema;

    unpinPage(&recMgr->buffPool, &recMgr->pgHandle);
    forcePage(&recMgr->buffPool, &recMgr->pgHandle);

    return RC_OK;
}

extern RC closeTable(RM_TableData *rel)
{
    if (rel == NULL)
    {
        return RC_OK;
    }
    else
    {
        if (rel->mgmtData != NULL)
        {
            RecManager *temp_recMgr = rel->mgmtData;
            shutdownBufferPool(&(temp_recMgr)->buffPool);
        }
        return RC_OK;
    }
}

extern RC deleteTable(char *name)
{
    return destroyPageFile(name);
}

extern int getNumTuples(RM_TableData *rel)
{
    RecManager *temp_recMgr = rel->mgmtData;
    return temp_recMgr->tupCount;
}

bool isRelAndRecordNull(RM_TableData *rel, Record *record)
{
    return rel != NULL && record != NULL;
}

void setRecordPageAndDataAndSlot(RID **recID, RecManager **tempRecMgr, int recordSize, char **data)
{
    (**recID).page = (**tempRecMgr).freePg;
    pinPage(&(*tempRecMgr)->buffPool, &(*tempRecMgr)->pgHandle, (*recID)->page);
    *data = (**tempRecMgr).pgHandle.data;
    (*recID)->slot = findFreeSlot(*data, recordSize);
}

void whileRecordIdSlotIsNegativeOne(RID **recID, RecManager **tempRecMgr, int recordSize, char **data)
{
    while ((*recID)->slot == -1)
    {
        unpinPage(&(*tempRecMgr)->buffPool, &(*tempRecMgr)->pgHandle);
        (*recID)->page = (*recID)->page + 1;
        pinPage(&(*tempRecMgr)->buffPool, &(*tempRecMgr)->pgHandle, (*recID)->page);
        *data = (*tempRecMgr)->pgHandle.data;
        (*recID)->slot = findFreeSlot(*data, recordSize);
    }
}

bool canMakeDirty(RecManager **tempRecMgr, RID **recID)
{
    return (*tempRecMgr)->freePg == (*recID)->page;
}

extern RC insertRecord(RM_TableData *rel, Record *record)
{
    char *data, *newData;
    bool isNotNull = isRelAndRecordNull(rel, record);
    if (isNotNull)
    {
        RID *recID = &record->id;
        RecManager *tempRecMgr = rel->mgmtData;
        int recordSize = getRecordSize(rel->schema);

        setRecordPageAndDataAndSlot(&recID, &tempRecMgr, recordSize, &data);
        whileRecordIdSlotIsNegativeOne(&recID, &tempRecMgr, recordSize, &data);

        newData = data != NULL && recID->slot != -1 ? data : NULL;

        if (canMakeDirty(&tempRecMgr, &recID))
        {
            markDirty(&tempRecMgr->buffPool, &tempRecMgr->pgHandle);
        }

        newData = newData + recID->slot * recordSize;
        *newData = '+';
        memcpy(++newData, (*record).data + 1, recordSize - 1);

        tempRecMgr->tupCount = tempRecMgr->tupCount + 1;
    }

    return RC_OK;
}

void whileCurPointerIsLessThanRecordSize(int recordSize, char *data, RID id)
{
    int curPointer = 0;
    while (curPointer < recordSize)
    {
        int index = id.slot * recordSize;
        data[index + curPointer] = '-';
        curPointer += 1;
    }
}

void makeDirtyUnpinAndForcePage(RecManager *tempRecMgr, bool shouldForce)
{
    markDirty(&tempRecMgr->buffPool, &tempRecMgr->pgHandle);
    unpinPage(&tempRecMgr->buffPool, &tempRecMgr->pgHandle);
    if (shouldForce)
    {
        forcePage(&tempRecMgr->buffPool, &tempRecMgr->pgHandle);
    }
}

extern RC deleteRecord(RM_TableData *rel, RID id)
{
    RecManager *tempRecMgr = rel->mgmtData;

    pinPage(&tempRecMgr->buffPool, &tempRecMgr->pgHandle, id.page);
    (*tempRecMgr).freePg = id.page;
    char *data = (*tempRecMgr).pgHandle.data;

    int recordSize = getRecordSize((*rel).schema);
    whileCurPointerIsLessThanRecordSize(recordSize, data, id);
    makeDirtyUnpinAndForcePage(tempRecMgr, true);

    return RC_OK;
}

void copyDataToRecord(char **data, Record **record, int recordSize, int dataPos)
{
    *data += dataPos;
    **data = '+';
    memcpy(++(*data), (**record).data + 1, recordSize - 1);
}

extern RC updateRecord(RM_TableData *rel, Record *record)
{
    RecManager *tempRecMgr = rel->mgmtData;

    pinPage(&tempRecMgr->buffPool, &tempRecMgr->pgHandle, (*record).id.page);

    RID id = (*record).id;
    char *data = (*tempRecMgr).pgHandle.data;
    int recordSize = getRecordSize((*rel).schema);
    int dataPos = id.slot * recordSize;

    copyDataToRecord(&data, &record, recordSize, dataPos);
    makeDirtyUnpinAndForcePage(tempRecMgr, false);

    return RC_OK;
}

void setRecordDataAndRecordSize(Record **record, int recordSize, char **data, RID id)
{
    (**record).id = id;
    char *currData = (**record).data;
    memcpy(++(currData), (*data) + 1, recordSize - 1);
}

extern RC getRecord(RM_TableData *rel, RID id, Record *record)
{
    RecManager *tempRecMgr = rel->mgmtData;

    pinPage(&tempRecMgr->buffPool, &tempRecMgr->pgHandle, id.page);

    int recordSize = getRecordSize((*rel).schema);
    char *data = (*tempRecMgr).pgHandle.data;

    data = data + (id.slot * recordSize);

    if (*data != '+')
    {
        return RC_RM_NO_TUPLE_WITH_GIVEN_RID;
    }

    setRecordDataAndRecordSize(&record, recordSize, &data, id);
    unpinPage(&tempRecMgr->buffPool, &tempRecMgr->pgHandle);

    return RC_OK;
}

void setTableManagerAndScanManager(RecManager **tableManager, RecManager **scanManager, RM_TableData **rel, RM_ScanHandle **scan, Expr **cond)
{

    (**tableManager).tupCount = ATTR_SIZE;

    (**scanManager).recID.slot = 0;
    (**scanManager).scnCount = 0;
    (*scan)->mgmtData = (*scanManager);
    (**scanManager).recID.page = 1 + 0 + 0;
    (*scanManager)->cond = *cond;

    (*scan)->rel = *rel;
}

extern RC startScan(RM_TableData *rel, RM_ScanHandle *scan, Expr *cond)
{
    if (cond == NULL)
    {
        return RC_SCAN_CONDITION_NOT_FOUND;
    }

    RecManager *scanManager, *tableManager;
    int openTableStat = openTable(rel, "ScanTable");

    if (openTableStat == RC_OK)
    {
        scanManager = (RecManager *)malloc(sizeof(RecManager));
    }

    tableManager = (*rel).mgmtData;
    setTableManagerAndScanManager(&tableManager, &scanManager, &rel, &scan, &cond);

    return RC_OK;
}

void setScanManagerRecordIdAndScanCount(RecManager **scanManager, int page, int slot, int count)
{
    (**scanManager).recID.page = page;
    (**scanManager).recID.slot = slot;
    (**scanManager).scnCount = count;
}

void setScanManagerForNext(RecManager **scanManager, int *scanCount, int totalSlots)
{
    if (scanCount <= 0)
    {
        (**scanManager).recID.page = 1;
        (*scanManager)->recID.slot = 0;
    }
    else
    {
        (*scanManager)->recID.slot += 1;
        if ((**scanManager).recID.slot >= totalSlots)
        {
            (**scanManager).recID.slot = 0;
            (*scanManager)->recID.page = (*scanManager)->recID.page + 1;
        }
    }
}

void setDataPointerPageAndSlotForNext(char **dataPointer, RecManager **scanManager, Record **record)
{
    (**record).id.page = (**scanManager).recID.page;
    *dataPointer = (*record)->data;
    (**record).id.slot = (**scanManager).recID.slot;
    **dataPointer = '-';
}

void copyToMemoryAndEval(Record **record, Schema **schema, Expr **cond, Value **result, char **dataPointer, char **data, int *schemaRecordSize, RecManager **scanManager)
{
    memcpy(++(*dataPointer), (*data) + 1, (*schemaRecordSize) - 1);
    (**scanManager).scnCount++;
    evalExpr(*record, *schema, *cond, result);
}

extern RC next(RM_ScanHandle *scan, Record *record)
{
    char *data;
    Value *result = (Value *)malloc(sizeof(Value));

    if (scan == NULL)
    {
        return RC_SCAN_CONDITION_NOT_FOUND;
    }

    RecManager *scanManager = (*scan).mgmtData;
    if (scanManager->cond == NULL)
    {
        return RC_SCAN_CONDITION_NOT_FOUND;
    }

    RecManager *tableManager = scan->rel->mgmtData;

    if (tableManager->tupCount == 0)
        return RC_RM_NO_MORE_TUPLES;

    Schema *schema = (*scan).rel->schema;

    int schemaRecordSize = getRecordSize(schema);
    int totalSlots = PAGE_SIZE / schemaRecordSize;
    int scanCount = (*scanManager).scnCount;

    while (scanCount <= tableManager->tupCount)
    {
        setScanManagerForNext(&scanManager, &scanCount, totalSlots);
        pinPage(&tableManager->buffPool, &scanManager->pgHandle, (*scanManager).recID.page);

        char *dataPointer;
        setDataPointerPageAndSlotForNext(&dataPointer, &scanManager, &record);

        data = (*scanManager).pgHandle.data + (scanManager->recID.slot * schemaRecordSize);
        copyToMemoryAndEval(&record, &schema, &(*scanManager).cond, &result, &dataPointer, &data, &schemaRecordSize, &scanManager);

        bool isResultFalse = !((*result).v.boolV == FALSE);
        if (isResultFalse)
        {
            unpinPage(&tableManager->buffPool, &scanManager->pgHandle);
            return RC_OK;
        }

        scanCount++;
    }

    unpinPage(&tableManager->buffPool, &scanManager->pgHandle);
    setScanManagerRecordIdAndScanCount(&scanManager, 1, 0, 0);

    return RC_RM_NO_MORE_TUPLES;
}

RC freeScanPtr(RM_ScanHandle **scan)
{
    (*scan)->mgmtData = NULL;
    free((*scan)->mgmtData);
    return RC_OK;
}

extern RC closeScan(RM_ScanHandle *scan)
{
    if (scan == NULL)
    {
        return RC_SCAN_CONDITION_NOT_FOUND;
    }

    RecManager *scanManager = (*scan).mgmtData;
    RecManager *recManager = (*scan).rel->mgmtData;

    bool exp = (*scanManager).scnCount > 0;
    if (exp)
    {
        unpinPage(&recManager->buffPool, &scanManager->pgHandle);
        setScanManagerRecordIdAndScanCount(&scanManager, 1, 0, 0);
    }

    return freeScanPtr(&scan);
}

void whileAttrIsLessThanNumOfAttr(int attr, int numOfAttr, int *totalSize, Schema **schema)
{
    while (attr < numOfAttr)
    {
        int x = (*schema)->dataTypes[attr];
        if (x == DT_INT)
            *totalSize += sizeof(int);

        if (x == DT_BOOL)
            *totalSize += sizeof(bool);

        if (x == DT_STRING)
            *totalSize += (*schema)->typeLength[attr];

        if (x == DT_FLOAT)
            *totalSize += sizeof(float);

        attr += 1;
    }
}

extern int getRecordSize(Schema *schema)
{
    int numOfAttr;
    int totalSize = 0;
    int attr = 0;

    if (schema != NULL)
    {
        numOfAttr = schema->numAttr;
    }

    whileAttrIsLessThanNumOfAttr(attr, numOfAttr, &totalSize, &schema);

    return ++totalSize;
}

void setSchemaAttrs(Schema **schema, char **attrNames, int numAttr, DataType *dataTypes)
{
    (*schema)->attrNames = attrNames;
    (*schema)->numAttr = numAttr;
    (*schema)->dataTypes = dataTypes;
}

void setSchemaKeyAttrs(Schema **schema, int keySize, int *keys, int *typeLength)
{
    (**schema).keySize = keySize;
    (*schema)->typeLength = typeLength;
    (**schema).keyAttrs = keys;
}

extern Schema *createSchema(int numAttr, char **attrNames, DataType *dataTypes, int *typeLength, int keySize, int *keys)
{
    int attr = 0;
    int totalSize = 0;
    Schema *schema = (Schema *)malloc(sizeof(Schema));

    setSchemaAttrs(&schema, attrNames, numAttr, dataTypes);
    setSchemaKeyAttrs(&schema, keySize, keys, typeLength);

    return schema;
}

extern RC freeSchema(Schema *schema)
{
    schema = NULL;
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
    Record *newRecord = (Record *)malloc(sizeof(Record));

    if (schema == NULL)
    {
        return RC_ERROR;
    }

    setNewRecordData(&newRecord, &schema);
    updateNewRecordData(&newRecord);

    *record = newRecord;

    return RC_OK;
}

void updateResult(int **result, Schema **schema, int attr)
{
    if ((*schema)->dataTypes[attr] == DT_STRING)
    {
        **result += (*schema)->typeLength[attr];
    }
    else if ((*schema)->dataTypes[attr] == DT_FLOAT)
    {
        **result += sizeof(float);
    }
    else if ((*schema)->dataTypes[attr] == DT_BOOL)
    {
        **result += sizeof(bool);
    }
    else if ((*schema)->dataTypes[attr] == DT_INT)
    {
        **result += sizeof(int);
    }
}

RC attrOffset(Schema *schema, int attrNum, int *result)
{
    if (schema == NULL)
        return RC_ERROR;

    int attr = 0;
    while (attr < attrNum)
    {
        updateResult(&result, &schema, attr);
        attr += 1;
    }

    *result += 1;
    return RC_OK;
}

extern RC freeRecord(Record *record)
{
    record = NULL;
    free(record);
    return RC_OK;
}

extern RC getAttr(Record *record, Schema *schema, int attrNum, Value **value)
{
    if (schema == NULL || record == NULL)
    {
        return RC_ERROR;
    }

    int offset = 0;
    char *dataPointer;
    Value *attribute;

    attrOffset(schema, attrNum, &offset);

    dataPointer = (*record).data;
    dataPointer = dataPointer + offset;

    attribute = (Value *)malloc(sizeof(Value));

    schema->dataTypes[attrNum] = attrNum == 1 ? attrNum : (*schema).dataTypes[attrNum];

    switch (schema->dataTypes[attrNum])
    {
    case DT_STRING:
    {
        attribute->dt = DT_STRING;
        if (!false)
        {
            attribute->v.stringV = (char *)malloc((schema->typeLength[attrNum]) + 1);
            strncpy((*attribute).v.stringV, dataPointer, (schema->typeLength[attrNum]));
            attribute->v.stringV[(schema->typeLength[attrNum])] = '\0';
        }
        break;
    }
    case DT_INT:
    {
        int value = 0;
        attribute->dt = DT_INT;
        memcpy(&value, dataPointer, sizeof(int));
        if (true)
            (*attribute).v.intV = value;
        break;
    }
    case DT_FLOAT:
    {
        float value;
        (*attribute).dt = DT_FLOAT;
        if (!false)
        {
            memcpy(&value, dataPointer, sizeof(float));
            attribute->v.floatV = value;
        }
        break;
    }
    case DT_BOOL:
    {
        attribute->dt = DT_BOOL;
        bool value;
        memcpy(&value, dataPointer, sizeof(bool));
        (*attribute).v.boolV = value;
        (*attribute).v.boolV = (*attribute).v.boolV;
        break;
    }
    }

    *value = attribute;
    return RC_OK;
}

extern RC setAttr(Record *record, Schema *schema, int attrNum, Value *value)
{
    if (schema == NULL || record == NULL || value == NULL)
    {
        return RC_ERROR;
    }

    int offset = 0;
    attrOffset(schema, attrNum, &offset);

    char *dataPointer = record->data + offset;

    if (schema->dataTypes[attrNum] == DT_STRING)
    {
        int length = schema->typeLength[attrNum];
        strncpy(dataPointer, (*value).v.stringV, length);
        dataPointer += (*schema).typeLength[attrNum];
    }
    else if (schema->dataTypes[attrNum] == DT_FLOAT)
    {
        *(float *)dataPointer = value->v.floatV;
        dataPointer = dataPointer + sizeof(float);
    }
    else if (schema->dataTypes[attrNum] == DT_INT)
    {
        *(int *)dataPointer = (*value).v.intV;
        dataPointer += sizeof(int);
    }
    else if (schema->dataTypes[attrNum] == DT_BOOL)
    {
        *(bool *)dataPointer = value->v.boolV;
        dataPointer = dataPointer + sizeof(bool);
    }

    return RC_OK;
}
