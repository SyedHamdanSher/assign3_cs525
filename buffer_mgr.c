#include <stdio.h>
#include "buffer_mgr.h"
#include "storage_mgr.h"
#include "dberror.h"
#include <stdlib.h>
#include <string.h>

typedef struct bufferPoolInfo* bufferNode;
typedef struct pageFrame* pageNode;
RC fifo_Technique (BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pageNum);
RC lru_Technique (BM_BufferPool *const bm, BM_PageHandle *const page,const PageNumber pageNum);

/******************************************Creating structure for bufferPool and pageFrames************************************/
//stores important information about buffer manager for mgmtdata in bufferPoolInfo nodes
struct bufferPoolInfo{

	int num_Read_IO;// stores total number of Read IO
	int num_Write_IO;//stores total number of Write IO
	bool page_Frame_Dirty[FRAMEMAX];// maintains dirty flags for each pageFrames
	int page_Frame_Fixed_Count[FRAMEMAX];//maintains fixed count of each pageFrame
		
	int pages_To_Page_Frame[PAGEMAX];//mapping of pageNumbers to page frames
	int page_Frames_To_Page[FRAMEMAX];//mapping of page frames to pages
	int total_Frames_Filled;//stores total number of filled frames count
	int max_Frames;//stores number of pageFrames inside buffer pool
	int lRU_Counter_PageFrame[FRAMELRU];// LRU counter information
	int total_Count_Pages;//stores total number of pages in frames of bufferPool

	SM_FileHandle filePointer;//stores file address of file
	pageNode head; //head of page frames linked list
	pageNode tail;//tail of page frames linked list
	pageNode lastNode;//maintains last node address of page frames list
};
	
//keeps the information regarding each node in pageFrames nodes
struct pageFrame{
	bool dirty_Bit; //dirty bit for page  true=dirty false= Not dirty
	int page_Number;//page number stored in buffer pageFrame
	int page_Frame_No;//frame number in page frames linked list
		
	int fixed_Count_Marked;//fixed count to mark usage of page by client
	int pinning; //pinning and unpinnig of page
	int filled; // whether frame is filled or not	
		
	char *data;//stores content of page.
	pageNode next;//pointer to next node in page frames linked list
	pageNode previous;//pointer to previous node in page frames linked list
};

/***********************************************************************************************************************************/

/******************************************Initializing Nodes for bufferPool and pageFrames*****************************************/
	//called when new page frame is created during initialization of buffer, each information is initialized with default value.
	pageNode getNewNode(){
		pageNode Node = calloc(PAGE_SIZE,sizeof(SM_PageHandle));
		Node->dirty_Bit=false;
		Node->page_Number=NO_PAGE;
		Node->page_Frame_No=0;
		
		Node->fixed_Count_Marked=0;
		Node->pinning=0;
		Node->filled=0;
		Node->next=NULL;
		Node->previous=NULL;
		Node->data=(char *)calloc(PAGE_SIZE,sizeof(SM_PageHandle));;
		return Node;
	}

	//storing the important information about buffer manager during initialization of buffer. 
	bufferNode initBufferPoolInfo(const int numPages,SM_FileHandle fileHandle){
		bufferNode bufferPool=calloc(PAGE_SIZE,sizeof(SM_PageHandle));
		bufferPool->num_Read_IO=0;
		bufferPool->num_Write_IO=0; 
		bufferPool->total_Frames_Filled=0;
		bufferPool->total_Count_Pages=0;
		bufferPool->max_Frames=numPages;//setting to number of frames maintained by BufferMananger
		bufferPool->filePointer=fileHandle;//file used by buffer manager
		
		//allocating memory
		memset(bufferPool->pages_To_Page_Frame,NO_PAGE,PAGEMAX*sizeof(int));
		memset(bufferPool->page_Frames_To_Page,NO_PAGE,FRAMEMAX*sizeof(int));
		memset(bufferPool->lRU_Counter_PageFrame,NO_PAGE,FRAMELRU*sizeof(int));
		memset(bufferPool->page_Frame_Dirty,NO_PAGE,FRAMEMAX*sizeof(bool));
		memset(bufferPool->page_Frame_Fixed_Count,NO_PAGE,FRAMEMAX*sizeof(int));
		return bufferPool;
	}
