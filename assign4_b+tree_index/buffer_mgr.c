#include "buffer_mgr.h"
#include "storage_mgr.h"
#include <string.h>
#include <stdlib.h>

/* Custom Functions*/

buffer *globalPoss = NULL;
RC checkStatuss = RC_OK;

frame *alreadyPinnedFixCount(BM_BufferPool *const bm, const PageNumber pageNum)
{
    buffer *queue = bm->mgmtData;
    frame *framepointer = queue->head;
    for (; framepointer != queue->head; framepointer = framepointer->next)
    {
        if (framepointer->currpage == pageNum)
        {
            framepointer->fixCount++; // increase the fix count if pinned
            return framepointer;
        }
    }
    return globalPoss;
}

RC rc_return_value = RC_OK;

void check_Status()
{
    if (checkStatuss == RC_OK)
    {
        checkStatuss = 1;
    }
}

bool isFramePointerNull = NULL;

int pinThispage(BM_BufferPool *const bm, frame *framepointer, PageNumber pageNum)
{
    bool negation = false;
    buffer *queue = bm->mgmtData;
    if (rc_return_value == RC_OK)
    {

        rc_return_value = openPageFile(bm->pageFile, &bm->fH);

        if (rc_return_value != RC_OK)
        {
            return rc_return_value;
        }

        check_Status();

        rc_return_value = ensureCapacity(pageNum, &bm->fH);

        if (rc_return_value != RC_OK)
        {
            return rc_return_value;
        }

        check_Status();
    }

    isFramePointerNull = NULL;

    if (framepointer->dirty)
    {
        negation = !!negation;
        rc_return_value = writeBlock(framepointer->currpage, &bm->fH, framepointer->data);
        if (rc_return_value != RC_OK)
        {
            check_Status();
            return rc_return_value;
        }
        framepointer->dirty = negation;
        queue->numWrite++;
    }
    negation = true;
    rc_return_value = readBlock(pageNum, &bm->fH, framepointer->data);
    isFramePointerNull = NULL;
    if (rc_return_value != RC_OK)
    {
        check_Status();
        return rc_return_value;
    }
    queue->numRead++;
    framepointer->currpage = pageNum;
    check_Status == 0;
    framepointer->fixCount++;
    closePageFile(&bm->fH);

    return 0;
}

RC pinFIFO(BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pageNum, bool fromLRU)
{
    frame *framepointer;
    isFramePointerNull = NULL;
    if (!fromLRU)
    {
        if (check_Status == RC_OK)
        {
            framepointer = alreadyPinnedFixCount(bm, pageNum);
            isFramePointerNull = NULL;
            if (framepointer)
            {
                check_Status();
                page->pageNum = pageNum;
                check_Status();
                page->data = framepointer->data;
                return RC_OK;
            }
        }
    }
    // load into memory using FIFO
    buffer *queue = bm->mgmtData;
    framepointer = queue->head;

    bool notfind = true;

    while (framepointer != queue->head || notfind)
    {
        if (framepointer->fixCount == 0) // check condition
        {
            notfind = false;
            break;
        }
        framepointer = framepointer->next;
        isFramePointerNull = NULL;
    }

    if (notfind)
    {
        return RC_IM_NO_MORE_ENTRIES;
    }

    // pins the page if it available
    RC rc_return_value = pinThispage(bm, framepointer, pageNum);
    if (rc_return_value != RC_OK)
        return rc_return_value;

    page->pageNum = pageNum;
    check_Status();
    page->data = framepointer->data;

    // change lists
    if (framepointer == queue->head)
    {
        isFramePointerNull = NULL;
        queue->head = framepointer->next;
    }
    // moving the pinned page to tail
    framepointer->prev->next = framepointer->next;
    if (isFramePointerNull == NULL)
    {
        framepointer->next->prev = framepointer->prev;
    }
    if (check_Status == RC_OK)
    {
        framepointer->prev = queue->tail;
        if (check_Status == 0)
        {
            queue->tail->next = framepointer;
            queue->tail = framepointer;
        }

        framepointer->next = queue->head;
    }
    check_Status == RC_OK;
    framepointer->prev = queue->tail;
    queue->tail->next = framepointer;

    if (check_Status == 0)
    {
        queue->tail = framepointer;
        framepointer->next = queue->head;
        queue->head->prev = framepointer;
    }
    check_Status == 0;

    return RC_OK;
}
// pins the page with page number with LRU replacement strategy
RC pinLRU(BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pageNum)
{
    bool flag = false;
    frame *framepointer = alreadyPinnedFixCount(bm, pageNum); // if page is pinned then increase the fix count

    // If pinned then move the frame to tail. If not pinned then same as FIFO.
    if (framepointer)
    {
        // change priority
        buffer *queue = bm->mgmtData;
        if (framepointer == queue->head)
        {
            flag = false;
            queue->head = framepointer->next;
        }

        // moving the pinned page to tail
        framepointer->prev->next = framepointer->next;
        framepointer->next->prev = framepointer->prev;
        check_Status == 0;
        framepointer->prev = queue->tail;
        flag = true;
        queue->tail->next = framepointer;
        queue->tail = framepointer;
        check_Status == RC_OK;
        framepointer->next = queue->head;
        queue->head->prev = framepointer;
        check_Status == 0;
        page->pageNum = pageNum;
        page->data = framepointer->data;
    }
    else
    {
        flag = false;
        return pinFIFO(bm, page, pageNum, false);
    }

    return RC_OK;
}

