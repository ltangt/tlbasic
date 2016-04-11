#ifndef _TLBASIC_GLOBAL_H_
#define _TLBASIC_GLOBAL_H_
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tbytecode.h"


#define MAX_CHAR 128   // number of characters of normal name
#define MAX_MSG  256   // max number of characters of message
#define MAX_TAB  99    // normal hash table size
#define SMART_TAB 7    // smart hash table size
#define SMALL_TAB 23   // small hash table size

#ifndef MAX
#define MAX(a,b) ((a) > (b) ?(a):(b))
#endif
#ifndef MIN
#define MIN(a,b) ((a) < (b) ?(a):(b))
#endif





int   sizeofData(DataType type);

#define ENABLE_ERROR_OUT 

#ifdef  ENABLE_ERROR_OUT
#define error_out(msg) printf(msg);
#else
#define error_out(msg)
#endif


#endif

