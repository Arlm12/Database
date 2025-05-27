#include <stdio.h>
#include <stdlib.h>
#include "buffer_mgr.h"
#include "storage_mgr.h"
#include <math.h>
#include "string.h"

// Helper methods

extern RC runValidations(BM_BufferPool *const bm, BM_PageHandle *const page)
{
    // printf("runValidations \n");

    // ensure that the buffer pool is not null. If null, an error code would be returned.
    if (bm == NULL)
    {

        RC_message = "Buffer Pool not existing";
        return RC_FILE_NOT_FOUND;
    }

    // ensure that the page is not null. If null, an error code would be returned.
    if (page == NULL)
    {
        RC_message = "The provided page is Invalid";
        return RC_FILE_NOT_FOUND;
    }

    return RC_OK;
}

int bufferSize = 0;
int readCount = 0;
int frameIndex = 0;
int writeCount = 0;
int recentHitCount = 0;
int clockPtr = 0;

// Buffer Manager Interface Pool Handling

// Interface method to initialize a new buffer pool using pageFrames and page replacement strategy.
extern RC initBufferPool(BM_BufferPool *const bm, const char *const pgFileStr, const int numOfPgs, ReplacementStrategy replacement_strategy, void *strategyData)
{
    // printf("initBufferPool \n");
    // ensure that the page file name is not null. If null, an error code would be returned.
    if (pgFileStr == NULL)
    {
        RC_message = "File not existing";
        return RC_FILE_NOT_FOUND;
    }

    // ensure that the buffer pool is not null. If null, an error code would be returned.
    if (bm == NULL)
    {
        RC_message = "Buffer Pool not existing";
        return RC_BUFF_POOL_NOT_FOUND;
    }

    // ensure that the number of pages is not less than 1. If less than 1, an error code would be returned.
    (*bm).pageFile = (char *)malloc(strlen(pgFileStr) + 1);
    strcpy((*bm).pageFile, pgFileStr);

    (*bm).numPages = numOfPgs;
    (*bm).strategy = replacement_strategy;

    // allocating memory for the page frames
    PageFrame *pageFrames = (PageFrame *)malloc(sizeof(PageFrame) * numOfPgs);
    bufferSize = numOfPgs;

    int frameIndex = 0;
    // initializing the page frames with default values
    while (frameIndex < numOfPgs)
    {
        pageFrames[frameIndex].dirtyCount = 0;
        pageFrames[frameIndex].pageNumber = -1;
        pageFrames[frameIndex].accessCount = 0;
        pageFrames[frameIndex].data = NULL;
        pageFrames[frameIndex].secondChance = 0;
        pageFrames[frameIndex].recentHit = 0;
        pageFrames[frameIndex].index = 0;
        frameIndex++;
    }

    // initializing the buffer pool with the page frames and its management data structure
    (*bm).mgmtData = pageFrames;

    // initializing the write counts and clock pointer
    writeCount = 0;
    clockPtr = 0;

    // return success code
    return RC_OK;
}

// Interface used to delete or destroy the buffer pool and initialize all it's values to null.
extern RC shutdownBufferPool(BM_BufferPool *const bm)
{
    // printf("shutdownBufferPool \n");
    // ensure that the buffer pool is not null. If null, an error code would be returned.
    if (bm == NULL)
    {
        RC_message = "Buffer Pool not existing";
        return RC_BUFF_POOL_NOT_FOUND;
    }

    // retrieving the pageFrames details stored in the bm's mgmtData attribute
    PageFrame *pf = (PageFrame *)(*bm).mgmtData;

    // if any dirtyCount pages present, writing back to the disk before destroying the buffer pool and if the writing back to the disk fails, an error code would be returned.
    if (forceFlushPool(bm) != RC_OK)
        return RC_WRITE_BACK_FAILED;

    int frameIndex = 0;
    while (frameIndex < bufferSize)
    {
        // if any of the page frame in the buffer pool is still accessed by any of the user, then RC_PINNED_PAGES_IN_BUFFER is returned as the frame cannot be freed.
        if (pf[frameIndex].accessCount != 0)
            return RC_PINNED_PAGES_IN_BUFFER;
        ++frameIndex;
    }

    frameIndex = 0;
    while (frameIndex < bufferSize)
    {
        // all the allocated page frames memory is freed.
        free(pf[frameIndex].data);
        ++frameIndex;
    }

    free(pf);
    // the buffer pool mgmtData attribute is made to NULL.
    (*bm).mgmtData = NULL;
    return RC_OK;
}