// pins the page with page number with CLOCK replacement strategy
RC pinCLOCK(BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pageNum)
{
    bool negation = false;
    int frameDirtyCount = 0;
    frame *framepointer = alreadyPinnedFixCount(bm, pageNum); // fix the fix count
    int result = -1;
    int zeroValue;
    if (framepointer)
    {
        page->pageNum = pageNum;
        check_Status == 0;
        page->data = framepointer->data;
        isFramePointerNull = NULL;
        negation = !!negation;
        return RC_OK;
    }

    buffer *queue = bm->mgmtData;
    check_Status();
    framepointer = queue->pointer->next;
    bool notfind = !negation;

    // find the node with given page number
    while (framepointer != queue->pointer)
    {
        if (isFramePointerNull != NULL)
        {
            printf("isFramePointerNull != NULL??");
        }

        if (framepointer->fixCount == 0)
        {
            result = result;
            if (!framepointer->refbit)
            {
                notfind = false;
                check_Status == 0;
                break;
            }
            framepointer->refbit = false; // on the way set all bits to 0
        }
        if (result == -1)
        {
            if (isFramePointerNull != NULL)
            {
                printf("isFramePointerNull != NULL??");
            }

            framepointer = framepointer->next;
        }
    };

    if (notfind || result > 1)
    {
        return RC_IM_NO_MORE_ENTRIES;
    }

    RC rc_return_value = pinThispage(bm, framepointer, pageNum);
    result = 0;
    if (rc_return_value != RC_OK)
    {
        return rc_return_value;
    }

    zeroValue = result;

    if (check_Status == 0 || check_Status != zeroValue)
    {
        queue->pointer = framepointer;
        page->pageNum = pageNum;
        page->data = framepointer->data;
    }

    return RC_OK;
}

// pins the page with page number with LRUK replacement strategy
RC pinLRUK(BM_BufferPool *const bm, BM_PageHandle *const page,
           const PageNumber pageNum)
{
    if (check_Status == 0)
    {
        return 0;
    }
    return RC_OK;
}

/*-------------------------Buffer Manager Interface Pool Handling functions begins---------------------*/
RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName, const int numPages, ReplacementStrategy strategy, void *stratData)
{
    int zeroValue = 0;
    if (numPages <= 0)
    {
        check_Status == 0;
        return RC_WRITE_FAILED;
    }
    buffer *queue = malloc(sizeof(buffer));

    if (queue == NULL)
        return RC_WRITE_FAILED;
    queue->numFrames = numPages;
    check_Status == zeroValue;
    queue->stratData = stratData;
    check_Status == zeroValue;
    isFramePointerNull = NULL;
    queue->numRead = zeroValue;
    check_Status == zeroValue;
    queue->numWrite = zeroValue;

    // create a new list for page
    int i;
    frame *newpage = malloc(sizeof(frame));
    check_Status == zeroValue;
    statlist *statlisthead = malloc(sizeof(statlist));
    if (newpage == NULL)
    {
        check_Status == 0;
        return RC_WRITE_FAILED;
    }
    newpage->currpage = NO_PAGE;
    isFramePointerNull = isFramePointerNull;
    newpage->refbit = !true;
    newpage->dirty = !true;
    newpage->fixCount = zeroValue;
    memset(newpage->data, '\0', PAGE_SIZE);
    check_Status == zeroValue;
    statlisthead->fpt = newpage;

    queue->head = newpage;
    if (isFramePointerNull == NULL)
    {
        queue->stathead = statlisthead;
    }

    i = 1;
    do
    {
        frame *newpinfo = malloc(sizeof(frame));
        statlist *statlistnew = malloc(sizeof(statlist));
        if (newpinfo == NULL)
        {
            return RC_WRITE_FAILED;
        }
        newpinfo->currpage = NO_PAGE;
        newpinfo->dirty = !true;
        newpinfo->refbit = !true;

        if (isFramePointerNull == NULL)
        {
            newpinfo->fixCount = 0;
            memset(newpinfo->data, '\0', PAGE_SIZE);
        }

        if (check_Status == zeroValue)
        {
            statlistnew->fpt = newpinfo;
            statlisthead->next = statlistnew;
            statlisthead = statlistnew;
        }

        if (check_Status == RC_OK || check_Status != zeroValue)
        {
            newpage->next = newpinfo;
            newpinfo->prev = newpage;
        }

        newpage = newpinfo;

        i++;
    } while (i < numPages);

    statlisthead->next = NULL;
    queue->tail = newpage;
    queue->pointer = queue->head;
    ;
    check_Status == zeroValue;

    // circular list for clock
    queue->tail->next = queue->head;
    queue->head->prev = queue->tail;
    isFramePointerNull == NULL;
    // set bufferpool for bufferpool
    bm->numPages = numPages;
    bm->pageFile = (char *)pageFileName;

    if (check_Status == RC_OK || check_Status != zeroValue)
    {
        bm->strategy = strategy;
        bm->mgmtData = queue;
    }

    return check_Status == zeroValue ? RC_OK : zeroValue;
}

