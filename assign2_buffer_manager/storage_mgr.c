#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "storage_mgr.h"
#include "dberror.h"
#include "dt.h"
//creating initStorageManager function now. The function do nothing now but we will use it later
void initStorageManager(void){
//keep it empty
}
//page file createed for storing the pages
RC createPageFile(char *fileName){
//to open the file. if file is not existing it is created. to open in read and write format we will use w+
FILE *file = fopen(fileName, "w+");
//this is for error handling
//this checks if when we open file it fails
if (file == NULL){
//it will return thr file not found error
return RC_FILE_NOT_FOUND;
}
//here the emptyPage is allocated memoryand the bytes are set to 0
SM_PageHandle emptyPage= (SM_PageHandle)calloc(PAGE_SIZE, sizeof(char));
//if emptyPage is Null meaning no enough memory, this will return error
if (emptyPage == NULL){
//and the file will be closed
fclose(file);
return RC_MEMORY_ALLOCATION_ERROR;
}
//this is used to write empty page to file
size_t writeSize= fwrite(emptyPage, sizeof(char), PAGE_SIZE, file);
//check if writeSize is less than PAGE_SIZE. If it is true it means there was a mistake in writing
if(writeSize<PAGE_SIZE){
//we will clean if error happened by first closing the file
fclose(file);
//then we will empty with the following command
free(emptyPage);
//lastly, error code is displayed to the user
return RC_WRITE_FAILED;
}
//if there is no error then we will empty the memory after writing
free(emptyPage);
//close the file
fclose(file);
//return success message
return RC_OK;
}
//now the file created is opened for further use
RC openPageFile(char *fileName, SM_FileHandle *fHandle){
//open the file. r+ is used to write and read for already exisiting files
FILE *file=fopen(fileName, "r+");
//if the file not found return the error message
if(file==NULL){
return RC_FILE_NOT_FOUND;
}
//this is used to go to the end of file and get the number of pages
fseek(file,0,SEEK_END);
//to get the size of the file in terms of bytes
long fileSize=ftell(file);
//to get the number of pages in the file
int totalNumPages=fileSize/PAGE_SIZE;
//set up fHandle for file handling with file name, total number of pages and the file pointer using curPagepos and mgmtInfo
fHandle->fileName=fileName;
fHandle->totalNumPages=totalNumPages;
//starting from the start
fHandle->curPagePos=0;
//to keep the pointer for later use
fHandle->mgmtInfo=file;
//success message
return RC_OK;
}
//now clean up. for that first close the file already opened
RC closePageFile(SM_FileHandle *fHandle){
FILE *file=(FILE *)fHandle->mgmtInfo;
//if pointer is null return the wrror message
if(file==NULL){
return RC_FILE_HANDLE_NOT_INIT;
}
fclose(file);
//empty the file handle pointer
fHandle->mgmtInfo=NULL;
//success message
return RC_OK;
}
//to delete the file from system
RC destroyPageFile(char *fileName){
//remove function is used to delete the file with the name given as the filename
//if file doesnot exist or we are not able to delete the file then it return non zero value
if(remove(fileName)!=0){
//if non zero value is printed means error occured so print the error message 
return RC_FILE_NOT_FOUND;
}
//if file is deleted then print success message
return RC_OK;
}
RC arunach12(){
    return 0;
}
//this is to read certain block
RC readBlock(int pageNum, SM_FileHandle *fHandle,SM_PageHandle memPage){
//to check if the page number mentioned is valis
//it should be greater than or equal to 1 and less than the total pages in the file
if(pageNum<0||pageNum>=fHandle->totalNumPages){
//if the page number is less than 0 or greater than the number of pages in the file return the error message
return RC_READ_NON_EXISTING_PAGE;
}
//this is used to get file pointer
FILE *file=(FILE *)fHandle->mgmtInfo;
//to check if file is opened by seeing the pointer
if(file==NULL){
//if it is null it means file handle is not initialized so print the error message saying so
return RC_FILE_HANDLE_NOT_INIT;
}
//calculating offset based on the entered page number
long offset=pageNum * PAGE_SIZE;
//moving file pointer to get the offset
fseek(file, offset, SEEK_SET);
//reading the data of page into memPage that is memory
size_t readSize=fread(memPage, sizeof(char),PAGE_SIZE,file);
//checking if fred is less than page size which means error occured
if(readSize<PAGE_SIZE){
//print the error message
return RC_READ_FAILED;
}
//go to the page number entered to be read
fHandle->curPagePos= pageNum;
return RC_OK;
}
void arunarun(){
    return 0;
}
//this is used to get the current block position of the file
int getBlockPos(SM_FileHandle *fHandle){
//this will get the current page position
return fHandle->curPagePos;
}
//to read first block of the file
RC readFirstBlock(SM_FileHandle *fHandle, SM_PageHandle memPage){
//since reading from firt block page num is 0
return readBlock(0,fHandle,memPage);
}
//to read the previous block compared to the block we are at
RC readPreviousBlock(SM_FileHandle *fHandle, SM_PageHandle memPage){
//calculate the previous page. we do it by subtracting 1 from the current page
int prevPage=fHandle->curPagePos -1;
return readBlock(prevPage, fHandle,memPage);
}
//to read the current block, the page we are at
RC readCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage){
//current page position to read the block
return readBlock(fHandle->curPagePos,fHandle,memPage);
}
//to read the next block, that is the next page compared to the page we are at
RC readNextBlock(SM_FileHandle *fHandle,SM_PageHandle memPage){
//to get the next page number, simply add 1 to the current position
int nextPage=fHandle->curPagePos+1;
//return the block to next page
return readBlock(nextPage, fHandle,memPage);
}
//read last block of the file
RC readLastBlock(SM_FileHandle *fHandle, SM_PageHandle memPage){
//to get the last page number simply subtract one from total number of pages
int lastPage=fHandle->totalNumPages -1;
return readBlock(lastPage, fHandle, memPage);
}
//to write data to the mentioned page number
RC writeBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){
//if page number is less than 0 that means it is invalid
if(pageNum<0){
//return the error message
return RC_WRITE_FAILED;
}
//to get the pointer from file handle
FILE *file=(FILE *)fHandle->mgmtInfo;
//if it is null it means file handle is not intialized so return the same error message
if(file==NULL){
return RC_FILE_HANDLE_NOT_INIT;
}
//this will check the status to see if enough pages are available
//if the pages are less we will use ensure capacity to add more pages
RC status= ensureCapacity(pageNum+1,fHandle);
if(status!=RC_OK){
return status;
}
//calculate the offset of the page number entered
long offset=pageNum*PAGE_SIZE;
//use fseek to calculate the offset
fseek(file,offset,SEEK_SET);
//write the content 
size_t writeSize=fwrite(memPage, sizeof(char), PAGE_SIZE,file);
if(writeSize<PAGE_SIZE){
return RC_WRITE_FAILED;
}
//updating the position to the the current position using the page number
fHandle->curPagePos=pageNum;
return RC_OK;
}
//to write the current block that is the current page we are at
RC writeCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage){
//we will call the writeBlock to the current position to perform writing the current block
return writeBlock(fHandle->curPagePos,fHandle,memPage);
}
//this is to add new empty block
//it is added to the end of the file
RC appendEmptyBlock(SM_FileHandle *fHandle){
FILE *file=(FILE *)fHandle->mgmtInfo;
//check if file is opened or not
if(file==NULL){
//if it returns null that means it is not intialized so return the message saying so
return RC_FILE_HANDLE_NOT_INIT;
}
//to add the new block to the end, go to the end of the file
fseek(file,0,SEEK_END);
//give memory for an empty page. keep it to 0 intially
SM_PageHandle emptyPage=(SM_PageHandle)calloc(PAGE_SIZE, sizeof(char));
//if any errors in memory allocation return this error message
if(emptyPage==NULL){
return RC_MEMORY_ALLOCATION_ERROR;
}
//this is to write the empty page to end of the file
size_t writeSize=fwrite(emptyPage, sizeof(char), PAGE_SIZE,file);
//if we write less than the page size 
if(writeSize<PAGE_SIZE){
//free memory allocated to the new empty block
free(emptyPage);
//and return this error message
return RC_WRITE_FAILED;
}
//after writing the page clean up the memory allocated
free(emptyPage);
//update the file handle such that it shows the updated total number of pages
fHandle->totalNumPages++;
//also add the current page position to new total number of pages
fHandle->curPagePos=fHandle->totalNumPages-1;
//success message
return RC_OK;
}
//check the file to see if the file have atleast the number of pages mentioned
RC ensureCapacity(int numberOfPages, SM_FileHandle *fHandle){
//check if the number of pages mentioned is less than the total number of pages
if(fHandle->totalNumPages>=numberOfPages){
//success message
return RC_OK;
}
//this will show the number of pages that should be added such that we satisfy the required capacity
int pagesToAdd=numberOfPages-fHandle->totalNumPages;
//this is to add empty pages till we reach the pagesToAdd number
for(int i=0;i<pagesToAdd;i++){
RC status=appendEmptyBlock(fHandle);
//if adding of empty block fails return status
if (status!=RC_OK){
return status;
}
}
//success message
return RC_OK;
}