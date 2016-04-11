#ifndef _TLBASIC_CODEFILE_H_
#define _TLBASIC_CODEFILE_H_
#include <stdio.h>
#include "tbytecode.h"

/*=========================================================================*/
#define MAGIC                   0xBABEFACE /* code file magic flag */
#define MAGIC_VERSION           0x0100     /* code file version */

/*=========================================================================
* Data structures for construct the code file
*=========================================================================*/
typedef struct _tGlobal_Variable
{
//	u16    name_len;
//	u8*    name;
	u8     type;
}tGlobal_Variable;

typedef struct _tFunction
{
	u8     type;
//	u16    name_len;
//	u8*    name;
	u8*    code;
	u32    codesize;
}tFunction;

//typedef struct _tNativeFuncDesc
//{
//	u16 name_len;
//	u8* name;
//	u8  type;
//}tNativeFuncDesc;

typedef struct _tCodeFile
{
	u32   magic;              /* must be 0xBABEFACE */
	u16   magic_version;      /* version */

	u16   num_global_Variables; 
	tGlobal_Variable *global_Variables;

	u16   num_functions;
	tFunction *functions;

}tCodeFile;

//=========================================================================
// functions for construct and release the code file structure
//=========================================================================

tCodeFile* tcodefile_create(void); /* create a new empty code file structure */

int     tcodefile_load(tCodeFile* codefile,FILE* fp); /* load code to construct the code file structure */
/* result of the tcodefile_load function */
#define RET_LOAD_SUCCESS           0x0 
#define RET_LOAD_NOTCODEFILE       0x1
#define RET_LOAD_NOTMAGICVERSION   0x2
#define RET_LOAD_ERROR             0x4

void    tcodefile_release(tCodeFile* codefile); /* release the code file structure */


#endif