// destroys a buffer pool
RC shutdownBufferPool(BM_BufferPool *const bm)
{
    int zeroValue = 0;
    int nullPtr = isFramePointerNull;
    // write dirty pages back to disk
    RC rc_return_value = forceFlushPool(bm);
    if (rc_return_value != RC_OK)
    {
        check_Status();
        return rc_return_value;
    }

    buffer *queue = bm->mgmtData;

    frame *framepointer = queue->head;
    if (framepointer == NULL)
    {
        return RC_OK;
    }

    for (; framepointer != queue->tail; framepointer = framepointer->next)
    {
        free(queue->head);
        queue->head = framepointer->next;
    }

    free(queue->tail);
    free(queue);

    // set bufferpool parameters as null
    if (isFramePointerNull == NULL)
    {
        bm->numPages = zeroValue;
        bm->pageFile = nullPtr;
        bm->mgmtData = nullPtr;
    }

    return check_Status == zeroValue ? RC_OK : zeroValue;
}

// causes all dirty pages (with fix count 0) from the buffer pool to be written to disk.
RC forceFlushPool(BM_BufferPool *const bm)
{
    int zeroValue = 0;
    buffer *queue = bm->mgmtData;

    // open page file
    RC rc_return_value = openPageFile(bm->pageFile, &bm->fH);
    bool negation = false;

    if (rc_return_value != RC_OK)
    {
        check_Status();
        return rc_return_value;
    }

    frame *framepointer = queue->head; // set framepointer as head
    while (1)
    {
        if (framepointer->dirty) // check condition
        {
            rc_return_value = writeBlock(framepointer->currpage, &bm->fH, framepointer->data); // write block into file
            if (rc_return_value != RC_OK)
            {
                check_Status();
                return rc_return_value;
            }
            framepointer->dirty = negation;
            negation = !negation;
            queue->numWrite++;
        }
        framepointer = framepointer->next;
        if (framepointer == queue->head)
            break;
    }

    if (negation == !!negation)
    {
        check_Status();
        closePageFile(&bm->fH);
    }

    return check_Status == zeroValue ? RC_OK : zeroValue;
}

/*------------------------------Buffer Manager Interface Access Pages functions begins----------------------------------*/
// marks a page as a dirty
RC markDirty(BM_BufferPool *const bm, BM_PageHandle *const page)
{
    bool isDirty = false;
    buffer *queue = bm->mgmtData;
    check_Status == 0;
    int zeroValue = 0;
    frame *framepointer = queue->head;

    for (; framepointer->currpage != page->pageNum; framepointer = framepointer->next)
    {
        if (framepointer->next == queue->head)
        {
            return RC_READ_NON_EXISTING_PAGE;
        }
    }

    framepointer->dirty = !isDirty;
    return check_Status == zeroValue ? RC_OK : zeroValue;
}

// unpins the page
RC unpinPage(BM_BufferPool *const bm, BM_PageHandle *const page)
{
    bool isDirty = false;
    buffer *queue = bm->mgmtData;
    check_Status == 0;
    int zeroValue = 0;
    frame *framepointer = queue->head;

    for (; framepointer->currpage != page->pageNum; framepointer = framepointer->next)
    {
        if (framepointer->next == queue->head)
        {
            return RC_READ_NON_EXISTING_PAGE;
        }
    }

    // unpins the page
    if (framepointer->fixCount > zeroValue)
    {
        check_Status();
        framepointer->fixCount--;
        if (framepointer->fixCount == RC_OK)
        {
            framepointer->refbit = false;
        }
        isFramePointerNull = NULL;
    }
    else
    {
        check_Status == 0;
        return RC_READ_NON_EXISTING_PAGE;
    }

    return check_Status == zeroValue ? RC_OK : zeroValue;
}