// Interface used to write back all the dirty pages to the disk if any frame in the buffer pool has the dirty bit set and the not accessed by any user.
extern RC forceFlushPool(BM_BufferPool *const bm)
{
    // printf("forceFlushPool \n");
    // ensuring the buffer manager passed as argument is not null. If null, an error code would be returned.
    if (bm == NULL)
    {

        RC_message = "Buffer Pool not existing";
        return RC_BUFF_POOL_NOT_FOUND;
    }
    // retrieving the pageFrames details stored in the bm's mgmtData attribute
    PageFrame *pf = (PageFrame *)(*bm).mgmtData;

    // checking if the file associated with the buffer manager is exisiting
    SM_FileHandle fp;
    if ((openPageFile(bm->pageFile, &fp)) != RC_OK)
    {
        RC_message = "Error in Opening file!";
        return RC_FILE_NOT_FOUND;
    }

    int frameIndex = 0;
    // for each frames in the buffer pool, checking if any frame has the dirty bit set and the not accessed by any user, then that particualr frame contents would be written to the disk using writeBlock method and subsequently it's dirty bit would be set to 0 and the overall writeCount value is incremented.
    while (frameIndex < bufferSize)
    {

        if ((pf[frameIndex].dirtyCount != 0) && (pf[frameIndex].accessCount == 0))
        {
            // calling the writeBlock method to write the contents to the disk
            writeBlock(pf[frameIndex].pageNumber, &fp, pf[frameIndex].data);
            // unset the dirty bit of the frame
            pf[frameIndex].dirtyCount = 0;
            // incrementing the overall write count value
            ++writeCount;
        }
        ++frameIndex;
    }

    return RC_OK;
}

// Buffer Manager Interface Access Pages

// Interface to write all dirty pages from the buffer pool to the disk.
extern RC markDirty(BM_BufferPool *const bm, BM_PageHandle *const page)
{
    // printf("markDirty \n");
    int res = runValidations(bm, page);

    if (res != RC_OK)
        return res;

    // retrieve the pageFrames details stored in the bm's mgmtData attribute
    PageFrame *pf = (PageFrame *)(bm->mgmtData);

    int frameIndex = 0;
    // for each frames in the buffer pool, check if any frame contains the same page number as the given page number
    while (frameIndex < bufferSize)
    {
        if (pf[frameIndex].pageNumber == page->pageNum)
        {
            pf[frameIndex].dirtyCount = 1;
            return RC_OK;
        }
        ++frameIndex;
    }

    // if the page number is not found, then RC_PAGE_NOT_FOUND is returned.
    return RC_ERROR;
}

// Interface to unpins the page if found in the buffer which was currently pinned.
extern RC unpinPage(BM_BufferPool *const bm, BM_PageHandle *const page)
{
    // printf("unpinPage \n");
    // ensure that the buffer manager passed as argument is not null. If null, an error code would be returned.
    int res = runValidations(bm, page);

    if (res != RC_OK)
        return res;

    // retrieving the pageFrames details stored in the bm's mgmtData attribute
    PageFrame *pf = (PageFrame *)(bm->mgmtData);

    int frameIndex = 0;
    while (frameIndex < bufferSize)
    {
        // if any frame in the buffer manager contains the same page number, then that particular frame having the required page, it's accesscount would be decreased by one value
        pf[frameIndex].accessCount -= pf[frameIndex].pageNumber == page->pageNum ? 1 : 0;
        ++frameIndex;
    }

    // if the page number is not found, then RC_PAGE_NOT_FOUND is returned.
    return RC_OK;
}

