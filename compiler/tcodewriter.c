#include "tcodewriter.h"

#include "global.h"
#include "tfile.h"

FILE* tcodewriter_os;
static void emitCode(u8 code);
static void emitDataType(u8 type);
static void emitString(const char* str);
static void emitInteger(s32 value);
static void emitDouble(f64 value);
static void emitIndex(s32 index);
static void emitAddress(s32 addr);
static void emitAccessFlag(u8 flag);

void tcodewriter_create() 
{
	tcodewriter_os = tmpfile();
	rewind(tcodewriter_os);
}

void tcodewriter_release()
{
	fclose(tcodewriter_os);
}

char* tcodewriter_getbuf(u32* codelen)
{
	char* codebuf;
	*codelen = ftell(tcodewriter_os);
	codebuf = (char*)malloc(*codelen);
	fseek(tcodewriter_os,0L,SEEK_SET);
	fread(codebuf,*codelen,1,tcodewriter_os);
	fseek(tcodewriter_os,*codelen,SEEK_SET);
	return codebuf;
}

long tcodewriter_codelength()
{
	return ftell(tcodewriter_os);
}

/**
* write data to code output stream
*/
static void emitCode(u8 code)
{
	fwrite(&code, sizeof(u8),1,tcodewriter_os);
}

static void emitDataType(u8 type)
{
	fwrite(&type,sizeof(u8),1,tcodewriter_os);
}

static void emitAccessFlag(u8 flag)
{
	fwrite(&flag,sizeof(u8),1,tcodewriter_os);
}

static void emitInteger(s32 value)
{
	fwrite(&value,sizeof(s32),1,tcodewriter_os);
}

static void emitDouble(f64 value)
{
	fwrite(&value,sizeof(f64),1,tcodewriter_os);
}

static void emitString(const char* str)
{
	u16 len = (u16)strlen(str);
	fwrite(&len,sizeof(u16),1,tcodewriter_os);
	fwrite(str,len,1,tcodewriter_os);
}

static void emitIndex(s32 index)
{
	fwrite(&index,sizeof(s32),1,tcodewriter_os);
}

static void emitAddress(s32 addr)
{
	fwrite(&addr,sizeof(s32),1,tcodewriter_os);
}

//////////////////////////////////////////////////////////////////////////
// writer instructions
void tcodewriter_NOP()
{
	emitCode(C_NOP);
}

void tcodewriter_CONST_Integer(s32 value)
{
	emitCode(C_CONST);
	emitDataType(IntegerType);
	emitInteger(value);
}

void tcodewriter_CONST_Double(f64 value)
{
	emitCode(C_CONST);
	emitDataType(DoubleType);
	emitDouble(value);
}

void tcodewriter_CONST_String(const char* str)
{
	emitCode(C_CONST);
	emitDataType(StringType);
	emitString(str);
}

void tcodewriter_LOAD(u8 access,s32 address)
{
	emitCode(C_LOAD);
	emitAccessFlag(access);
	emitAddress(address);
}

void tcodewriter_STORE(u8 access,s32 address)
{
	emitCode(C_STORE);
	emitAccessFlag(access);
	emitAddress(address);
}

void tcodewriter_FORCE_LOAD(u8 access, s32 address, u8 type)
{
	emitCode(C_FORCE_LOAD);
	emitAccessFlag(access);
	emitAddress(address);
	emitDataType(type);
}

void tcodewriter_ADD()
{
	emitCode(C_ADD);
}

void tcodewriter_SUB()
{
	emitCode(C_SUB);
}

void tcodewriter_MUL()
{
	emitCode(C_MUL);
}

void tcodewriter_DIV()
{
	emitCode(C_DIV);
}

void tcodewriter_MOD()
{
	emitCode(C_MOD);
}

void tcodewriter_AND()
{
	emitCode(C_AND);
}

void tcodewriter_OR()
{
	emitCode(C_OR);
}

void tcodewriter_NOT()
{
	emitCode(C_NOT);
}

void tcodewriter_LG()
{
	emitCode(C_LG);
}

void tcodewriter_LT()
{
	emitCode(C_LT);
}

void tcodewriter_EQ()
{
	emitCode(C_EQ);
}

void tcodewriter_LT_EQ()
{
	emitCode(C_LT_EQ);
}

void tcodewriter_LG_EQ()
{
	emitCode(C_LG_EQ);
}

void tcodewriter_NOT_EQ()
{
	emitCode(C_NOT_EQ);
}

void tcodewriter_OPP()
{
	emitCode(C_OPP);
}

void tcodewriter_HEAP_LOAD(u8 type)
{
	emitCode(C_HEAP_LOAD);
	emitDataType(type);
}

void tcodewriter_HEAP_STORE(u8 type)
{
	emitCode(C_HEAP_STORE);
	emitDataType(type);
}

void tcodewriter_ALLOC()
{
	emitCode(C_ALLOC);
}

void tcodewriter_ALLOC_ARRAY(u32 dimension)
{
	emitCode(C_ALLOC_ARRAY);
	emitInteger(dimension);
}

void tcodewriter_FREE()
{
	emitCode(C_FREE);
}

void tcodewriter_FREE_ARRAY(u32 dimension)
{
	emitCode(C_FREE_ARRAY);
	emitInteger(dimension);
}

void tcodewriter_PUSH(u8 type)
{
	emitCode(C_PUSH);
	emitDataType(type);
}

void tcodewriter_POP(u32 count)
{
	emitCode(C_POP);
	emitInteger(count);
}

void tcodewriter_POP_RESTOP(u32 count)
{
	emitCode(C_POP_RESTOP);
	emitInteger(count);
}

void tcodewriter_CONVERT(u8 type)
{
	emitCode(C_CONVERT);
	emitDataType(type);
}

void tcodewriter_FJP(s32 size)
{
	emitCode(C_FJP);
	emitAddress(size);
}

void tcodewriter_TJP(s32 size)
{
	emitCode(C_TJP);
	emitAddress(size);
}

void tcodewriter_JMP(s32 size)
{
	emitCode(C_JMP);
	emitAddress(size);
}

void tcodewriter_CALL(s32 function_index)
{
	emitCode(C_CALL);
	emitIndex(function_index);
}

void tcodewriter_INVOKE(s32 native_index)
{
	emitCode(C_INVOKE);
	emitIndex(native_index);
}

void tcodewriter_RET(u32 param_bytes)
{
	emitCode(C_RET);
	emitInteger(param_bytes);
}
