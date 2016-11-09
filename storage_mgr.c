#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "storage_mgr.h"
#include "dberror.h"

/* manipulating page files */

FILE *file;//Global file variable used by all functions
int blockTobeRead;// Global varaible to store page number, to be used by all Read functions

//This functions intializes the Storage Manager. Returns void
void initStorageManager(void){

}

void allocateMemory(char **total){
	*total = (char *) (calloc(PAGE_SIZE, sizeof(char))); 
}

//This creates a pagefile and intializes a memory allocation using calloc
RC createPageFile (char *fileName) {

    file = fopen(fileName, "w");
    int totalSize;
	//memory allocation
	char *total; 
	allocateMemory(&total);
    strcat(total,"1\t");
    totalSize=fwrite(total, sizeof(char), PAGE_SIZE, file);
    fclose(file);
    return RC_OK;

}

//Opens an existing page file,fields of this file handle should be initialized with the information about the opened file.
RC openPageFile (char *fileName, SM_FileHandle *fHandle) {
    FILE *file;
	char *string=(char *) calloc(PAGE_SIZE, sizeof(char));
	if(!(file=fopen(fileName,"r+"))){
			RC_message="Desired file doesn't exist";
			return RC_FILE_NOT_FOUND;
		}
		else{
        fgets(string, PAGE_SIZE, file);
		fHandle->curPagePos = 0;
        string = strtok(string, "\t");
        fHandle->mgmtInfo = file;
        fHandle->totalNumPages = atoi(string);
        fHandle->fileName = fileName;
        return RC_OK;
	}
}


//Close an open page file
RC closePageFile (SM_FileHandle *fHandle){
    fHandle=NULL;
	if(file!=NULL){
	fclose(file);
	file=NULL;
	}	
	free(file);
	return RC_OK;

}
//This function destroys a file. It first checks if the file exists.
RC destroyPageFile (char *fileName){
    remove(fileName);
		return RC_OK;

}

RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {//A page handle is an pointer to an area in memory storing the data of a page
//The method reads the pageNumth block from a file and stores its content in the memory pointed to by the memPage page handle. If the file has less than pageNum pages, the method should return RC_READ_NON_EXISTING_PAGE.
	if(fHandle!=NULL){
			int success;
			//checking the desired page to be read is within limit of totalNumPages also checks numPage should be more than 0
			if(((pageNum)<=(fHandle->totalNumPages)) && (pageNum)>=0){
					success=fseek(fHandle->mgmtInfo,(((pageNum+1)*PAGE_SIZE)),SEEK_SET);//seek to desired position to be read
					fread(memPage,sizeof(char),PAGE_SIZE,fHandle->mgmtInfo);// Reading from file and writing data to memory
					fHandle->curPagePos=pageNum;//setting the current page position file pointer			
					return RC_OK;
			}
			else{
					RC_message="Page to be read doesn't exist in file";
					return RC_READ_NON_EXISTING_PAGE;
				}
		}else{
			RC_message="desired file related data is not initialized";
			return RC_FILE_HANDLE_NOT_INIT;
		}

}



int getBlockPos (SM_FileHandle *fHandle) {
//Return the current page position in a file
	return fHandle->curPagePos;
}

RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {

//Read the first respective last page in a file
	return readBlock(0,fHandle,memPage);

}

//Read the previous page relative to the curPagePos of the file. The curPagePos should be moved to the page that was read. If the user tries to read a block before the first page of after the last page of the file, the method should return RC_READ_NON_EXISTING_PAGE
RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {

	return readBlock(fHandle->curPagePos-1,fHandle,memPage);

}

// readCurrentBlock reads the curPagePosth page counted from the beginning of the file.
RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){

	return readBlock(fHandle->curPagePos, fHandle,memPage);
}

//Read the next page relative to the curPagePos of the file
RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){

	return readBlock(fHandle->curPagePos+1,fHandle,memPage);

}

//Read the first respective last page in a file
RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){

	return readBlock(fHandle->totalNumPages,fHandle,memPage);

}

RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
if(fHandle!=NULL){
			if(((pageNum)<=(fHandle->totalNumPages))&&(pageNum)>=0){
					fseek(fHandle->mgmtInfo,(((pageNum+1)*PAGE_SIZE)),SEEK_SET);//seek to position to be written
					fwrite(memPage,sizeof(char),PAGE_SIZE,fHandle->mgmtInfo);//writing to file from memory
					fHandle->curPagePos=pageNum;//updating current page file pointer
					return RC_OK;			
			}
	}else{
			RC_message="desired file related data is not initialized";
			return RC_FILE_HANDLE_NOT_INIT;
		 }
}

RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
////Write a page to disk using either the current position or an absolute position.
	int pos;
	pos = getBlockPos(fHandle);
	RC flag;
	flag = writeBlock(pos, fHandle, memPage);
	return flag;
}

RC appendEmptyBlock (SM_FileHandle *fHandle) {
//Increase the number of pages in the file by one. The new last page should be filled with zero bytes.
	int totalBytes;
		FILE *file;
		SM_PageHandle pageHandle;
			if(fHandle!=NULL){
			pageHandle = (char *) calloc(PAGE_SIZE, sizeof(char));
			fseek(fHandle->mgmtInfo,(((fHandle->totalNumPages+1)*PAGE_SIZE))*sizeof(char),SEEK_END);
			fwrite(pageHandle,PAGE_SIZE,sizeof(char),file);
			fHandle->totalNumPages = fHandle->totalNumPages + 1;
			fHandle->curPagePos = fHandle->totalNumPages;
			rewind(fHandle->mgmtInfo);
			fprintf(fHandle->mgmtInfo, "%d\n" , fHandle->totalNumPages);
			fseek(fHandle->mgmtInfo, (fHandle->totalNumPages + 1)*PAGE_SIZE*sizeof(char), SEEK_SET);
			free(pageHandle);
		}else{
			RC_message="desired file related data is not initialized";
			return RC_FILE_HANDLE_NOT_INIT;
		}
}

RC ensureCapacity(int numOfPages, SM_FileHandle *fHandle) {
//If the file has less than numberOfPages pages then increase the size to numberOfPages.
	int diff,i;
		if(fHandle!=NULL){
			if((fHandle->totalNumPages)<numOfPages){
				diff=numOfPages-(fHandle->totalNumPages);
				for(i=0;i<diff;i++){
					appendEmptyBlock(fHandle);
				}
				return RC_OK;
			}else{
				return RC_OK;
			}	
		}else{
			RC_message="desired file related data is not initialized";
			return RC_FILE_HANDLE_NOT_INIT;
		}
}