/***********************************************************************************************************************************/

	//creates a new buffer pool with numPages page frames using the page replacement strategy strategy. 
	//The pool is used to cache pages from the page file with name pageFileName. Initially, all page frames should be empty. 
	//The page file should already exist, i.e., this method should not generate a new page file
	RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName, const int numPages, ReplacementStrategy strategy, void *stratData)
	{
		
		SM_FileHandle fileHandle;
		int i=1;
		//openingPageFile using storage_mgr.c function
		if (openPageFile ((char *)pageFileName, &fileHandle) == RC_OK){
		bufferNode bufferPool=initBufferPoolInfo(numPages,fileHandle);
		
		//setting bufferManager field values
		bm->numPages=numPages;
		bm->pageFile=(char *)pageFileName;
		bm->strategy=strategy;//stores strategy to be used by BM when the page is to be replaced in frames.
		bufferPool->head=bufferPool->tail=getNewNode();
		bufferPool->head->page_Frame_No=0;
		
		//creating pageFrame Nodes which equals to numPages and using int i variable to use it as page_Frame_No of each frames
		while(i<numPages){
			bufferPool->tail->next = getNewNode();
			bufferPool->tail->next->previous = bufferPool->tail; //insert in the front
			bufferPool->tail = bufferPool->tail->next; //linking the link lists
			bufferPool->tail->page_Frame_No = i;
			i++;
		}

		bufferPool->lastNode=bufferPool->tail;
		bm->mgmtData=bufferPool;//storing bufferPoolInfo to managementData of BufferManagement to be used by various functions of BufferManager
		return RC_OK;
		}else{
		return RC_FILE_NOT_FOUND;
		}
	}
	
	
	
	//this function returns bufferManagerInfo.
	bufferNode getMgmtInfo(BM_BufferPool *const bm){
		if(bm!=NULL){
		bufferNode mgmtInfo=(bufferNode)bm->mgmtData;
		return mgmtInfo;
		}
        else{
            return ((bufferNode)RC_FILE_NOT_FOUND);
        }
	}
	

	//this function returns page_Frame_Fixed_Count value at each pageFrames.
	int *getFixCounts (BM_BufferPool *const bm){
		if(bm!=NULL){
			bufferNode mgmtInfo=getMgmtInfo(bm);
			pageNode tmp=mgmtInfo->head;//starting from head
			while(tmp!=NULL){//traverse till there are no frames.
				//stores fixed count value at each page frame to an aaray
				(mgmtInfo->page_Frame_Fixed_Count)[tmp->page_Frame_No]=tmp->fixed_Count_Marked; 
				tmp=tmp->next;
			}
			free(tmp);
			return mgmtInfo->page_Frame_Fixed_Count;
		}
        else{
            
            return ((int*)RC_FILE_NOT_FOUND);
        }
	}
	
	//this function returns total num_Read_IO i.e. read operation done by BufferManager from disk
	int getNumReadIO (BM_BufferPool *const bm){
		if(bm!=NULL){
			bufferNode mgmtInfo=getMgmtInfo(bm);
			return mgmtInfo->num_Read_IO;
		}
        else{
            
            return RC_FILE_NOT_FOUND;
        }
	}

	
	//this function returns page_Frame_Dirty i.e. an array of dirtyFlags at each pageFrame of BufferManager
	bool *getDirtyFlags (BM_BufferPool *const bm){
		if(bm!=NULL){
		bufferNode mgmtInfo=getMgmtInfo(bm);
		pageNode tmp=mgmtInfo->head;
			while(tmp!=NULL){
				//stores dirty bit value at each page in page frame.
				(mgmtInfo->page_Frame_Dirty)[tmp->page_Frame_No]=tmp->dirty_Bit;
				tmp=tmp->next;
			}
		free(tmp);
		return mgmtInfo->page_Frame_Dirty;
		}
        else{
            
            return ((bool*)RC_FILE_NOT_FOUND);
        }
	}
	
	//this function returns total num_Write_IO i.e. Write operation done by BufferManager from disk
	int getNumWriteIO (BM_BufferPool *const bm){
		if(bm!=NULL){
		bufferNode mgmtInfo=getMgmtInfo(bm);
		return mgmtInfo->num_Write_IO;
		}
        else{
            
            return RC_FILE_NOT_FOUND;
        }
	}
	
	//this function marks page specified in page->pageNum as dirty
	RC markDirty (BM_BufferPool *const bm, BM_PageHandle *const page){
		if(bm!=NULL){
			bufferNode mgmtInfo=getMgmtInfo(bm);
			pageNode tmp=mgmtInfo->head;
			while(tmp!=NULL){
				if(tmp->page_Number==page->pageNum){//searches page to be marked dirty in page frames.
					tmp->dirty_Bit=true;//marking page as dirty.
				}
				tmp=tmp->next;
			}
			free(tmp);
            return RC_OK;
			
		}else{
			
			return RC_BUFFER_NOT_INITIALIZED;
		}
	}
	
	//unpins the page page. The pageNum field of page should be used to figure out which page to unpin.
	//unpins the page specified in page->pageNum only if the page has fixed_Count greater than 1.
	RC unpinPage (BM_BufferPool *const bm, BM_PageHandle *const page){
		if(bm!=NULL){
			bufferNode mgmtInfo=getMgmtInfo(bm);
			pageNode tmp=mgmtInfo->head;
			while(tmp!=NULL){
				if(tmp->page_Number==page->pageNum && tmp->fixed_Count_Marked>0){
					tmp->fixed_Count_Marked-=1;//decreasing fixed_Count_Marked value of page.
				}
				tmp=tmp->next;
			}
			free(tmp);
			return RC_OK;
		}else{
			
			return RC_BUFFER_NOT_INITIALIZED;
		}
        
    }
	
	

	//pins the page with page number pageNum.
	//The buffer manager is responsible to set the pageNum field of the page handle passed to the method
	RC pinPage (BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pageNum){
	if(bm!=NULL){
		
			if(bm->strategy==RS_FIFO){
				 fifo_Technique(bm,page,pageNum);//FIFO.
				
			}else if(bm->strategy==RS_LRU){
				lru_Technique(bm,page,pageNum);//LRU
			}
				return RC_OK;
			}else{
				
				return RC_BUFFER_NOT_INITIALIZED;
			}
	}
		
	//destroys a buffer pool. This method should free up all resources associated with buffer pool. 
	//For example, it should free the memory allocated for page frames. 
	//If the buffer pool contains any dirty pages, then these pages should be written back to disk before destroying the pool. 
	//It is an error to shutdown a buffer pool that has pinned pages.
	RC shutdownBufferPool(BM_BufferPool *const bm){
		if(bm!=NULL){
			forceFlushPool(bm);//flushes the page frames and writes dirty pages return to disk
			bufferNode mgmtInfo=getMgmtInfo(bm);
			pageNode tmp=mgmtInfo->head;
			int i;
			for(i=0;tmp!=NULL;i++){
				
				//assignes each page frames to head and releases head node.
				free(mgmtInfo->head->data);
				free(mgmtInfo->head);
				tmp=tmp->next;
				mgmtInfo->head=tmp;
			}
		//makes head and tail node of linked listpage frames to NULL
		mgmtInfo->head=NULL;
		free(tmp);
		mgmtInfo->tail=NULL;
		return RC_OK;
		}else{
			
			return RC_BUFFER_NOT_INITIALIZED;
		}
	}

	
	// causes all dirty pages (with fix count 0) from the buffer pool to be written to disk.
	RC forceFlushPool(BM_BufferPool *const bm){
		if(bm!=NULL){
			SM_FileHandle fileHandle;
			bufferNode mgmtInfo=getMgmtInfo(bm);
			pageNode tmp=mgmtInfo->head;
			if (openPageFile ((char *)(bm->pageFile), &fileHandle) == RC_OK){
			while(tmp != NULL){
			if(tmp->dirty_Bit){//true
			//write the page to disk 
            if(writeBlock(tmp->page_Number, &fileHandle, tmp->data) == RC_OK){
				tmp->dirty_Bit = false;
				(mgmtInfo->num_Write_IO)+=1;//updating num_write_IO count
			}else{
			  return RC_WRITE_FAILED;
			}
			}
			tmp = tmp->next;
			}
			free(tmp);
			return RC_OK;
		}else{
			
			return RC_FILE_NOT_FOUND;
		}
		}else{
			
			return RC_BUFFER_NOT_INITIALIZED;
		}
		
	}
	//should write the current content of the page back to the page file on disk.
	RC forcePage (BM_BufferPool *const bm, BM_PageHandle *const page){
		if(bm!=NULL){
			SM_FileHandle fileHandle;
			bufferNode mgmtInfo=getMgmtInfo(bm);
			pageNode tmp=mgmtInfo->head;
		if (openPageFile ((char *)(bm->pageFile), &fileHandle) == RC_OK){		
			while(tmp!=NULL){
				if(tmp->page_Number==page->pageNum && tmp->dirty_Bit){
					if(writeBlock(tmp->page_Number,&(mgmtInfo->filePointer),tmp->data)==RC_OK){
						tmp->dirty_Bit=false; // making dirty bit false
						mgmtInfo->num_Write_IO+=1; //incrementing num_write_IO by one 
					}
				}
				tmp=tmp->next;
			}
			free(tmp);
			return RC_OK;
		}else{
			
			return RC_FILE_NOT_FOUND;
		}
		}else{
			
			return RC_BUFFER_NOT_INITIALIZED;
		}
	}
	
	//this function returns an array page numbers stored in pagFrames
	int *getFrameContents (BM_BufferPool *const bm){
		bufferNode mgmtInfo=getMgmtInfo(bm);
		return mgmtInfo->page_Frames_To_Page;
	}
	
	
	//this function finds node with page_Number equals to pageNum and the nreturn the node
	pageNode findNode(BM_BufferPool *const bm,BM_PageHandle *const page,const PageNumber pageNum ){	 
	if(bm!=NULL){
			bufferNode mgmtInfo = getMgmtInfo(bm);
			pageNode find=mgmtInfo->head;
			   while(find!=NULL){
				   if(find->page_Number==pageNum){//compares frame page's page number with pageNum to be search
						return find;
					 }
					find=find->next;
				}
				return NULL;
		}else{
			
			return NULL;
		}
	}//
	
	//this function finds whether the page to be read by client is already there in memory or not
	pageNode findNodeinMemory(BM_PageHandle *const page,const PageNumber pageNum,BM_BufferPool *const bm){
		if(bm!=NULL){
		bufferNode mgmtInfo=getMgmtInfo(bm);
		pageNode tmp=mgmtInfo->head;
		pageNode find;
		 if((mgmtInfo->pages_To_Page_Frame)[pageNum] != NO_PAGE){
			if(((find = findNode(bm,page, pageNum)) != NULL)){
			//page to be read is already in memory? if true increase the fixed_Count_Marked of page
			find->fixed_Count_Marked+=1;
			page->pageNum = pageNum;
			page->data = find->data; 
			return find;
		}else{
			return NULL;
		}
		}else{
			return NULL;
		}
	}else{
		
		return NULL;
	}
	
	}
	
	//this function update page head information during replacement
	void updatePageHead(BM_BufferPool *const bm,pageNode node){
		if(bm!=NULL){
			bufferNode mgmtInfo=getMgmtInfo(bm);
			pageNode hd=mgmtInfo->head;
			//page to be replaced is at head of pageFrame linked list data structure? if yes, no replacement
			if(node==mgmtInfo->head){
				return;
			}
			else{
			//else if page to be replaced is last node in the page frames linked list, adjust page frames 
			if(node==mgmtInfo->lastNode){
				pageNode t = mgmtInfo->lastNode->previous;
				mgmtInfo->lastNode = t;
				t->next = NULL;
				hd->previous = node;
				node->next = hd;
				mgmtInfo->head=node;
				node->previous = NULL;
				mgmtInfo->head->previous=NULL;
				mgmtInfo->head=node;
				mgmtInfo->head->previous=NULL;
				return;
			}else{
			//page to be replaced is between node? adjust page frames 
				node->previous->next = node->next;
				node->next->previous = node->previous;
				hd->previous = node;
				node->next = hd;
				mgmtInfo->head=node;
				node->previous = NULL;
				mgmtInfo->head->previous=NULL;
				mgmtInfo->head=node;
				mgmtInfo->head->previous=NULL;
				return;
				}
			}
			}
			else
			{
			return;
			}
	}
	
	
	
	//this function is used at the time of replacement of pages in page frame
	RC updatePage(BM_BufferPool *const bm,BM_PageHandle *const page,pageNode node,const PageNumber pageNum){
		bufferNode mgmtInfo = getMgmtInfo(bm);
		RC flag;
		SM_FileHandle fileHandle;
		if(bm!=NULL){
			if ((flag = openPageFile ((char *)(bm->pageFile), &fileHandle)) == RC_OK){
			//page to be replaced is dirty?, write it back to disk before replacement 
			if(node->dirty_Bit){
				ensureCapacity(pageNum, &fileHandle);
				if((flag = writeBlock(node->page_Number,&fileHandle, node->data)) == RC_OK){
					(mgmtInfo->num_Write_IO)+=1;
			  }else{
				  return flag;
				}		
			}
			//use ensuing capacity function of assignment one to ensure capacity before reading the page
			ensureCapacity(pageNum, &fileHandle);
			//reading the pageNum from file
			if((flag = readBlock(pageNum, &fileHandle, node->data)) == RC_OK){
				(mgmtInfo->pages_To_Page_Frame)[node->page_Number] = NO_PAGE;
				node->page_Number = pageNum;
				(mgmtInfo->pages_To_Page_Frame)[node->page_Number] = node->page_Frame_No;
				(mgmtInfo->page_Frames_To_Page)[node->page_Frame_No] = node->page_Number;
				node->dirty_Bit = false;
				node->fixed_Count_Marked = 1;
				page->pageNum = pageNum;
				page->data = node->data;
				mgmtInfo->num_Read_IO+=1;
			}else
			{
				return flag;
			}
			return RC_OK;
			}else
			{
				return flag;
			}
		 	}else{
			
			return RC_BUFFER_NOT_INITIALIZED;
		}
	}
	
	//this function calls updatepagehead function to update page head
	void update(BM_BufferPool *const bm,pageNode node,const PageNumber pageNum){
		int i=0,j=0;
		for(i=0;i<bm->numPages;i++){
			j++;
		}
		updatePageHead(bm,node);
	}
	
	RC updatePageFrame(BM_BufferPool *const bm,BM_PageHandle *const page,pageNode node,const PageNumber pageNum){
		int i=0,j=0;
		RC flag;
		for(i=0;i<bm->numPages;i++){
			j++;
		}
		flag=updatePage(bm, page, node, pageNum);
		return flag;
	}
		
	//FIFO page replacement
	RC fifo_Technique (BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pageNum){
	if(bm!=NULL){
		bufferNode mgmtInfo=getMgmtInfo(bm);
		RC flag;
		pageNode status;
		int j;
		//page to be read is already in memory? if yes return pointer to that pageFrame
		if((status=(findNodeinMemory(page,pageNum,bm)))!=NULL){
		return RC_OK;
		}
		//there are any empty frames in memory? if yes then read page to that frame from disk file.
		if((mgmtInfo->total_Frames_Filled) < mgmtInfo->max_Frames){
			status = mgmtInfo->head;
			for(j=0;j<mgmtInfo->total_Frames_Filled;++j){
				status = status->next;
			}
			//increase the total page_Count in page frames.
			(mgmtInfo->total_Frames_Filled)+=1;
			update(bm, status,pageNum);
			flag = updatePageFrame(bm, page, status, pageNum);
			if(flag!=RC_OK){
				return flag;
			}
			return RC_OK;	
			}
			else{
			//pageFrame is not empty? if yes, search for a page which has come first i.e. fixed_Count_Marked=0
			status = mgmtInfo->lastNode;
			for(j=0;(status != NULL && status != NULL && status->fixed_Count_Marked != 0);j++){
				status = status->previous;
			}
			update(bm, status,pageNum);
			//read the page from disk file for replacement.
			flag = updatePageFrame(bm, page, status, pageNum);
			if(flag!=RC_OK){
				return flag;
			}
			return RC_OK;	
		}
		}else{
		
		return RC_BUFFER_NOT_INITIALIZED;
	}
}	
			

