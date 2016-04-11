#include "tNativeFunction.h"

#include "global.h"
#include "tmem.h"
#include "tstring.h"

#include "tVM.h"
#include "tVMValue.h"

/*=========================================================================*/
tVMValue        tNativeFunction_ParamValue;      /* a common temp VM Value */
int             tNativeFunction_Count;

extern tRunstack* tVM_runstack;


//========================================================================
// 取数据类型
//========================================================================
u8 tNativeFunction_PopDataType()
{
	u8 dataType;
	tRunstack_popU8(tVM_runstack,dataType);
	tRunstack_pushU8(tVM_runstack,dataType);
	return dataType;
}

//========================================================================
// 以Integer方式弹出1个参数
//========================================================================
s32 tNativeFunction_PopParamAsInteger()
{
	tRunstack_popVMValue(tVM_runstack,&tNativeFunction_ParamValue);
	if(tNativeFunction_ParamValue.type != IntegerType)
		tVMValue_ConvertType(&tNativeFunction_ParamValue,IntegerType);
	return tNativeFunction_ParamValue.value.int_val;
}

//========================================================================
// 以Byte方式弹出1个参数
//========================================================================
u8 tNativeFunction_PopParamAsByte()
{
	tRunstack_popVMValue(tVM_runstack,&tNativeFunction_ParamValue);
	if(tNativeFunction_ParamValue.type != ByteType)
		tVMValue_ConvertType(&tNativeFunction_ParamValue,ByteType);
	return tNativeFunction_ParamValue.value.byte_val;
}

//========================================================================
// 以long方式弹出1个参数
//========================================================================
s32 tNativeFunction_PopParamAsLong()
{
	tRunstack_popVMValue(tVM_runstack,&tNativeFunction_ParamValue);
	if(tNativeFunction_ParamValue.type != LongType)
		tVMValue_ConvertType(&tNativeFunction_ParamValue,LongType);
	return tNativeFunction_ParamValue.value.long_val;
}

//========================================================================
// 以Double方式弹出1个参数
//========================================================================
f64 tNativeFunction_PopParamAsDouble()
{
	tRunstack_popVMValue(tVM_runstack,&tNativeFunction_ParamValue);
	if(tNativeFunction_ParamValue.type != DoubleType)
		tVMValue_ConvertType(&tNativeFunction_ParamValue,DoubleType);
	return tNativeFunction_ParamValue.value.double_val;
}

//========================================================================
// 以string方式弹出1个参数
//========================================================================
u8* tNativeFunction_PopParamAsString()
{
	tRunstack_popVMValue(tVM_runstack,&tNativeFunction_ParamValue);
	if(tNativeFunction_ParamValue.type != StringType)
		tVMValue_ConvertType(&tNativeFunction_ParamValue,StringType);
	return (u8*)tNativeFunction_ParamValue.value.string_val;
}

//========================================================================
// 以Integer方式弹出1个参数
//========================================================================
void* tNativeFunction_PopParamAsPtr()
{
	tRunstack_popVMValue(tVM_runstack,&tNativeFunction_ParamValue);
	if(tNativeFunction_ParamValue.type != PtrType)
		tVMValue_ConvertType(&tNativeFunction_ParamValue,PtrType);
	return (void*)tNativeFunction_ParamValue.value.ptr_val;
}

//========================================================================
// 以Type方式弹出1个参数(结构)
//========================================================================
void* tNativeFunction_PopParamAsType()
{
	tRunstack_popVMValue(tVM_runstack,&tNativeFunction_ParamValue);
	if(tNativeFunction_ParamValue.type != TypeType)
		tVMValue_ConvertType(&tNativeFunction_ParamValue,TypeType);
	return (void*)tNativeFunction_ParamValue.value.ptr_val;
}

//========================================================================
// 将 0 （NULL）放到结果桟中
//========================================================================
void tNativeFunction_PushReturnAsNull()
{
	tNativeFunction_ParamValue.type = NullType;
	tNativeFunction_ParamValue.value.int_val = 0;
	tRunstack_pushVMValue(tVM_runstack,&tNativeFunction_ParamValue);
}

//========================================================================
// 将 Integer 数据放到结果桟中
//========================================================================
void tNativeFunction_PushReturnAsInteger(s32 integer)
{
	tNativeFunction_ParamValue.type = IntegerType;
	tNativeFunction_ParamValue.value.int_val = integer;
	tRunstack_pushVMValue(tVM_runstack,&tNativeFunction_ParamValue);	
}

//========================================================================
// 将 BYTE 数据放到结果桟中
//========================================================================
void tNativeFunction_PushReturnAsByte(u8 byte_value)
{
	tNativeFunction_ParamValue.type = ByteType;
	tNativeFunction_ParamValue.value.byte_val = byte_value;
	tRunstack_pushVMValue(tVM_runstack,&tNativeFunction_ParamValue);
}

//========================================================================
// 将 long 数据放到结果桟中
//========================================================================
void tNativeFunction_PushReturnAsLong(s32 long_value)
{
	tNativeFunction_ParamValue.type = LongType;
	tNativeFunction_ParamValue.value.long_val = long_value;
	tRunstack_pushVMValue(tVM_runstack,&tNativeFunction_ParamValue);
}

//========================================================================
// 将 Double 数据放到结果桟中
//========================================================================
void tNativeFunction_PushReturnAsDouble(f64 double_value)
{
	tNativeFunction_ParamValue.type = DoubleType;
	tNativeFunction_ParamValue.value.double_val = double_value;
	tRunstack_pushVMValue(tVM_runstack,&tNativeFunction_ParamValue);
}

//========================================================================
// 将 string 数据放到结果桟中
//========================================================================
void tNativeFunction_PushReturnAsString(u8* string)
{
	tNativeFunction_ParamValue.type = StringType;
	tNativeFunction_ParamValue.value.string_val = (ptr32)string;
	tRunstack_pushVMValue(tVM_runstack,&tNativeFunction_ParamValue);	
}

//========================================================================
// 将 Ptr指针 数据放到结果桟中
//========================================================================
void tNativeFunction_PushReturnAsPtr(void* ptr)
{
	tNativeFunction_ParamValue.type = PtrType;
	tNativeFunction_ParamValue.value.ptr_val = ptr;
	tRunstack_pushVMValue(tVM_runstack,&tNativeFunction_ParamValue);
}

//========================================================================
// 将 Type(结构)指针 数据放到结果桟中
//========================================================================
void tNativeFunction_PushReturnAsType(void* Type)
{
	tNativeFunction_ParamValue.type = TypeType;
	tNativeFunction_ParamValue.value.ptr_val = (ptr32)Type;
	tRunstack_pushVMValue(tVM_runstack,&tNativeFunction_ParamValue);
}
