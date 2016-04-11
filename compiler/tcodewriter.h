#ifndef _TLBASIC_CODEWRITER_H_
#define _TLBASIC_CODEWRITER_H_
#include "tbytecode.h"
#include "tfile.h"

#define NATIVEFUNC_GET 0
#define NATIVEFUNC_SET 1

void  tcodewriter_create();
void  tcodewriter_release();
char* tcodewriter_getbuf(u32* codelen);
long  tcodewriter_codelength(); 

void  tcodewriter_NOP();
void  tcodewriter_CONST_Integer(s32 value);
void  tcodewriter_CONST_Double(f64 value);
void  tcodewriter_CONST_String(const char* str);
void  tcodewriter_LOAD(u8 access, s32 address);
void  tcodewriter_STORE(u8 access,s32 address);
void  tcodewriter_FORCE_LOAD(u8 access, s32 address, u8 type);
void  tcodewriter_ADD();
void  tcodewriter_SUB();
void  tcodewriter_MUL();
void  tcodewriter_DIV();
void  tcodewriter_MOD();
void  tcodewriter_AND();
void  tcodewriter_OR();
void  tcodewriter_NOT();
void  tcodewriter_LG();
void  tcodewriter_LT();
void  tcodewriter_EQ();
void  tcodewriter_LT_EQ();
void  tcodewriter_LG_EQ();
void  tcodewriter_NOT_EQ();
void  tcodewriter_OPP();
void  tcodewriter_HEAP_LOAD(u8 type);
void  tcodewriter_HEAP_STORE(u8 type);
void  tcodewriter_ALLOC();
void  tcodewriter_ALLOC_ARRAY(u32 dimension);
void  tcodewriter_FREE();
void  tcodewriter_FREE_ARRAY(u32 dimension);
void  tcodewriter_PUSH(u8 type);
void  tcodewriter_POP(u32 count);
void  tcodewriter_POP_RESTOP(u32 count);
void  tcodewriter_CONVERT(u8 type);
void  tcodewriter_FJP(s32 size);
void  tcodewriter_TJP(s32 size);
void  tcodewriter_JMP(s32 size);
void  tcodewriter_CALL(s32 function_index);
void  tcodewriter_INVOKE(s32 native_index);
void  tcodewriter_RET(u32 param_bytes);


#endif	 // _TLBASIC_CODEWRITER_H_