// Interface to write the current content of the page to the page file on the disk.
extern RC forcePage(BM_BufferPool *const bm, BM_PageHandle *const page)
{
    // printf("forcePage \n");
    // ensuring the buffer manager passed as argument is not null. If null, an error code would be returned.
    int res = runValidations(bm, page);

    if (res != RC_OK)
        return res;

    SM_FileHandle fp;
    // retrieving the pageFrames details stored in the bm's mgmtData attribute
    PageFrame *pf = (PageFrame *)(*bm).mgmtData;

    // checking if the file associated with the buffer manager is existing
    if ((openPageFile(bm->pageFile, &fp)) != RC_OK)
    {
        RC_message = "Error in File opening";
        return RC_FILE_NOT_FOUND;
    }

    // for each frame in the pageframes, checking if the required page is present
    int frameIndex = 0;
    while (frameIndex < bufferSize)
    {
        // if any frame in the buffer manager contains the same page number, then that particular frame having the required page, it's details, would be written to disk using the writeBlock method and subsequently the dirty bit would be set to 0 and the overall writeCount value is incremented.
        if (pf[frameIndex].pageNumber == page->pageNum)
        {
            // calling the writeBlock method to write the contents to the disk
            writeBlock(pf[frameIndex].pageNumber, &fp, pf[frameIndex].data);
            // unset the frame's dirty bit
            pf[frameIndex].dirtyCount = 0;
            // incrementing the overall write count value
            ++writeCount;
            return RC_OK;
        }
        ++frameIndex;
    }

    // return RC_OK
    return RC_OK;
}

// Function is identical to a to a queue technique. First page in the buffer pool is in front, and this page will be first removed if the buffer pool is full.
extern void FIFO(BM_BufferPool *const bm, PageFrame *page)
{
    // printf("FIFO \n");
    // ensuring the buffer manager passed as argument is not null. If null, an error code would be returned.
    if (bm == NULL)
    {

        RC_message = "Buffer Pool not existing";
        // return RC_BUFF_POOL_NOT_FOUND;
    }

    // retieving the pageframes details stored in the bm's mgmtData attribute
    PageFrame *pageFrames = (PageFrame *)(bm->mgmtData);
    // determining the frame position where the existing frame has to be replaced with new frame

    // iterating over each frames and checking for the determined frame position
    for (int frame = 0; frame < bufferSize; ++frame)
    {
        // if the determined frame positon has zero access count but has dirty bit set, before replacing, the content has to be updated in the disk
        if (pageFrames[frameIndex].accessCount == 0)
        {
            // checking if dirty bit is set to 1
            if (pageFrames[frameIndex].dirtyCount != 0)
            {
                SM_FileHandle filehandle;
                // opening the file associated with the buffer pool
                if ((openPageFile(bm->pageFile, &filehandle)) != RC_OK)
                {
                    printf("Error in File opening \n");
                    RC_message = "Error in File opening";
                    // return RC_FILE_NOT_FOUND;
                }
                // writing on to the disk
                writeBlock(pageFrames[frameIndex].pageNumber, &filehandle, pageFrames[frameIndex].data);
                writeCount = writeCount + 1;
            }
            // setting the replacement frame with the required data
            pageFrames[frameIndex].data = page->data;
            int temp_pgNum = page->pageNumber;
            pageFrames[frameIndex].pageNumber = temp_pgNum;
            int temp_dirtyBit = page->dirtyCount;
            pageFrames[frameIndex].dirtyCount = temp_dirtyBit;
            int temp_accessCount = page->accessCount;
            pageFrames[frameIndex].accessCount = temp_accessCount;
            frameIndex = (frameIndex + 1) % bufferSize;
            break;
        }
        else
        {
            // incrementing the frame position value;
            frameIndex = (frameIndex + 1) % bufferSize;
        }
    }
}