//LRU will find and replace the page which is least recently used by client
RC lru_Technique (BM_BufferPool *const bm, BM_PageHandle *const page,const PageNumber pageNum)
{
	if(bm!=NULL){
			RC flag;
			pageNode status;
			bufferNode mgmtInfo=getMgmtInfo(bm);
			int j;
			//page to be read is already in memory? if yes, return the pointer to page to client
			if((status = findNodeinMemory(page,pageNum,bm)) != NULL){
				update(bm, status,pageNum);
				return RC_OK;
			}
			//empty frames in memory? if yes, read page to that frame from disk file.
			if((mgmtInfo->total_Frames_Filled) < mgmtInfo->max_Frames){
				status = mgmtInfo->head;
				for(j=0;j<mgmtInfo->total_Frames_Filled;++j){
					status = status->next;
				}
				//if page to be read is not in memory
				mgmtInfo->total_Frames_Filled+=1;	
				//reading page
				if((flag = updatePageFrame(bm, page, status, pageNum)) == RC_OK){
					update(bm, status,pageNum);
				}else{
				return flag;
				}
				return RC_OK;
			}
			else{
				//search for a page which is least recently used in past i.e. fixed_Count_Marked 0.
				status = mgmtInfo->lastNode;      
				while(status != NULL && status->fixed_Count_Marked != 0){
					status = status->previous;
				}
				//frame is found with page fixed_Count_Marked 0? if yes, read the page to that page frame.
				//else return error
				if (status != NULL){
					if((flag = updatePageFrame(bm, page, status, pageNum)) == RC_OK){
						update(bm, status,pageNum);
					}else{
						return flag;
					}
					return RC_OK;
				}else{
					
				return RC_NO_MORE_EMPTY_FRAME;
				}
			}
    	}else{
		
		return RC_BUFFER_NOT_INITIALIZED;
	}
}

	

	
	
