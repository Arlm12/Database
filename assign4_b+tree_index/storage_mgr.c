#include "dberror.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "storage_mgr.h"

// declare a common file pointer to be used in the functions
FILE *fp;
int checkStatus = 0;

/* Implement interfaces for manipulating page in files */

// Initialize the storage manager
extern void initStorageManager(void)
{
    fp = NULL;
    printf("Storage Manager Initialized");
}

// Interface to create a page in the file
extern RC createPageFile(char *fileName)
{
    // Open the file in write mode using built in fopen function
    fp = fopen(fileName, "w");
    // Check if file exists, if not return file not found error
    if (fp == NULL)
    {
        return RC_FILE_NOT_FOUND;
    }
    // Allocate memory for the page using built in malloc function
    char *mem = (char *)malloc(PAGE_SIZE);
    // Check if memory is allocated, if not return memory allocation error
    memset(mem, '\0', PAGE_SIZE);
    // Write the page to the file using built in fwrite function
    size_t totalWritten = fwrite(mem, sizeof(char), PAGE_SIZE, fp);
    // Defensive check to see if the written elements is within the given page limit
    if (totalWritten < PAGE_SIZE)
    {
        return RC_PAGE_MEM_OVERFLOW;
    }
    // Close the file using built in fclose function
    fclose(fp);
    // De-allocate the used memory
    free(mem);
    // Return success code
    return RC_OK;
}

// Interface to open a page in the file
extern RC openPageFile(char *fileName, SM_FileHandle *fHandle)
{
    // Open the file in read mode using built in fopen function
    fp = fopen(fileName, "r");
    // Check if file exists, if not return file not found error
    if (fp == NULL)
    {
        return RC_FILE_NOT_FOUND;
    }
    // Get the size of the file using built in fseek and ftell functions
    int status = fseek(fp, 0, SEEK_END);
    // Defensive check: returns zero if successful, else a non-zero value
    if (status == 0)
    {

        int size = ftell(fp);
        // Defensive check: returns the current value of the position indicator if successful, else -1
        if (size == -1)
        {
            fclose(fp);
            return RC_READ_NON_EXISTING_PAGE;
        }

        // Set the file handle attributes
        fHandle->curPagePos = 0;
        fHandle->fileName = fileName;
        fHandle->totalNumPages = size / PAGE_SIZE;
        // Close the file using built in fclose function
        fclose(fp);
        // Return success code
        return RC_OK;
    }
    fclose(fp);
    return RC_FILE_SEEK_FAILED;
}

// Interface to close a page to the file
extern RC closePageFile(SM_FileHandle *fHandle)
{
    // Create a file pointer and open the file in read mode
    fp = fopen(fHandle->fileName, "r");
    // Check if file exists, if not return file not found error
    if (fp == NULL)
    {
        return RC_FILE_NOT_FOUND;
    }
    // Close the file using built in fclose function
    fclose(fp);
    // Return success code
    return RC_OK;
}

// Interface to destroy a given file
extern RC destroyPageFile(char *fileName)
{
    // Remove the file using built in remove function
    int status = remove(fileName);
    // Check is file is removed - returns 0 if operation is successful, then return success code
    if (status == 0)
    {
        return RC_OK;
    }
    // Else return file not found error
    return RC_FILE_NOT_FOUND;
}

// Interface to read a single block from the file
extern RC readBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    // Open the file in read mode using built in fopen function
    fp = fopen(fHandle->fileName, "r");
    // Check if file exists, if not return file not found error
    if (fp == NULL)
    {
        return RC_FILE_NOT_FOUND;
    }
    // Check if page number is valid range(0 to totalPages in a file), if not return read non existing page error
    if (pageNum > fHandle->totalNumPages)
    {
        return RC_READ_NON_EXISTING_PAGE;
    }
    // Set the pointer to the particular block position in the file using built in fseek function
    size_t status = fseek(fp, pageNum * PAGE_SIZE, SEEK_SET);
    if (status != 0)
    {
        // return file seek failed error
        return RC_FILE_SEEK_FAILED;
    }
    // Read the total number of elements using built in fread function
    fread(memPage, sizeof(char), PAGE_SIZE, fp);
    // Update the current page position in the page handler
    fHandle->curPagePos = ftell(fp);
    // Close the file using built in fclose function
    fclose(fp);
    // Return success code
    return RC_OK;
}

// Interface to fetch the current block position in the page
extern int getBlockPos(SM_FileHandle *fHandle)
{
    // Defensive check: if file handle is not initialized, return file handle not initialized error
    if (fHandle == NULL)
    {
        return RC_FILE_HANDLE_NOT_INIT;
    }
    // Return the current page position
    return fHandle->curPagePos;
}

// Interface to read the first block in the file
extern RC readFirstBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    // Call the readBlock function to read the first block in the file because page numbers 0 indexed
    int blockPos = 0;
    // check_Status();
    return readBlock(blockPos, fHandle, memPage);
}

// Interface to read the previous block relative to current position in the file
extern RC readPreviousBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{

    // Previous block position can be calculated by subtracting 1 from the current block position
    // Call the readBlock function to read the previous block in the file
    return readBlock((fHandle->curPagePos) - 1, fHandle, memPage);
}

// Interface to read the current block in the file
extern RC readCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    // Current page position can be retrieved from the fHandle
    int currentBlock = fHandle->curPagePos;
    // check_Status();
    // Call the readBlock function to read the current block in the file
    return readBlock(currentBlock, fHandle, memPage);
}