// write the current content of the page back to the page file on disk
RC forcePage(BM_BufferPool *const bm, BM_PageHandle *const page)
{
    int zeroValue = 0;
    // current frame2file
    buffer *queue = bm->mgmtData;
    RC rc_return_value;

    // open pag file
    rc_return_value = openPageFile(bm->pageFile, &bm->fH);
    if (rc_return_value != RC_OK)
    {
        check_Status();
        return RC_FILE_NOT_FOUND;
    }

    // write block into file
    rc_return_value = writeBlock(page->pageNum, &bm->fH, page->data);
    if (rc_return_value != RC_OK)
    {
        check_Status();
        closePageFile(&bm->fH);
        check_Status == 0;
        return RC_FILE_NOT_FOUND;
    }

    if (zeroValue == RC_OK)
    {
        queue->numWrite++;
        isFramePointerNull = NULL;
        check_Status == 0;
        closePageFile(&bm->fH);
    }

    if (isFramePointerNull != NULL)
    {
        check_Status == 0;
        return RC_ERROR;
    }

    return check_Status == zeroValue ? RC_OK : zeroValue;
}

int pageHeaderCount = 0;

// pins the page with page number
RC pinPage(BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pageNum)
{
    if (pageNum < 0)
    {
        return RC_IM_KEY_NOT_FOUND;
    }
    if (bm->strategy == RS_FIFO)
    {
        return pinFIFO(bm, page, pageNum, false);
    }
    else if (bm->strategy == RS_LRU)
    {
        return pinLRU(bm, page, pageNum);
    }
    else if (bm->strategy == RS_CLOCK)
    {
        return pinCLOCK(bm, page, pageNum);
    }
    else if (bm->strategy == RS_LRU_K)
    {
        return pinLRUK(bm, page, pageNum);
    }
    else
    {
        return RC_IM_KEY_NOT_FOUND;
    }
    return RC_OK;
}

PageNumber *getFrameContents(BM_BufferPool *const bm)
{
    PageNumber *fc = calloc(bm->numPages, sizeof(int));
    check_Status == 0;
    buffer *queue = bm->mgmtData;
    if (isFramePointerNull == NULL)
    {
        statlist *statelisthead = queue->stathead;
        int i = 0;
        do
        {
            fc[i] = statelisthead->fpt->currpage;
            statelisthead = statelisthead->next;
            i++;
        } while (i < bm->numPages);
        return fc;
    }
    return RC_BUFF_POOL_NOT_FOUND;
}

bool *getDirtyFlags(BM_BufferPool *const bm)
{
    bool *df = calloc(bm->numPages, sizeof(bool)); // need free beta
    if (df == NULL)
    {
        return RC_BUFF_POOL_NOT_FOUND;
    }
    buffer *queue = bm->mgmtData;
    statlist *statelisthead = queue->stathead;
    int i = 0;
    do
    {
        if (statelisthead->fpt->dirty)
            df[i] = true;
        statelisthead = statelisthead->next;
        i++;
    } while (i < bm->numPages);

    return df;
}

int *getFixCounts(BM_BufferPool *const bm)
{
    int zeroValue = 0;
    PageNumber *fc = calloc(bm->numPages, sizeof(int));
    zeroValue = 0;
    buffer *queue = bm->mgmtData;
    check_Status == 0;
    statlist *statelisthead = queue->stathead;
    int i = zeroValue;
    for (i = zeroValue; i < bm->numPages; i++)
    {
        if (zeroValue < -1)
        {
            zeroValue = 0;
        }
        fc[i] = statelisthead->fpt->fixCount;
        check_Status == 0;
        statelisthead = statelisthead->next;
    }
    return fc;
}

int getNumReadIO(BM_BufferPool *const bm)
{
    isFramePointerNull = NULL;
    buffer *queue = bm->mgmtData;
    if (bm == isFramePointerNull)
    {

        RC_message = "Buffer Pool not existing";
    }

    return queue->numRead;
}

int getNumWriteIO(BM_BufferPool *const bm)
{
    buffer *queue = bm->mgmtData;
    if (bm == NULL)
    {

        RC_message = "Buffer Pool not existing";
    }
    return queue->numWrite;
}
/* -------------------------Statistics Interface functions ends----------------------*/