// Function to removes the page frame that hasn't been used for a long time compared to other page frames in the buffer pool.
extern void LRU(BM_BufferPool *const bm, PageFrame *page)
{
    // printf("LRU \n");
    // ensuring the buffer manager passed as argument is not null. If null, an error code would be returned.
    if (bm == NULL)
    {

        RC_message = "Buffer Pool not existing";
        // return RC_BUFF_POOL_NOT_FOUND;
    }

    // retieving the pageframes details stored in the bm's mgmtData attribute
    PageFrame *pageFrames = (PageFrame *)(*bm).mgmtData;

    int replacementIndex, leaseRecentValue;

    // looping through each frame in the buffer pool to retrieve the frame having access count =0(not used by any user)
    for (int frame = 0; frame < bufferSize; frame++)
    {
        int curr_recenthitvalue;
        if (pageFrames[frame].accessCount == 0)
        {
            // assuming that particular frame as the replacment frame, and it's recentHit value as least recent hit value
            replacementIndex = frame;
            curr_recenthitvalue = pageFrames[frame].recentHit;
            leaseRecentValue = curr_recenthitvalue;
            break;
        }
    }

    // once the frame with accessCount = 0 has been determined, the frame with next least recentHit value is determined and that frame would be considered for replacing with the required page
    int replacementFrame = replacementIndex + 1;
    int temp_index;
    for (; replacementFrame < bufferSize; replacementFrame++)
    {
        if (pageFrames[replacementFrame].recentHit < leaseRecentValue)
        {
            replacementIndex = replacementFrame;
            temp_index = pageFrames[replacementIndex].recentHit;
            leaseRecentValue = temp_index;
        }
    }

    // checking if the replacment frame has it's dirty bit set to 1, to write back to disk
    if (pageFrames[replacementIndex].dirtyCount == 1)
    {
        SM_FileHandle fileHandle;
        if ((openPageFile(bm->pageFile, &fileHandle)) != RC_OK)
        {
            // printf("Error in File opening \n");
            RC_message = "Error in File opening";
            // return RC_FILE_NOT_FOUND;
        }

        // writing on to the disk
        writeBlock(pageFrames[replacementIndex].pageNumber, &fileHandle, pageFrames[replacementIndex].data);
        // incrementing the write count
        ++writeCount;
    }

    // the replacement frame has to be updated with the required page's attribute
    // setting the accessCount value for the replacement index
    int temp_accessCount = page->accessCount;
    pageFrames[replacementIndex].accessCount = temp_accessCount;
    // setting the data value for the replacement index
    pageFrames[replacementIndex].data = page->data;
    // setting the recenthit value for the replacement index
    int temp_recentHit = page->recentHit;
    pageFrames[replacementIndex].recentHit = temp_recentHit;
    // setting the pagenumber value for the replacement index
    int temp_pageNumber = page->pageNumber;
    pageFrames[replacementIndex].pageNumber = temp_pageNumber;
    // setting the dirtyCount value for the replacement index
    int temp_dirtybit = page->dirtyCount;
    pageFrames[replacementIndex].dirtyCount = temp_dirtybit;
}

// Function to keep a track of the last added page frame in the buffer pool. ClockPointer is used which is a counter to point the page frames in the buffer pool.
extern void CLOCK(BM_BufferPool *const bm, PageFrame *page)
{
    // printf("CLOCK \n");
    // ensuring the buffer manager passed as argument is not null. If null, an error code would be returned.
    if (bm == NULL)
    {

        RC_message = "Buffer Pool not existing";
        // return RC_BUFF_POOL_NOT_FOUND;
    }

    // retieving the pageframes details stored in the bm's mgmtData attribute
    PageFrame *pageFrames = (PageFrame *)(*bm).mgmtData;

    while (true)
    {
        // checking if the clock pointer frame hasn't recently been accessed ie the second chance is zero and that frame can be used for replacement with new frame
        if (pageFrames[clockPtr].secondChance == 0)
        {
            // checking if the replacment frame has it's dirty bit set to 1, to write back to disk
            if (pageFrames[clockPtr].dirtyCount == 1)
            {
                SM_FileHandle fileHandle;
                if ((openPageFile(bm->pageFile, &fileHandle)) != RC_OK)
                {
                    RC_message = "Error in File opening";
                    // return RC_FILE_NOT_FOUND;
                }

                // writing on to the disk
                writeBlock(pageFrames[clockPtr].pageNumber, &fileHandle, pageFrames[clockPtr].data);
                // incrementing the write count
                ++writeCount;
            }

            // the replacement frame has to be updated with the required page's attribute
            pageFrames[clockPtr].data = page->data;
            pageFrames[clockPtr].pageNumber = page->pageNumber;
            pageFrames[clockPtr].dirtyCount = page->dirtyCount;
            pageFrames[clockPtr].accessCount = page->accessCount;
            pageFrames[clockPtr].recentHit = page->recentHit;

            // setting the clock pointer to the next frame
            clockPtr = (clockPtr + 1) % bufferSize;
            break;
        }
        // scenario where the clock pointer frame has been recently accessed, that is it has a second chance, so it can be survived and hence can't be used for replacing technique
        else
        {
            // the second chance of the clock pointer frame would be made 0 and the clock pointer would be incremented.
            pageFrames[clockPtr].secondChance = 0;
            clockPtr = (clockPtr + 1) % bufferSize;
        }
    }
}

