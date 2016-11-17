 
*********************************************************
 ASSIGNMENT THREE
 
*********************************************************
 ( Name, CWID, Email id, Leader Name)
1. Ericson Akatakpo, A20349354, eakatakp@hawk.iit.edu, Ericson Akatakpo
2. Syed Hamdan Sher, A20378710, ssher1@gmail.com, Ericson Akatakpo
3. James Mwakichako, A20297757, jmwakich@hawk.iit.edu, Ericson Akatakpo
4. Gaurav Gadhvi, A20344904, ggadhvi@gmail.com, Ericson Akatakpo

*********************************************************

The execution process for the Record Manager begins by building the code with the Make file is responsible for creating the test_assign3_1
Binary file and also links the test_assign3_1.c with all corresponding *.c and *.h files.the Make file also links each file in the folder 
with each other.

*********************************************************
Extra Credit Implementation :- Tombstone functionality was implemented to efficiently manage the deletion of record for better Space utilization.
three extra test cases were implemented in test_assign3_1 file


*********************************************************
	Steps to Execute the record manager:
1) Copy all the files to one folder
2) In a Unix Terminal, navigate to the folder where files are stored.
3) Execute the make file with the "make" command
4) Execute test cases with the  "./test_assign3_1" command
5) Execute "make expr" command to run
6) Execute "./expr" command to run




*********************************************************

THE STRUCTURE OF THE RECORD MANAGER AND DEFINITIONS 

*********************************************************

buffer_mgr.h  ==== >  It contains the  definition for various structures and functions used by buffer_mgr.c. 

buffer_mgr.c ==== >  This is the main file and it contains the Buffer manager function which is used to initialize buffer as per the number of PageFrames. 
		     Its function is to copy file in pages from disk to PageFrame. It also checks for fixedcount of Pages and Dirty bits to write the edited
 		     page back to the disk. It has the FIFO and LRU agorithm to evict pages based on the algorithm strategy if the page frames are full.



buffer_mgr_stat.c ==== >  This file is made up of the BufferPool statistic functions.

buffer_mgr_stat.h ==== >  This file made up of the BufferPool statistic functions definitions.

dberror.c ==== >  It is made up of the printerror and errormessage function. For every specific error its function is to outputs the error code and the specific error message

dberror.h ==== >  The function of this is to define the page_size constant as 4096 and definition for RC(return codes) for various File and Block functions in the storage_mgr.
 

dt.h ==== >  It is made up of the constant variables for either True and False. 
 


storage_mgr.h ==== >  This file responsible contains various file and block functions which are called from test_assign1_1.c 
		     It also contains the read and write functions from file on disk to buffer and vise-versa. 
		      It contains creating, opening and closing file functions as well and also implements the dberror.h and test_helpers.h file.
 


test_helper.h ==== >  this contains validation tasks for each function that are called within test_assign1. 
		      It also prints the RC code and RC message passed based on function's success or failure which depends on the values being passed to it.



test_expr.c ==== >  The function of this file is to tests the serialize and descerialize value of tuples,  boolean opeartors and compares value as well as test complex expressions. 

		    One extra test case is implemented in the test_expr file.



tables.h ==== >  The function of this file is to defines the basic structure for schemas, tables, records, records ids and values. It is made up of functions to serialize data structures as string.



re_serializer.c ==== >  This file is made up functions to serialize tuples of table for various table fucntions like scan, update, and delete. It also seralizes schema as per different datatypes. 
                       It is made up functions for seralizing records and attributes of table. Serializes data value as per different datatype.



record_mgr.c ==== >  This file contains all the functions for the record manager. We can create, update, delete and insert in tables. 
		     We can search records within a table using scan functions. 
		     We can aslo Create table schema as well as initiating and shutting down record manager.




*********************************************************
Following are record manager functions:-


*********************************************************


1. updateRecordInfo(Record *record, int pageNumber,int slotNumber) ::: The function helps to updates the record based on record id passed and  the tombstone infomation of a record which is deleted.



2. processTombStone(recordTableInfo *recordTable) ::: The major duty of this function is to find the head of tombstone node for a table.



3. *stringFromTableMaker(recordTableInfo *recordTable,VarString *varString)::: The major duty of this function is to converts the table information to string for tombstones records using varstring and append function.



4. *translateTableToString(recordTableInfo *recordTable) ::: The major duty of this function is to converts the table information to string using varstring and append function.



5. processTombData(recordTableInfo *recordTable,char **s1,char **s2) ::: The major duty of this function is to process the tombstone data using record page number and slot number.



