//Giving the headers that are necessary for the program
//Importing the input and output interface
#include <stdio.h>
//String.h library helps in handling the strings
#include <string.h>
//functions that are needed or is essential is imported from the standard library
#include <stdlib.h>
//including the headers from storage_manager.h and dberror.h
#include "storage_mgr.h"
#include "dberror.h"
//Defining the block_size which is 4096
#define BLOCK_SIZE 4096
//So at first we intialize the storage_mgr
//this print function helps in finding out if the program has be initalized and started running
void initStorageManager(void){
printf("********* The storage manager program has started running ! **********");
}
//first we need to create a new file which has nothing that means it is empty
RC createPageFile(char *fileName){
//This file tries to open the file based on the name 
FILE *file=fopen(fileName,"w");
//if statement checks if the file is equall to null 
//if it is null then it would mean that the file is not present
//then it would return file not found
if(file==NULL){
printf("The file that should be opened is not there. Try running the file again");
return RC_FILE_NOT_FOUND;
}
//Now we start by filling the page with zeros
//that is creating empty blocks
char emptyBlock[BLOCK_SIZE];
memset(emptyBlock,0,BLOCK_SIZE);
//now after creating the empty blocks enter them in the file
size_t written=fwrite(emptyBlock, sizeof(char), BLOCK_SIZE, file);
//if the block size is not equal to the contents written in then the write will fail
//if the write fails then the return function will be write failed.
if(written!=BLOCK_SIZE){
fclose(file);
printf("The operation of writing in the file has failed.");
return RC_WRITE_FAILED;
}
//if the write is done then the file is closed
fclose(file);
//the status RC_OK is returned
printf("The function has completed");
return RC_OK;
}
RC arun(){
return 0;
}
//now after the file is written the file that is present is opened 
//the function below opens the file 
RC openPageFile(char *fileName, SM_FileHandle *fHandle){
//open the file using the name of the file
FILE *file=fopen(fileName,"r+");
//if the file is not found then it would return file not found
if(file==NULL){
printf("The file that should be opened is not there");
return RC_FILE_NOT_FOUND;
}
//these are the filehandling methods
fHandle->fileName=fileName;
fHandle->mgmtInfo=file;
//This function is used to find the number of pages present in the file
fseek(file,0L,SEEK_END);
long fileSize=ftell(file);
//So in this if with the block size we can find out the number of pages
// if the block size is something then it will have the block size value 
fHandle->totalNumPages=fileSize / BLOCK_SIZE;
fHandle->curPagePos=0;
//if this function runs perfectly then it would return the value below
printf("The function has completed");
return RC_OK;
//ack sent
}
//closing the file
RC closePageFile(SM_FileHandle *fHandle){
FILE *file=(FILE *) fHandle->mgmtInfo;
//Returns file handle is not initated 
if(file==NULL){
printf("File not intiated");
return RC_FILE_HANDLE_NOT_INIT;
}
//close the file 
fclose(file);
//return ack
printf("The function has completed");
return RC_OK;
}
//Deleting or removing the file
RC destroyPageFile(char *fileName){
//if the file is not found then there is nothing to be deleted 
if(remove(fileName)!=0){
printf("file is not there");
return RC_FILE_NOT_FOUND;
}
//return ack
printf("the function has completed");
return RC_OK;
}
RC newarun(){
return 0;
}
//opening the file and reading the blocks in the file 
RC readBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){
FILE *file=(FILE *) fHandle->mgmtInfo;
//checks if the file is empty
if(file==NULL){
printf("file not initiated");
return RC_FILE_HANDLE_NOT_INIT;
}
//if the pages does not exist
if(pageNum>=fHandle->totalNumPages||pageNum<0){
printf("page does not exist");
return RC_READ_NON_EXISTING_PAGE;
}
//Go to the correct page using the current page number
printf("seeking");
fseek(file,pageNum*BLOCK_SIZE,SEEK_SET);
size_t readBytes=fread(memPage, sizeof(char), BLOCK_SIZE, file);
//this tries to read the block if this is not equal to the bytes then the page does not exist
if(readBytes!=BLOCK_SIZE){
printf("not present");
return RC_READ_NON_EXISTING_PAGE;
}
//file handling where we get the current position of using the page number
fHandle->curPagePos=pageNum;
printf("function has run");
//return ack
return RC_OK;
//this will only take place if the function runs
}
//to retrieve the position in the file the below function is used
int getBlockPos(SM_FileHandle *fHandle){
printf("getting the position of the block");
//this returns the current position in the page
return fHandle->curPagePos;
}
//to get the first block in the file 
RC readFirstBlock(SM_FileHandle *fHandle, SM_PageHandle memPage){
printf("reading the first block");
//returns the block
return readBlock(0,fHandle,memPage);
}
//to get the previous block from the current position
RC readPreviousBlock(SM_FileHandle *fHandle, SM_PageHandle memPage){
//checks if the page is present or not 
if(fHandle->curPagePos==0){
printf("the page is not present");
return RC_READ_NON_EXISTING_PAGE;
}
//this returns the previous block in the return statement
printf("reading previous block");
return readBlock(fHandle->curPagePos-1,fHandle,memPage);
}
//to read the current block from the file we use the below function
RC readCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage){
//here we do not check if the page or block is present or not since it is unnecessary
printf("reading current block");
return readBlock(fHandle->curPagePos, fHandle, memPage);
}
//here in the next function we check for the next block
RC readNextBlock(SM_FileHandle *fHandle, SM_PageHandle memPage){
//here we need to check if the current block is present or not
if(fHandle->curPagePos>=fHandle->totalNumPages-1){
printf("the page is not present");
return RC_READ_NON_EXISTING_PAGE;
}
//if the block is present then we read the next block and return it.
printf("reading the consicutive block");
return readBlock(fHandle->curPagePos+1,fHandle,memPage);
}
//like the previous functions we read the last block of the page
RC readLastBlock(SM_FileHandle *fHandle, SM_PageHandle memPage){
//this return statement returns the last block
printf("reading the final block");
return readBlock(fHandle->totalNumPages-1,fHandle,memPage);
}
//to write a block we use the below function
//in this functions we have multiple things which are essential
RC writeBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){
FILE*file = (FILE*) fHandle->mgmtInfo;
//First we check if the file is present or not
if(file==NULL){
printf("file is not present");
return RC_FILE_HANDLE_NOT_INIT;
}
//now we check if we are able to write or not
if(pageNum>=fHandle->totalNumPages){
printf("the write operation did not run");
return RC_WRITE_FAILED;
}
//Now we start by going to the place and writing the blocks
//seek function is used for that 
printf("seeking");
fseek(file,pageNum*BLOCK_SIZE,SEEK_SET);
//below is the write function
size_t written=fwrite(memPage,sizeof(char),BLOCK_SIZE,file);
//Again we check if the block is written or not
//if not then we return that the write has failed 
if(written!=BLOCK_SIZE){
printf("the write operation has failed");
return RC_WRITE_FAILED;
}
//file handling is done below
fHandle->curPagePos=pageNum;
//ack is given if the funciton runs using the below return statement 
printf("the function is completed");
return RC_OK;
}
//funtion to write the current block
RC writeCurrentBlock(SM_FileHandle *fHandle,SM_PageHandle memPage){
//the below written function returns the block as the argument 
printf("going to write the block");
return writeBlock(fHandle->curPagePos,fHandle,memPage);
}
//adding a block to the file that is present
RC appendEmptyBlock(SM_FileHandle *fHandle){
FILE *file=(FILE*)fHandle->mgmtInfo;
//here we check if the file is present or not
if(file==NULL){
printf("file not initiated");
return RC_FILE_HANDLE_NOT_INIT;
}
//inserting a block that doesn't have content
char emptyBlock[BLOCK_SIZE];
memset(emptyBlock,0,BLOCK_SIZE);
//now we seek to the position that we need to add the block to
//the below seek function is used to seek to the location
fseek(file,0L,SEEK_END);
size_t written=fwrite(emptyBlock,sizeof(char),BLOCK_SIZE,file);
//we check if are able to write
if(written!=BLOCK_SIZE){
printf("write operation did not complete");
return RC_WRITE_FAILED;
}
//after the block is added then we increase the number of pages
fHandle->totalNumPages++;
//ack is sent using the below return statement
printf("the function has run");
return RC_OK;
}
//Below we have a funtion which is used to check whether ther is some pages in the file
RC ensureCapacity(int numberofPages,SM_FileHandle *fHandle){
if(fHandle->totalNumPages<numberofPages){
//delcaring the vaiables
int pagesToAdd=numberofPages-fHandle->totalNumPages;
for(int i=0;i<pagesToAdd;i++){
//this call the function that will add the block
printf("calling appendblock function");
appendEmptyBlock(fHandle);
}
}
//after the function runs
//ack is sent from this return statement
printf("the function has run");
return RC_OK;
}