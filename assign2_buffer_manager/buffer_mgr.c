//including all the required libraries
//Including the input output library
#include <stdio.h> 
//including the standard library
#include <stdlib.h>
//including the string functions
#include <string.h>
//including the limits functions
#include <limits.h>
//including the header files 
//buffer manager header file
#include "buffer_mgr.h"
//storage manager header file
#include "storage_mgr.h"
//dberror header file 
#include "dberror.h"
//dt.h where the boolean functions is declared
#include "dt.h"
//we need to give the stucture for the buffer pool
typedef struct PageFrame{
//this is where we have the page number and the data
BM_PageHandle pageHandle;
//this is the flag for dirty pages
bool dirty;
//this has the fix count
int fixCount;
//this contains the reference history for LRU-K
int *refHistory;
//like the fix count this has the reference count for LRU-K
int refCount;
//indicates the position in fifo
int fifoPos;
//has the age of LRU
int age;
}PageFrame;
//we need to give the stucture for managing the buffer pool
typedef struct BufferPoolInfo {
//has the file handle for the page file
SM_FileHandle fileHandle;
//this is the structure defined before and it will be an array
PageFrame *frames;
//number of frames
int numFrames;
//number of read inputs and outputs
int readIO;
//number of write inputs and outputs
int writeIO;
//gives the position of fifo
int fifoCounter;
//gives the counter for lru age
int lruCounter;
//contains the value of k for lru-k
int k;
//global time counter
int time;
}BufferPoolInfo;
// helper functions 
int findPageInPool(BM_BufferPool *const bm, PageNumber pageNum);//function to find the page from the buffer pool
int findFreeFrame(BM_BufferPool *const bm);
int selectVictimFrame(BM_BufferPool *const bm);
void updateLRU(BM_BufferPool *const bm, int frameIndex);
void updateLRUK(BM_BufferPool *const bm, int frameIndex);
//intializing the bufferpool
RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName, const int numPages, ReplacementStrategy strategy, void *stratData){
printf("intializing the buffer");
//we start by allocating the memory for the information of the buffer pool
BufferPoolInfo *bpInfo=(BufferPoolInfo *)malloc(sizeof(BufferPoolInfo));
if(bpInfo==NULL){
printf("memory allocation error");
return RC_MEMORY_ALLOCATION_ERROR;}
//opening the page file
RC status = openPageFile((char *)pageFileName, &bpInfo->fileHandle);
//first we check for the status
if(status!=RC_OK){
//if the status is ok then we free the buffer pool information
free(bpInfo);
//after freeing the information we return the status
return status;
}
//here we are intializing the buffer pool with the values
//we assign number of frames with the number of pages
bpInfo->numFrames=numPages;
//the number of read inputs and outputs is equal to zero
bpInfo->readIO=0;
//the number of write inputs and outputs is equal to zero
bpInfo->writeIO=0;
//we have the fifo counter to zero
bpInfo->fifoCounter=0;
//and the lru counter is also set to zero
bpInfo->lruCounter=0;
//the global time is set to zero
bpInfo->time=0;
//in the below function we set the of k
if(strategy==RS_LRU_K && stratData != NULL){
bpInfo->k = *(int*)stratData;//if the k value is already present 
}
else{
bpInfo->k = 2;//if there is no k value set then we set the value to the default is 2
}
//we are allocating memory for the page frames
//this was declared as the structure above
bpInfo->frames=(PageFrame *)malloc(sizeof(PageFrame) * numPages);
//here we check if the frames are null
if(bpInfo->frames==NULL){
closePageFile(&bpInfo->fileHandle);
//we free the buffer pool information
free(bpInfo);
//we return the error statement thet there is an error in allocating the memory.
return RC_MEMORY_ALLOCATION_ERROR;
}
printf("initalize values for the buffer pool");
//intializing the value for all the pages
for(int i=0;i<numPages;i++){
//page number
bpInfo->frames[i].pageHandle.pageNum=NO_PAGE;
//data value
bpInfo->frames[i].pageHandle.data=NULL;
//dirty pages
bpInfo->frames[i].dirty=false;
//fix count
bpInfo->frames[i].fixCount=0;
//fifo position
bpInfo->frames[i].fifoPos=0;
//age of the lru
bpInfo->frames[i].age=0;
//reference count
bpInfo->frames[i].refCount=0;
//reference history
bpInfo->frames[i].refHistory=NULL;
}
//and we intialize the buffer manager
bm->pageFile=(char *)pageFileName;
//number of pages
bm->numPages=numPages;
//strategy
bm->strategy=strategy;
//buffer pool info is stored in management data
bm->mgmtData=bpInfo;
//if the functions is executed and there are no errors then we return RC_OK 
printf("the function has executed successfully");
return RC_OK;
}
void aruna123(){
    return 0;
}
//Function to shutdown buffer
RC shutdownBufferPool(BM_BufferPool *const bm){
// geting the information of the buffer 
BufferPoolInfo *bpInfo = (BufferPoolInfo *)bm->mgmtData;
//for loop runs for all the pages
for(int i=0;i<bpInfo->numFrames;i++)
{
//if the frames or the number of pages are greater than zero
//then we need to shut down the entire buffer pool
if(bpInfo->frames[i].fixCount > 0)
//return statement 
    return RC_SHUTDOWN_POOL_WITH_PINNED_PAGES;
}
//here we force the buffer and remove the data
RC status = forceFlushPool(bm);
//if we return any other error codes other than RC_OK
if(status != RC_OK)
    return status;
//print()
for(int i=0; i<bpInfo->numFrames; i++){
//in the given frame if the page is not empty
if(bpInfo->frames[i].pageHandle.data !=NULL){
//if the there is data in the page then remove it
free(bpInfo->frames[i].pageHandle.data);
//Then assign the page data as null
bpInfo->frames[i].pageHandle.data=NULL;
}
//if the reference history is not equal to null
if(bpInfo->frames[i].refHistory !=NULL){
//remove the reference history
free(bpInfo->frames[i].refHistory);
//Then assign the reference history of the frame as null
bpInfo->frames[i].refHistory=NULL;
}
}
//remove the frames
free(bpInfo->frames);
//close the page and move on
closePageFile(&bpInfo->fileHandle);
//remove the buffer information
free(bpInfo);
//assign the buffer manager to null value
bm->mgmtData=NULL;
//return RC_OK if the function has run
printf("the function has executed successfully");
return RC_OK;
}
//this is the force flush function
//This function was called in the previous funtion shut down buffer pool
RC forceFlushPool(BM_BufferPool *const bm){
BufferPoolInfo *bpInfo=(BufferPoolInfo *)bm->mgmtData;
//this writes the block information
for(int i=0;i<bpInfo->numFrames;i++){
if(bpInfo->frames[i].dirty && bpInfo->frames[i].fixCount==0){
RC status = writeBlock(bpInfo->frames[i].pageHandle.pageNum, &bpInfo->fileHandle, bpInfo->frames[i].pageHandle.data);
//if there is any status other than RC_OK
if(status != RC_OK)
//return the status 
    return status;
//writing
bpInfo->writeIO++;
//assign dirty as false
bpInfo->frames[i].dirty = false;
}
}
//return RC_OK after the function executes
return RC_OK;
}
//make dirty function
RC markDirty(BM_BufferPool *const bm, BM_PageHandle *const page){
BufferPoolInfo *bpInfo =(BufferPoolInfo *)bm->mgmtData;
//finding the page in the pool 
int frameIndex=findPageInPool(bm, page->pageNum);
//if the frame index is -1 then the page is not present 
if(frameIndex==-1)
//return page not found
    return RC_PAGE_NOT_FOUND;
bpInfo->frames[frameIndex].dirty=true;
//if the function is executed return RC_OK
printf("the function has executed successfully");
return RC_OK;
}
//unpin page function
RC unpinPage(BM_BufferPool *const bm, BM_PageHandle *const page){
BufferPoolInfo *bpInfo =(BufferPoolInfo *)bm->mgmtData;
//finding the page in the pool
int frameIndex=findPageInPool(bm, page->pageNum);
//seeing if the page is available or not
if(frameIndex==-1)
    return RC_PAGE_NOT_FOUND;
//reducing the number of pages 
if(bpInfo->frames[frameIndex].fixCount >0)
    bpInfo->frames[frameIndex].fixCount--;
//if the function is executed then return RC_OK
printf("the function has executed successfully");
return RC_OK;
}
//force page function
RC forcePage(BM_BufferPool *const bm, BM_PageHandle *const page){
BufferPoolInfo *bpInfo =(BufferPoolInfo *)bm->mgmtData;
//finding the page in the pool
int frameIndex=findPageInPool(bm, page->pageNum);
//page not found 
if(frameIndex==-1)
    return RC_PAGE_NOT_FOUND;
//call write block function and assign it to the status
RC status=writeBlock(bpInfo->frames[frameIndex].pageHandle.pageNum, &bpInfo->fileHandle, bpInfo->frames[frameIndex].pageHandle.data);
//if the status is not equal to RC_OK
if(status!=RC_OK)
//returning the status
    return status;
//write
bpInfo->writeIO++;
bpInfo->frames[frameIndex].dirty=false;
//return RC_OK if the program has run
printf("the function has executed successfully");
return RC_OK;
}
void new_arun(){
    //print();
    return 0;
}
//pin page function
RC pinPage(BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pageNum){
//if the page number is below zero
if(pageNum<0)
//page number is invalid
    return RC_INVALID_PAGE_NUM;
//print()
BufferPoolInfo *bpInfo=(BufferPoolInfo *)bm->mgmtData;
//increment the global timer
bpInfo->time++;
int frameIndex=findPageInPool(bm, pageNum);
//print()
if(frameIndex!=-1){
//increment the counter
bpInfo->frames[frameIndex].fixCount++;
//assigning page Num to Page
page->pageNum=pageNum;
// Page data is here and is assigned to variable data through bpInfo
page->data=bpInfo->frames[frameIndex].pageHandle.data;
//Checking for Strategy
if(bm->strategy==RS_LRU)
    updateLRU(bm,frameIndex);
else if(bm->strategy==RS_LRU_K)
    updateLRUK(bm,frameIndex);
    //returning RC_OK if the function is executed
    printf("the function has executed successfully");
return RC_OK;
}
// assigning free frame
frameIndex=findFreeFrame(bm);
if(frameIndex==-1){
frameIndex=selectVictimFrame(bm);
if(frameIndex==-1){
    // return if buffer pool is full
return RC_BUFFER_POOL_FULL;
}
// if frame index is dirty
if(bpInfo->frames[frameIndex].dirty){
RC status=writeBlock(bpInfo->frames[frameIndex].pageHandle.pageNum, &bpInfo->fileHandle, bpInfo->frames[frameIndex].pageHandle.data);
// if the function is not executed 
//return status != RC_OK
//return current Status
if (status != RC_OK)
    return status;
bpInfo->writeIO++;
//page dirty is false
bpInfo->frames[frameIndex].dirty = false;
}
free(bpInfo->frames[frameIndex].pageHandle.data);
bpInfo->frames[frameIndex].pageHandle.data = NULL;
// Reset reference history
if (bpInfo->frames[frameIndex].refHistory != NULL) {
free(bpInfo->frames[frameIndex].refHistory);
bpInfo->frames[frameIndex].refHistory = NULL;
}
bpInfo->frames[frameIndex].refCount = 0;
}
// This part is where we get the page from the disk
// We're telling which page we want and making space for it in our memory
// If we can't make space, we say sorry, no room in the memory
// Then we clean the space so it's all fresh and ready
bpInfo->frames[frameIndex].pageHandle.pageNum = pageNum;
bpInfo->frames[frameIndex].pageHandle.data = (SM_PageHandle)malloc(PAGE_SIZE);
if (bpInfo->frames[frameIndex].pageHandle.data == NULL)
    return RC_MEMORY_ALLOCATION_ERROR;
memset(bpInfo->frames[frameIndex].pageHandle.data, 0, PAGE_SIZE);
// this checks if we need to make new pages
// this is like when u need page 10 but only got 5 pages
if (pageNum >= bpInfo->fileHandle.totalNumPages) {
RC status = ensureCapacity(pageNum + 1, &bpInfo->fileHandle);
if (status != RC_OK)
    return status;
}
RC status = readBlock(pageNum, &bpInfo->fileHandle, bpInfo->frames[frameIndex].pageHandle.data);
if (status != RC_OK)
    return status;
bpInfo->readIO++;
// we say we use this page now
// its like saying we are reading a book
bpInfo->frames[frameIndex].fixCount = 1;
bpInfo->frames[frameIndex].dirty = false;
// now we do stuffs for diff strategies
// this help us decide which page to kick out later
if (bm->strategy == RS_FIFO) {
    bpInfo->frames[frameIndex].fifoPos = bpInfo->fifoCounter++;
} else if (bm->strategy == RS_LRU) {
    updateLRU(bm, frameIndex);
} else if (bm->strategy == RS_LRU_K) {
    updateLRUK(bm, frameIndex);
} else {
// maybe we do more stuff later 
// like if we invent new ways to choose pages
}
// give back the page we found
// its like handing over the book we just got
page->pageNum = pageNum;
page->data = bpInfo->frames[frameIndex].pageHandle.data;
printf("the function has executed successfully");
return RC_OK;
}
// this tells us what is in the buffer pool
// its like checking what books we got in our bag
PageNumber *getFrameContents(BM_BufferPool *const bm) {
BufferPoolInfo *bpInfo = (BufferPoolInfo *)bm->mgmtData;
PageNumber *frameContents = (PageNumber *)malloc(sizeof(PageNumber) * bm->numPages);
// we look at all pages and say if they real or not
// if not real we say NO_PAGE, like empty spot in bag
for (int i = 0; i < bm->numPages; i++) {
if (bpInfo->frames[i].pageHandle.pageNum != NO_PAGE)
    frameContents[i] = bpInfo->frames[i].pageHandle.pageNum;
else
    frameContents[i] = NO_PAGE;
}
return frameContents;
}
// this tell us which pages are dirty
// dirty is like when u scribble in a book
bool *getDirtyFlags(BM_BufferPool *const bm) {
BufferPoolInfo *bpInfo = (BufferPoolInfo *)bm->mgmtData;
bool *dirtyFlags = (bool *)malloc(sizeof(bool) * bm->numPages);
// we check each page if its dirty
// its like checking which books got scribbles
for (int i = 0; i < bm->numPages; i++) {
dirtyFlags[i] = bpInfo->frames[i].dirty;
}
return dirtyFlags;
}
RC arun12(){
    //empty function
    return 0;
}
// this say how many times we use each page
// like counting how many times we read each book
int *getFixCounts(BM_BufferPool *const bm) {
BufferPoolInfo *bpInfo = (BufferPoolInfo *)bm->mgmtData;
int *fixCounts = (int *)malloc(sizeof(int) * bm->numPages);
// we count for all pages
// its like keeping score of popular books
for (int i = 0; i < bm->numPages; i++) {
fixCounts[i] = bpInfo->frames[i].fixCount;
}
return fixCounts;
}
// this tell how many times we read stuff
// like counting trips to library
int getNumReadIO(BM_BufferPool *const bm) {
BufferPoolInfo *bpInfo = (BufferPoolInfo *)bm->mgmtData;
return bpInfo->readIO;
}
// this tell how many times we write stuff
// like countin how often we return books
int getNumWriteIO(BM_BufferPool *const bm) {
BufferPoolInfo *bpInfo = (BufferPoolInfo *)bm->mgmtData;
return bpInfo->writeIO;
}
// this help us find a page in the buffer
// its like searching for a book in your backpack
int findPageInPool(BM_BufferPool *const bm, PageNumber pageNum) {
BufferPoolInfo *bpInfo = (BufferPoolInfo *)bm->mgmtData;
// we look at all frames to find the page
// checking every pocket til we find the book
for (int i = 0; i < bpInfo->numFrames; i++) {
if (bpInfo->frames[i].pageHandle.pageNum == pageNum)
return i;
}
return -1;  // we no find it
}
// this finds empty spot in buffer
// like finding space for new book in full bag
int findFreeFrame(BM_BufferPool *const bm) {
BufferPoolInfo *bpInfo = (BufferPoolInfo *)bm->mgmtData;
// we look for frame with no page
// its like checking for empty spots in the bag
for (int i = 0; i < bpInfo->numFrames; i++) {
if (bpInfo->frames[i].pageHandle.pageNum == NO_PAGE)
return i;
}
return -1;  // no empty spot
}
RC aruna_chalam18(){
    return 0;
}
// this chooses which page to kick out when buffer full
// like picking which book to take out when bag too heavy
int selectVictimFrame(BM_BufferPool *const bm) {
BufferPoolInfo *bpInfo = (BufferPoolInfo *)bm->mgmtData;
int victim = -1;
if (bm->strategy == RS_FIFO) {
// for fifo we kick out oldest one
// like taking out book we put in first
int oldestPos = INT_MAX;
for (int i = 0; i < bpInfo->numFrames; i++) {
if (bpInfo->frames[i].fixCount == 0) {
if (bpInfo->frames[i].fifoPos < oldestPos) {
oldestPos = bpInfo->frames[i].fifoPos;//getting the oldest position
victim = i;
}
}
}
} else if (bm->strategy == RS_LRU) {
// for lru we kick out least used one
// like removing book we never read in ages
int oldestAge = INT_MAX;
for (int i = 0; i < bpInfo->numFrames; i++) {
if (bpInfo->frames[i].fixCount == 0) {
if (bpInfo->frames[i].age < oldestAge) {
oldestAge = bpInfo->frames[i].age; //getting the oldest age of the LRU
victim = i;
}
}
}
} else if (bm->strategy == RS_LRU_K) {
// lru-k is complicated but we try
// its like choosing based on how often we read lately
int oldestRef = -1;
int oldestTime = INT_MAX;
for (int i = 0; i < bpInfo->numFrames; i++) {
if (bpInfo->frames[i].fixCount == 0) {
if (bpInfo->frames[i].refCount >= bpInfo->k) {
int kRefTime = bpInfo->frames[i].refHistory[bpInfo->k - 1];
if (kRefTime < oldestTime) {
oldestTime = kRefTime;
victim = i;
}
} else {
// we like pages with less uses
// kinthe like preferring books we aint read much
if (oldestRef == -1 || bpInfo->frames[i].refCount < oldestRef) {
oldestRef = bpInfo->frames[i].refCount;
victim = i;
}
}
}
}
} else {
// we don't know what to do for other types
// more ways later for other types
// we should return error at this stage
return -1;
}
return victim;
}
// this updates lru 
// like updating when we last read each book
void updateLRU(BM_BufferPool *const bm, int frameIndex) {
BufferPoolInfo *bpInfo = (BufferPoolInfo *)bm->mgmtData;
bpInfo->lruCounter++;
bpInfo->frames[frameIndex].age = bpInfo->lruCounter;
}
RC arunachalam(){
    //empty function
    return 0;
}
// this upthete lru-k 
// its tricky, like keeping track of last few times we read each book
void updateLRUK(BM_BufferPool *const bm, int frameIndex) {
BufferPoolInfo *bpInfo = (BufferPoolInfo *)bm->mgmtData;
int k = bpInfo->k;
// we make new list if we don't have one
// like starting a new reading log for a book
if (bpInfo->frames[frameIndex].refHistory == NULL) {
bpInfo->frames[frameIndex].refHistory = (int *)malloc(sizeof(int) * k);
memset(bpInfo->frames[frameIndex].refHistory, -1, sizeof(int) * k);
}
// we move everything down to make room for new thing
// like moving old dates to add new one in reading log
for (int i = k - 1; i > 0; i--) {
bpInfo->frames[frameIndex].refHistory[i] = bpInfo->frames[frameIndex].refHistory[i - 1];
}
// put new thing at front
// adding today's date to top of reading log
bpInfo->frames[frameIndex].refHistory[0] = bpInfo->time;
// count how many times we use it
// but we only care bout last k times
if (bpInfo->frames[frameIndex].refCount < k)
bpInfo->frames[frameIndex].refCount++;
}