// Interface to read the next block relative to current position in the file
extern RC readNextBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{

    // Next block position can be calculated by adding 1 to the current block position
    // Call the readBlock function to read the next block in the file
    // check_Status();
    return readBlock((fHandle->curPagePos) + 1, fHandle, memPage);
}

// Interface to read the final block in the file
extern RC readLastBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    // Total number of pages can be retrieved from the fHandle
    int totalPages = fHandle->totalNumPages;
    // Call the readBlock function to read the last block in the file
    return readBlock(totalPages - 1, fHandle, memPage);
}

// TODO: debug this interface later, for now this code works fine
extern RC writeBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)
{

    // fopen() function is used in write only mode. If file with existing filename is not present then a new file with that filename is created.
    FILE *SM_TempFile = fopen(fHandle->fileName, "r+");

    // check if file with given file name is created or present.
    if (SM_TempFile == NULL)
    {
        printf("Error opening the file\n");
        return (RC_FILE_NOT_FOUND);
    }

    // check for page number. Page number should range between 0 to total number of pages in page handler.
    if (pageNum < 0 || pageNum > fHandle->totalNumPages)
    {
        return RC_WRITE_FAILED;
    }

    // if it's the first page to which data has to be written
    if (pageNum == 0)
    {
        // move the file pointer from the beginning of the file to the page number where block needs to be written
        int fileseek = fseek(SM_TempFile, pageNum * PAGE_SIZE, SEEK_SET);

        // check if the seek operation was successful.
        if (fileseek == 0)
        {
            // writing on to the disk.
            fwrite(memPage, sizeof(char), PAGE_SIZE, SM_TempFile);
        }

        // if the seek operation was not successful, return an error
        else
        {
            fclose(SM_TempFile);
            return RC_READ_NON_EXISTING_PAGE;
        }

        // updating the current page position.
        fHandle->curPagePos = ftell(SM_TempFile);

        // Closing the file at the end
        fclose(SM_TempFile);
    }
    else
    {
        fHandle->curPagePos = pageNum * PAGE_SIZE;

        // appending empty block to occcupy the new contents
        appendEmptyBlock(fHandle);

        // move the file pointer from the beginning of the file to the page number where block needs to be written
        int fileseek = fseek(SM_TempFile, fHandle->curPagePos, SEEK_SET);

        // return an error if the seek operation was unsuccessful.
        if (fileseek != 0)
        {
            fclose(SM_TempFile);
            return RC_READ_NON_EXISTING_PAGE;
        }

        // fwrite() is used to write the contents of the memPage into the set cursor position
        fwrite(memPage, sizeof(char), strlen(memPage), SM_TempFile);

        // updating the current page position.

        fHandle->curPagePos = ftell(SM_TempFile);

        // Closing the file at the end
        fclose(SM_TempFile);
    }

    return RC_OK;
}

extern RC writeCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    // Current page position can be retrieved from the fHandle
    int currentPage = fHandle->curPagePos;
    // Call the writeBlock function with currentPage to write the current block in the file
    return writeBlock(currentPage, fHandle, memPage);
}

extern RC appendEmptyBlock(SM_FileHandle *fHandle)
{
    // Open the file in append mode using built in fopen function
    fp = fopen(fHandle->fileName, "a+");
    // Check if file with given name exists, if not return file not found error
    if (fp == NULL)
    {
        checkStatus = RC_FILE_NOT_FOUND;
        return RC_FILE_NOT_FOUND;
    }

    // Create a new block of size PAGE_SIZE using built in calloc function
    SM_PageHandle block = (SM_PageHandle)calloc(PAGE_SIZE, sizeof(char));
    // Fill the allocated memory with null character
    check_Status();
    memset(block, '\0', PAGE_SIZE);
    // Set the pointer to the end of the file using built in fseek function
    size_t status = fseek(fp, 0, SEEK_END);
    // Check if file seek operation is successful, if not return file seek failed error
    if (status != 0)
    {
        // frees the block memory and returns RC_WRITE_FAILED
        free(block);
        // Return write failed error
        return RC_WRITE_FAILED;
    }

    // Write to the new block using built in fwrite function
    int writtenCount = fwrite(block, sizeof(char), PAGE_SIZE, fp);
    // Check if write operation is successful by comparing with PAGE_SIZE, if not return write failed error
    if (writtenCount != 1)
    {
        // Return write failed error
        return RC_WRITE_FAILED;
    }
    // Increment the total number of pages in the file
    fHandle->totalNumPages++;

    // Close the file using built in fclose function
    fclose(fp);
    // Free the allocated memory for the block
    free(block);
    // Return success code
    return RC_OK;
}

extern RC ensureCapacity(int numberOfPages, SM_FileHandle *fHandle)
{
    // Open the file in append mode using built in fopen function
    fp = fopen(fHandle->fileName, "r");
    // Check if file with given name exists, if not return file not found error
    if (fp == NULL)
    {
        return RC_FILE_NOT_FOUND;
    }

    // Check if the number of pages is equal to the capacity of the file, then return success code
    if (fHandle->totalNumPages >= numberOfPages)
    {
        return RC_OK;
    }

    for (int count = fHandle->totalNumPages; count < numberOfPages;)
    {
        appendEmptyBlock(fHandle);
        count = fHandle->totalNumPages;
    }

    fclose(fp);
    return RC_OK;
}
