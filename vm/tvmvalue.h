#ifndef _TLBASIC_VMVALUE_H_
#define _TLBASIC_VMVALUE_H_

#include "tbytecode.h"

typedef struct _tVMValue
{
	u8 type;
	union
	{
		s32    stackindex;
		ptr32  string_val;
		s16    short_val;
		s32    int_val;
		ptr32  ptr_val;
		u8     byte_val;
		s32    long_val;
		f32    float_val;
		f64    double_val;
	}value;
}tVMValue;


u32   tVMValue_Size(tVMValue* value);
int   tVMValue_FreeSelf(tVMValue* value);
int   tVMValue_HeapLoad(ptr32 addr,u8 type,tVMValue* result);
int   tVMValue_HeapStore(ptr32 addr,tVMValue* op1);
ptr32 tVMValue_HeapAllocMultiArray(s32 max_dimension,const s32* ranges,s32 startindex);

int  tVMValue_Add(tVMValue* op1,tVMValue* op2,tVMValue* result);
int  tVMValue_Sub(tVMValue* op1,tVMValue* op2,tVMValue* result);
int  tVMValue_Mul(tVMValue* op1,tVMValue* op2,tVMValue* result);
int  tVMValue_Div(tVMValue* op1,tVMValue* op2,tVMValue* result);
int  tVMValue_Mod(tVMValue* op1,tVMValue* op2,tVMValue* result);
int  tVMValue_Opp(tVMValue* op,tVMValue* result);

int  tVMValue_AND(tVMValue* op1,tVMValue* op2,tVMValue* result);
int  tVMValue_OR(tVMValue* op1,tVMValue* op2,tVMValue* result);

int  tVMValue_EQ(tVMValue* op1,tVMValue* op2,tVMValue* result);
int  tVMValue_NOTEQ(tVMValue* op1,tVMValue* op2,tVMValue* result);
int  tVMValue_LT(tVMValue* op1,tVMValue* op2,tVMValue* result);
int  tVMValue_LG(tVMValue* op1,tVMValue* op2,tVMValue* result);
int  tVMValue_LTEQ(tVMValue* op1,tVMValue* op2,tVMValue* result);
int  tVMValue_LGEQ(tVMValue* op1,tVMValue* op2,tVMValue* result);

int  tVMValue_ConvertType(tVMValue* op,u8 desttype);


#endif