6. TranslateStringToRecordTable(char *recordTableInfoString) : The major duty of this function is to convert the string to record table information to create table in Page.



7. dSchema(char *stringSchema,char temp,int count) ::: The major duty of this function is to convert the value from table into string output.



8. tableInformationToFileData(char *name,char *temp,recordTableInfo *recordTable) ::: The major duty of this function is to convert table information to file data.



9. createTable (char *name, Schema *schema) ::: The major duty of this function is to create a table in page file with different datatypes and with custom defined schema size.



10. initBufferManagerForRecord(char *name,struct BM_BufferPool *bManager,struct BM_PageHandle *pageHandle) ::: The major duty of this function is to initialize the record manager by caling the initBufferpool function and PinPage function.



11. shutdownRecordManager () ::: The major duty of this function is to shutdown record manager.



12. openTable (RM_TableData *rel, char *name) ::: The major duty of this function is to open the table in page file to scan, update, insert, and delete from table.



13. closeTable (RM_TableData *rel) ::: The major duty of this function is to closs the table in page file and frees up the record manager memory.



14. deleteTable (char *name) ::: The major duty of this function is to deletes the table from page file.



15. getNumTuples (RM_TableData *rel) ::: The major duty of this function is to get the number of tuples from a relation.



16. insertRecord (RM_TableData *rel, Record *record) ::: The major duty of this function is to add a record to a table in page file.



17. deleteRecord (RM_TableData *rel, RID id) ::: The major duty of this function is to search a record in table and then delete a record from relation.



18. updateRecord (RM_TableData *rel, Record *record) ::: The major duty of this function is to search a record in table and then update the record of a relation.



19. getRecord (RM_TableData *rel, RID id, Record *record) ::: This function will be called by updaterecord and deleterecord to find the tuple in the relation.



20. startScan (RM_TableData *rel, RM_ScanHandle *scan, Expr *cond) ::: The major duty of this function is to find the matching records fro the relation as per the condition passed as expression passed by client.



21. next (RM_ScanHandle *scan, Record *record) ::: The major duty of this function is to help in scanning the relaion to find the matching records which fulfills the condition.



22. closeScan (RM_ScanHandle *scan) ::: The major duty of this function is to close the scanning of relaiton.



23. getRecordSize (Schema *schema) ::: The major duty of this function is to get the record size of relation.



24. *createSchema (int numAttr, char **attrNames, DataType *dataTypes, int *typeLength, int keySize, int *keys) ::: The major duty of this function is to create table schema with different attibutes, datatypes, and keys.



25. freeSchema (Schema *schema) ::: The major duty of this function is to free the relation schema.



26. createRecord (Record **record, Schema *schema) ::: The major duty of this function is to create a new record in a relation.



27. freeRecord (Record *record) ::: The major duty of this function is to free the data associated with record and then it frees the record.



28. getAttr (Record *record, Schema *schema, int attrNum, Value **value) ::: The major duty of this function is to get the attribute value from relation.



29. setAttr (Record *record, Schema *schema, int attrNum, Value *value) ::: The major duty of this function is to gset the attribute values of relation.



30. checkTombStone(Record *record,RM_TableData *rel,RID id,int *pageNumber,int *slotNumber, int *tcount) ::: The major duty of this function is to implement the tombstone functionality in relation.


*********************************************************
CUSTOM STRUCTURES:-

*********************************************************

*** We made a change to rm_serializer.c to APPEND(result, "%s", (i == ((schema->numAttr)-1)) ? "" : ","); in serializeRecord. to remove an extra comma in the end .

. recordTableInfo:- the recordTableInfo structure major function is to store the data related to table structure. Like numberoftuples, schemesize, recordstart, recordend, slotsize etc. 

2. Variabstring :::- The Variabstring structure is used to point data in buffer along with its size.



3. searchrecord :::- the searchrecord structure store data related to record of a table in terms of slot number and page number which will be used for scanning and searching records.



4. tombStoneNode :::- the tombStoneNode stores deleted records information like its RID and pointer in a linked list form.

 

expr.c ====> The expr.c file contains functions which evaluate an expression that client require during scan, update, and delete. It can also evaluate expression against records already contained in table.

rm_serializer.h ====> The rm_serializer.h file contains the attribute offset function along with varstring, string, size, and append functions which are called in record_mgr.c



test_assign3_1.c ====> The 
test_assign3_1.c file contains the main function which is used to call all the test function on record_mgr.c. 