// Interface to pin a page in the buffer pool
extern RC pinPage(BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber n)
{
    // printf("pinPage \n");
    // ensuring the buffer manager passed as argument is not null. If null, an error code would be returned.
    int res = runValidations(bm, page);

    if (res != RC_OK)
        return res;

    // retieving the pageframes details stored in the bm's mgmtData attribute
    PageFrame *pf = (PageFrame *)(*bm).mgmtData;

    // if the buffer pool is empty -> the first frame is empty
    if (pf[0].pageNumber == -1)
    {
        SM_FileHandle fp;
        // opening page from the file associated with the buffer pool
        openPageFile(bm->pageFile, &fp);
        (pf + 0)->data = (SM_PageHandle)malloc(PAGE_SIZE);
        ensureCapacity(n, &fp);

        // read the block of data from the file
        readBlock(n, &fp, (pf)->data);

        // setting the first frame with the various attributes such as page number, accesscount, number of hits, data
        (pf)->pageNumber = n;
        ++((pf)->accessCount);
        readCount = recentHitCount = 0;

        (pf)->recentHit = recentHitCount;
        (pf)->index = 0;
        (pf)->secondChance = 0;

        page->pageNum = n;
        page->data = (pf)->data;
        return RC_OK;
    }
    else
    {

        bool isBufferFull = true;
        // for each frame in the buffer pool, if the page is already present in buffer pool, the associated access count and hitCount would be incremented, if the page is not present, then has to be read from disk
        for (int frame = 0; frame < bufferSize; frame++)
        {
            // checking if the frame are empty
            if (pf[frame].pageNumber == -1)
            {
                SM_FileHandle fp;
                // opening the file associated with the buffer pool
                openPageFile(bm->pageFile, &fp);
                pf[frame].index = 0;
                pf[frame].accessCount = 1;
                pf[frame].pageNumber = n;
                pf[frame].data = (SM_PageHandle)malloc(PAGE_SIZE);
                // reading the required data from disk
                readBlock(n, &fp, pf[frame].data);
                // setting the attributes of the frame in which new page has been loaded
                ++readCount;
                ++recentHitCount;

                if (bm->strategy == RS_CLOCK)
                    pf[frame].secondChance = 0;
                else if (bm->strategy == RS_LRU)
                    pf[frame].recentHit = recentHitCount;

                // setting the bufferFull boolean value to False
                isBufferFull = false;
                // setting the attributes of the page structure
                page->data = pf[frame].data;
                // setting the pagenum attribute for the page
                page->pageNum = n;
                break;
            }
            // scenario where frames are not empty
            else
            {
                // scenario where the frame isn't empty and the required page number is already present in the buffer pool
                if (pf[frame].pageNumber == n)
                {
                    // incrementing the access count of already present page in the buffer pool
                    pf[frame].accessCount++;
                    isBufferFull = !(true);
                    // incrementing the hit count of already present page in the buffer pool
                    ++recentHitCount;

                    if (bm->strategy == RS_CLOCK)
                        pf[frame].secondChance = 1;
                    else if (bm->strategy == RS_LRU)
                        pf[frame].recentHit = recentHitCount;

                    // setting the attributes of the page structure
                    page->data = pf[frame].data;
                    // setting the clock ptr attribute
                    clockPtr = clockPtr;
                    // setting the pagenum attribute for the page
                    page->pageNum = n;
                    break;
                }
            }
        }

        // scenario where there are no more frames left empty, and the corresponding replacment strategy has to be used
        if (isBufferFull == true)
        {
            SM_FileHandle fp;
            // creating a new frame that has to be provided for the replacement strategy
            PageFrame *newframe = (PageFrame *)malloc(sizeof(PageFrame));
            // opening the file associated with the buffer pool
            openPageFile(bm->pageFile, &fp);
            newframe->data = (SM_PageHandle)malloc(PAGE_SIZE);
            // reading the block from the data
            readBlock(n, &fp, newframe->data);
            // setting the values for the new frame
            newframe->accessCount = 1;
            newframe->index = 0;
            newframe->pageNumber = n;
            newframe->dirtyCount = 0;
            ++readCount;
            ++recentHitCount;

            switch (bm->strategy)
            {
            case RS_LRU:
                newframe->recentHit = recentHitCount;
                break;
            case RS_CLOCK:
                newframe->secondChance = 0;
                break;
            default:
                break;
            }
            // setting the attributes of the page structure
            int temp_pgnumber;
            temp_pgnumber = n;
            page->pageNum = temp_pgnumber;
            page->data = newframe->data;
            // depending on the replacement technique that is been used for the buffer pool, the new frame would be passed to that particular startegy function
            switch (bm->strategy)
            {
            case RS_CLOCK:
                CLOCK(bm, newframe);
                break;
            case RS_FIFO:
                FIFO(bm, newframe);
                break;
            case RS_LRU:
                LRU(bm, newframe);
                break;
            default:
                break;
            }

            free(newframe);
        }
        return RC_OK;
    }
}

