#ifndef _TLBASIC_CODEFILE_H_
#define _TLBASIC_CODEFILE_H_
#include "tfile.h"
#include "tbytecode.h"
#include "global.h"

//////////////////////////////////////////////////////////////////////////

#define MAGIC                   0xBABEFACE
#define MAGIC_VERSION           0x0100

#define MAX_GLOBAL_Variable    64*1024
#define MAX_FUNCTION            32*1024

typedef struct _tGlobal_Variable
{
	u16    name_len;
	u8*    name;
	u8     type;
}tGlobal_Variable;

typedef struct _tFunction
{
	u16    name_len;
	u8*    name;
	u8     type;
	u8*    code;
	u32    codesize;
}tFunction;

typedef struct _tNativeFuncDesc
{
	u16 name_len;
	u8* name;
	u8  type;
}tNativeFuncDesc;

typedef struct _tCodeFile
{
	u32   magic; /* 0xBABEFACE */
	u16   magic_version;

	u16   num_global_Variables;
	tGlobal_Variable global_Variables[MAX_GLOBAL_Variable];

	u16   num_functions;
	tFunction functions[MAX_FUNCTION];

}tCodeFile;


tCodeFile* tcodefile_create();
void       tcodefile_insert_globalvar(tCodeFile* codefile,const char *name,u8 type);
void       tcodefile_insert_function(tCodeFile* codefile,const char *name,u8 type,u8* code,u32 codesize);
void       tcodefile_release(tCodeFile* codefile);
void       tcodefile_toStream(tCodeFile* codefile,FILE* os);
void       tcodefile_print(tCodeFile* codefile);



#endif
