#ifndef DBERROR_H
#define DBERROR_H

#include "stdio.h"

/* module wide constants */
#define PAGE_SIZE 4096

/* return code definitions */
typedef int RC;

#define RC_OK 0
#define RC_FILE_NOT_FOUND 1
#define RC_FILE_HANDLE_NOT_INIT 2
#define RC_WRITE_FAILED 3
#define RC_READ_NON_EXISTING_PAGE 4

#define RC_RM_COMPARE_VALUE_OF_DIFFERENT_DATATYPE 200
#define RC_RM_EXPR_RESULT_IS_NOT_BOOLEAN 201
#define RC_RM_BOOLEAN_EXPR_ARG_IS_NOT_BOOLEAN 202
#define RC_RM_NO_MORE_TUPLES 203
#define RC_RM_NO_PRINT_FOR_DATATYPE 204
#define RC_RM_UNKOWN_DATATYPE 205

#define RC_IM_KEY_NOT_FOUND 300
#define RC_IM_KEY_ALREADY_EXISTS 301
#define RC_IM_N_TO_LAGE 302
#define RC_IM_NO_MORE_ENTRIES 303

#define RC_ERROR 401
#define RC_PAGE_MEM_OVERFLOW 402
#define RC_FILE_SEEK_FAILED 403

#define RC_BUFF_POOL_NOT_FOUND 404
#define RC_INVALID_PAGENUM 405
#define RC_WRITE_BACK_FAILED 406
#define RC_PAGE_ERROR 407

#define RC_PINNED_PAGES_IN_BUFFER 500
#define RC_RM_NO_TUPLE_WITH_GIVEN_RID 501
#define RC_SCAN_CONDITION_NOT_FOUND 502
#define RC_TABLE_FILE_NO_ACCESS 503
#define RC_KEY_NUM_TOO_LARGE 504
#define RC_IM_FAILURE 505

#define RC_IM_MEMORY_ALLOCATION_FAILED 701
#define RC_IM_MANAGEMENT_DATA_ERROR 702
#define RC_IM_NULL_TREE_HANDLE 703
#define RC_IM_NULL_POINTER 704
#define RC_IM_RID_NOT_FOUND 705
/* holder for error messages */
extern char *RC_message;

/* print a message to standard out describing the error */
extern void printError (RC error);
extern char *errorMessage (RC error);

#define THROW(rc,message) \
		do {			  \
			RC_message=message;	  \
			return rc;		  \
		} while (0)		  \

// check the return code and exit if it is an error
#define CHECK(code)							\
		do {									\
			int rc_internal = (code);						\
			if (rc_internal != RC_OK)						\
			{									\
				char *message = errorMessage(rc_internal);			\
				printf("[%s-L%i-%s] ERROR: Operation returned error: %s\n",__FILE__, __LINE__, __TIME__, message); \
				free(message);							\
				exit(1);							\
			}									\
		} while(0);


#endif