// Statistics Interface

// Interface that returns the list of all the contents of the pages stored in the buffer pool.
extern PageNumber *getFrameContents(BM_BufferPool *const bm)
{
    // printf("getFrameContents\n");
    PageFrame *pageFrames;
    PageNumber *frameContents;
    // ensuring the buffer manager passed as argument is not null. If null, an error code would be returned.
    if (bm == NULL)
    {
        RC_message = "Buffer Pool not existing";
    }

    // creating new integer array containing the page number's of the pages present in the buffer pool
    frameContents = malloc(sizeof(PageNumber) * bufferSize);
    // retiriving all the frames in the buffer pool
    pageFrames = (PageFrame *)(bm->mgmtData);

    // iterating over each frame
    int frame = 0;
    while (frame < bufferSize)
    {
        // checking if any page has been loaded into that frame
        if (pageFrames[frame].pageNumber == -1)
            frameContents[frame] = NO_PAGE;
        else
            frameContents[frame] = pageFrames[frame].pageNumber;
        frame = frame + 1;
    }

    return frameContents;
}

// Interface that returns the list of all the dirty flags of the pages stored in the buffer pool.
extern bool *getDirtyFlags(BM_BufferPool *const bm)
{
    // printf("getDirtyFlags\n");
    // ensuring the buffer manager passed as argument is not null. If null, an error code would be returned.
    if (bm == NULL)
    {

        RC_message = "Buffer Pool not existing";
    }

    // having an array of bool values where true represents the corresponding frame in the buffer pool has dirty bit set
    bool *dirtyFlags_arr = malloc(sizeof(bool) * bufferSize);
    // retiriving all the frames in the buffer pool
    PageFrame *pageFrames = (PageFrame *)(bm->mgmtData);

    // iterating over each frame
    for (int frame = 0; frame < bufferSize; frame++)
    {
        // checking if the dirty bit of that frame has been set to 1
        if (pageFrames[frame].dirtyCount != 1)
            dirtyFlags_arr[frame] = false;

        else
            dirtyFlags_arr[frame] = true;
    }

    return dirtyFlags_arr;
}

// Interface that returns the list of all the fix counts of the pages stored in the buffer pool.
extern int *getFixCounts(BM_BufferPool *const bm)
{
    // printf("getFixCounts\n");
    // ensuring the buffer manager passed as argument is not null. If null, an error code would be returned.
    if (bm == NULL)
    {

        RC_message = "Buffer Pool not existing";
    }

    // having an array of intger values where each value represents the corresponding frame's access count in the buffer pool
    int *fixCounts_arr = (int *)malloc(sizeof(int) * bufferSize);
    // retiriving all the frames in the buffer pool
    PageFrame *pageFrames = (PageFrame *)(*bm).mgmtData;

    // iterating over each frame present in the buffer pool
    int frame = 0;
    while (frame < bufferSize)
    {
        // checking the value of the access count attribute of each frame and storing it in the fixCounts array
        if (pageFrames[frame].accessCount > 0)
            fixCounts_arr[frame] = pageFrames[frame].accessCount;
        else
            fixCounts_arr[frame] = 0;

        frame++;
    }

    return fixCounts_arr;
}

// Interface that returns the total number of IO read count performed by the buffer pool which means total number of pages read from the disk.
extern int getNumReadIO(BM_BufferPool *const bm)
{
    // printf("getNumReadIO\n");
    // ensuring the buffer manager passed as argument is not null. If null, an error code would be returned.
    if (bm == NULL)
    {

        RC_message = "Buffer Pool not existing";
    }

    // returning the number of pages that has been read from the disk
    return (readCount + 1);
}

// Interface that returns the total number of IO write count performed by the buffer pool which means total number of pages written to the disk.
extern int getNumWriteIO(BM_BufferPool *const bm)
{
    // printf("getNumWriteIO\n");
    // ensuring the buffer manager passed as argument is not null. If null, an error code would be returned.
    if (bm == NULL)
    {

        RC_message = "Buffer Pool not existing";
    }

    // returning the total writecount to the disk
    return writeCount;
}
