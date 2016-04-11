#ifndef _TLBASIC_GLOBAL_H_
#define _TLBASIC_GLOBAL_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "tbytecode.h"
#include "tmem.h"

#define MAX_CHAR 128   /* number of characters of normal name */
#define MAX_MSG  256   /* max number of characters of message */
#define MAX_FILENAME_LEN 32 /* max length of the file names */
#define MAX_TAB  99    /* normal hash table size */
#define SMART_TAB 7    /* smart hash table size */
#define SMALL_TAB 23   /* small hash table size */

#ifndef MAX
#define MAX(a,b) ((a) > (b) ?(a):(b))
#endif
#ifndef MIN
#define MIN(a,b) ((a) < (b) ?(a):(b))
#endif

#define RET_OK                       0
#define RET_ERROR                    (-1)
#define RET_COMPILE_SUCCESS          RET_OK 
#define RET_ERROR_COMPILE_COMPILE    (-11)
#define RET_ERROR_COMPILE_SOURCEFILE (-12)
#define RET_ERROR_COMPILE_CODEFILE   (-13)
#define RET_ERROR_COMPILE_LISTFILE   (-14)

#define RET_ERROR_FILE_NOTEXIST      (-20)
#define RET_ERROR_FILE_ALREADYOPEN   (-21)

#define ENABLE_ERROR_OUT 

#ifdef  ENABLE_ERROR_OUT
#define error_out(msg) printf(msg);
#else
#define error_out(msg)
#endif

int  sizeofData(DataType type);
extern FILE* g_listfp;
extern FILE* g_codefp;

#endif

