//=========================================================================
// VM 的值计算部分
// WXC . 2005.8
//=========================================================================

#include "tVMValue.h"

#include "global.h"
#include "tmem.h"
#include "tstring.h"
#include "tbytecode.h"

s32 tVMValue_StringConvertToDate(u8* str_value);
u8* tVMValue_DateConvertToString(s32 date_value);

/* determine the type of result value of two operate values */
#define ARITH_RET_TYPE(type1,type2) MAX(type1,type2);

/*=========================================================================
* FUNCTION:      tVMValue_Size
* TYPE:          public interface
* OVERVIEW:      get the total size of a VM value
* INTERFACE:
*   parameters:  
*               value tVMValue*: the VM value
*   returns:   
*               u32 : the total size of a VM value
*=======================================================================*/
u32 tVMValue_Size(tVMValue* value)
{
	/* total size =  size of value data + size of value type */
	return sizeofData(datatypeType) + sizeofData(value->type); 
}

/*=========================================================================
* FUNCTION:      tVMValue_FreeSelf
* TYPE:          public interface
* OVERVIEW:      free the temp heap memory  the VM value pointed
* INTERFACE:
*   parameters:  
*               value tVMValue*: the VM value
*   returns:   
*               1: success
*=======================================================================*/
int tVMValue_FreeSelf(tVMValue* value)
{
	/* only string is the temp heap memory of the VM value */
	if(value->type == StringType && value->value.string_val != NULL)
		mem_free(value->value.string_val);

	return 1;
}

/*=========================================================================
* FUNCTION:      tVMValue_HeapLoad
* TYPE:          public interface
* OVERVIEW:      load a VM value from the heap memory
* INTERFACE:
*   parameters:  
*                     addr ptr32: the address of the heap memory
*                        type u8: the of type of the loading VM value
*               result tVMValue*: the VM value
*   returns:   
*               1: success
*=======================================================================*/
int tVMValue_HeapLoad(ptr32 addr,u8 type,tVMValue* result)
{
	result->type = type;
	switch(type)
	{
	case IntegerType:
	case DateType:
		result->value.int_val = *(s32*)addr;
		break;
	case StringType:
		result->value.string_val = str_copynew(*(ptr32*)addr);
		break;
	case ByteType:
		result->value.byte_val = *(u8*)addr;
		break;
	case ShortType:
		result->value.short_val = *(s16*)addr;
		break;
	case FloatType:
		result->value.float_val = *(f32*)addr;
		break;
	case DoubleType:
		result->value.double_val = *(f64*)addr;
		break;
	case LongType:
		result->value.long_val = *(s32*)addr;
		break;
	case PtrType:
	case TypeType:
		result->value.ptr_val = *(ptr32*)addr;
		break;
	}
	return 1;
}

/*=========================================================================
* FUNCTION:      tVMValue_HeapStore
* TYPE:          public interface
* OVERVIEW:      store a VM value into the heap memory
* INTERFACE:
*   parameters:  
*                  addr ptr32: the address of the heap memory
*               op1 tVMValue*: the VM value to store
*   returns:   
*               1: success
*=======================================================================*/
int tVMValue_HeapStore(ptr32 addr,tVMValue* op1)
{
	switch(op1->type)
	{
	case IntegerType:
	case DateType:
		*(s32*)addr = op1->value.int_val;
		break;
	case StringType:
		if(*(ptr32*)addr != NULL)
			mem_free(*(ptr32*)addr);
		*(ptr32*)addr = op1->value.string_val;
		break;
	case ByteType:
		*(u8*)addr = op1->value.byte_val;
		break;
	case ShortType:
		*(s16*)addr = op1->value.short_val;
		break;
	case FloatType:
		*(f32*)addr = op1->value.float_val;
		break;
	case DoubleType:
		*(f64*)addr = op1->value.double_val;
		break;
	case LongType:
		*(s32*)addr = op1->value.long_val;
		break;
	case PtrType:
	case TypeType:
		*(ptr32*)addr = op1->value.ptr_val;
		break;
	}

	return 1;
}

/*=========================================================================
* FUNCTION:      tVMValue_HeapAllocMultiArray
* TYPE:          public interface
* OVERVIEW:      allocate multi-array heap memory
* INTERFACE:
*   parameters:  
*               max_dimension s32: the dimension the array
*               ranges const s32*: the ranges of every dimension.
*                      etc. new byte[10][11][12],the ranges = {10,11,12}
*               startindex s32   : the start index of the ranges
*   returns:   
*               ptr32: the head pointer of the multi-array
*=======================================================================*/
ptr32 tVMValue_HeapAllocMultiArray(s32 max_dimension,const s32* ranges,s32 startindex)
{
	s32 i;
	ptr32 head_p,p;

	head_p = mem_alloc(ranges[startindex]);
	p      = head_p;
	if(startindex+1 < max_dimension)
	{
		for(i=0;i<ranges[startindex];i+= sizeof(ptr32))
			*(ptr32*)(p+i) = tVMValue_HeapAllocMultiArray(max_dimension,ranges,startindex+1);
	}
	return head_p;
}

//=========================================================================
// Add
//=========================================================================
int tVMValue_Add(tVMValue* op1,tVMValue* op2,tVMValue* result)
{
	result->type = ARITH_RET_TYPE(op1->type,op2->type);
	tVMValue_ConvertType(op1,result->type);
	tVMValue_ConvertType(op2,result->type);
	switch(result->type)
	{
	case IntegerType:
	case DateType:
		result->value.int_val = op1->value.int_val + op2->value.int_val;
		break;
	case ByteType:
		result->value.byte_val = op1->value.byte_val + op2->value.byte_val;
		break;
	case LongType:
		result->value.long_val = op1->value.long_val + op2->value.long_val;
		break;
	case ShortType:
		result->value.short_val = op1->value.short_val+op2->value.short_val;
		break;
	case FloatType:
		result->value.float_val = op1->value.float_val+op2->value.float_val;
		break;
	case DoubleType:
		result->value.double_val = op1->value.double_val + op2->value.double_val;
		break;
	case StringType:
		result->value.string_val = str_addstr(op1->value.string_val,op2->value.string_val);
		break;
	}
	return 1;
}

//=========================================================================
// Sub
//=========================================================================
int tVMValue_Sub(tVMValue* op1,tVMValue* op2,tVMValue* result)
{
	result->type = ARITH_RET_TYPE(op1->type,op2->type);
	tVMValue_ConvertType(op1,result->type);
	tVMValue_ConvertType(op2,result->type);
	switch(result->type)
	{
	case IntegerType:
	case DateType:
		result->value.int_val = op1->value.int_val - op2->value.int_val;
		break;
	case ByteType:
		result->value.byte_val = op1->value.byte_val - op2->value.byte_val;
		break;
	case ShortType:
		result->value.short_val = op1->value.short_val - op2->value.short_val;
		break;
	case FloatType:
		result->value.float_val = op1->value.float_val - op2->value.float_val;
		break;
	case LongType:
		result->value.long_val = op1->value.long_val - op2->value.long_val;
		break;
	case DoubleType:
		result->value.double_val = op1->value.double_val - op2->value.double_val;
		break;
	}
	return 1;
}

//=========================================================================
// Mul. 乘
//=========================================================================
int tVMValue_Mul(tVMValue* op1,tVMValue* op2,tVMValue* result)
{
	result->type = ARITH_RET_TYPE(op1->type,op2->type);
	tVMValue_ConvertType(op1,result->type);
	tVMValue_ConvertType(op2,result->type);
	switch(result->type)
	{
	case IntegerType:
	case DateType:
		result->value.int_val = op1->value.int_val * op2->value.int_val;
		break;
	case ByteType:
		result->value.byte_val = op1->value.byte_val * op2->value.byte_val;
		break;
	case ShortType:
		result->value.short_val = op1->value.short_val * op2->value.short_val;
		break;
	case FloatType:
		result->value.float_val = op1->value.float_val * op2->value.float_val;
		break;
	case LongType:
		result->value.long_val = op1->value.long_val * op2->value.long_val;
		break;
	case DoubleType:
		result->value.double_val = op1->value.double_val * op2->value.double_val;
		break;
	}
	return 1;
}

//=========================================================================
// Div 除法
//=========================================================================
int tVMValue_Div(tVMValue* op1,tVMValue* op2,tVMValue* result)
{
	result->type = ARITH_RET_TYPE(op1->type,op2->type);
	tVMValue_ConvertType(op1,result->type);
	tVMValue_ConvertType(op2,result->type);
	switch(result->type)
	{
	case IntegerType:
	case DateType:
		result->value.int_val = op1->value.int_val / op2->value.int_val;
		break;
	case ByteType:
		result->value.byte_val = op1->value.byte_val / op2->value.byte_val;
		break;
	case ShortType:
		result->value.short_val = op1->value.short_val / op2->value.short_val;
		break;
	case FloatType:
		result->value.float_val = op1->value.float_val / op2->value.float_val;
		break;
	case LongType:
		result->value.long_val = op1->value.long_val / op2->value.long_val;
		break;
	case DoubleType:
		result->value.double_val = op1->value.double_val / op2->value.double_val;
		break;
	}
	return 1;
}

//=========================================================================
// Mod 求模
//=========================================================================
int tVMValue_Mod(tVMValue* op1,tVMValue* op2,tVMValue* result)
{
	result->type = ARITH_RET_TYPE(op1->type,op2->type);
	tVMValue_ConvertType(op1,result->type);
	tVMValue_ConvertType(op2,result->type);
	switch(result->type)
	{
	case IntegerType:
	case DateType:
		result->value.int_val = op1->value.int_val % op2->value.int_val;
		break;
	case ByteType:
		result->value.byte_val = op1->value.byte_val % op2->value.byte_val;
		break;
	case ShortType:
		result->value.short_val = op1->value.short_val % op2->value.short_val;
		break;
	case LongType:
		result->value.long_val = op1->value.long_val % op2->value.long_val;
		break;
	}
	return 1;
}

//=========================================================================
// Div 取负值
//=========================================================================
int tVMValue_Opp(tVMValue* op,tVMValue* result)
{
	result->type = op->type;
	switch(op->type)
	{
	case IntegerType:
	case DateType:
		result->value.int_val = -op->value.int_val;
		break;
	case ByteType:
		result->value.byte_val = -op->value.byte_val;
		break;
	case ShortType:
		result->value.short_val = -op->value.short_val;
		break;
	case FloatType:
		result->value.float_val = -op->value.float_val;
		break;
	case LongType:
		result->value.long_val = -op->value.long_val;
		break;
	case DoubleType:
		result->value.double_val = -op->value.double_val;
		break;
	}
	return 1;
}

//=========================================================================
// And 位与运算
//=========================================================================
int tVMValue_AND(tVMValue* op1,tVMValue* op2,tVMValue* result)
{
	result->type = ARITH_RET_TYPE(op1->type,op2->type);
	tVMValue_ConvertType(op1,result->type);
	tVMValue_ConvertType(op2,result->type);
	switch(result->type)
	{
	case ByteType:
		result->value.byte_val = op1->value.byte_val & op2->value.byte_val;
		break;
	case IntegerType:
	case DateType:
		result->value.int_val = op1->value.int_val & op2->value.int_val;
		break;
	case ShortType:
		result->value.short_val = op1->value.short_val & op2->value.short_val;
		break;
	case LongType:
		result->value.long_val = op1->value.long_val & op2->value.long_val;
		break;
	}
	return 1;
}

//=========================================================================
// OR 位或运算
//=========================================================================
int tVMValue_OR(tVMValue* op1,tVMValue* op2,tVMValue* result)
{
	result->type = ARITH_RET_TYPE(op1->type,op2->type);
	tVMValue_ConvertType(op1,result->type);
	tVMValue_ConvertType(op2,result->type);
	switch(result->type)
	{
	case ByteType:
		result->value.byte_val = op1->value.byte_val | op2->value.byte_val;
		break;
	case IntegerType:
	case DateType:
		result->value.int_val = op1->value.int_val | op2->value.int_val;
		break;
	case ShortType:
		result->value.short_val = op1->value.short_val | op2->value.short_val;
		break;
	case LongType:
		result->value.long_val = op1->value.long_val | op2->value.long_val;
		break;
	}
	return 1;
}

//=========================================================================
// ==  等于比较
//=========================================================================
int tVMValue_EQ(tVMValue* op1,tVMValue* op2,tVMValue* result)
{
	u8 ari_type = ARITH_RET_TYPE(op1->type,op2->type);
	tVMValue_ConvertType(op1,ari_type);
	tVMValue_ConvertType(op2,ari_type);
	switch(ari_type)
	{
	case ByteType:
		result->value.int_val = op1->value.byte_val == op2->value.byte_val? 1: 0;
		break;
	case IntegerType:
	case DateType:
		result->value.int_val = op1->value.int_val == op2->value.int_val ? 1:0;
		break;
	case LongType:
		result->value.int_val = op1->value.long_val == op2->value.long_val ? 1:0;
		break;
	case ShortType:
		result->value.short_val = op1->value.short_val == op2->value.short_val ? 1: 0;
		break;
	case FloatType:
		result->value.int_val = op1->value.float_val == op2->value.float_val ? 1: 0;
		break;
	case DoubleType:
		result->value.int_val = op1->value.double_val == op2->value.double_val ? 1:0;
		break;
	case StringType:
		result->value.int_val = strcmp(op1->value.string_val,op2->value.string_val) == 0? 1: 0;
		break;
	case PtrType:
	case TypeType: // compare the memory address of the array
		result->value.int_val = op1->value.ptr_val == op2->value.ptr_val ? 1: 0;
		break;
	}
	result->type = IntegerType;
	return 1;
}

//=========================================================================
// ！=  不等于比较
//=========================================================================
int tVMValue_NOTEQ(tVMValue* op1,tVMValue* op2,tVMValue* result)
{
	u8 ari_type = ARITH_RET_TYPE(op1->type,op2->type);
	tVMValue_ConvertType(op1,ari_type);
	tVMValue_ConvertType(op2,ari_type);
	switch(ari_type)
	{
	case IntegerType:
	case DateType:
		result->value.int_val = op1->value.int_val != op2->value.int_val ? 1:0;
		break;
	case ByteType:
		result->value.int_val = op1->value.byte_val != op2->value.byte_val? 1: 0;
		break;
	case LongType:
		result->value.int_val = op1->value.long_val != op2->value.long_val ? 1:0;
		break;
	case ShortType:
		result->value.short_val = op1->value.short_val != op2->value.short_val ? 1: 0;
		break;
	case FloatType:
		result->value.int_val = op1->value.float_val != op2->value.float_val ? 1: 0;
		break;
	case DoubleType:
		result->value.int_val = op1->value.double_val != op2->value.double_val ? 1:0;
		break;
	case StringType:
		result->value.int_val = strcmp(op1->value.string_val,op2->value.string_val) != 0? 1: 0;
		break;
	case PtrType:
	case TypeType: // compare the memory address of the array
		result->value.int_val = op1->value.ptr_val != op2->value.ptr_val ? 1: 0;
		break;
	}
	result->type = IntegerType;
	return 1;
}

//=========================================================================
// < 
//=========================================================================
int tVMValue_LT(tVMValue* op1,tVMValue* op2,tVMValue* result)
{
	u8 ari_type = ARITH_RET_TYPE(op1->type,op2->type);
	tVMValue_ConvertType(op1,ari_type);
	tVMValue_ConvertType(op2,ari_type);
	switch(ari_type)
	{
	case ByteType:
		result->value.int_val = op1->value.byte_val < op2->value.byte_val? 1: 0;
		break;
	case IntegerType:
	case DateType:
		result->value.int_val = op1->value.int_val < op2->value.int_val ? 1:0;
		break;
	case ShortType:
		result->value.short_val = op1->value.short_val < op2->value.short_val? 1 : 0;
		break;
	case FloatType:
		result->value.int_val = op1->value.float_val < op2->value.float_val ? 1: 0;
		break;
	case LongType:
		result->value.int_val = op1->value.long_val < op2->value.long_val ? 1:0;
		break;
	case DoubleType:
		result->value.int_val = op1->value.double_val < op2->value.double_val ? 1:0;
		break;
	}
	result->type = IntegerType;
	return 1;

}

//=========================================================================
// >
//=========================================================================
int tVMValue_LG(tVMValue* op1,tVMValue* op2,tVMValue* result)
{
	u8 ari_type = ARITH_RET_TYPE(op1->type,op2->type);
	tVMValue_ConvertType(op1,ari_type);
	tVMValue_ConvertType(op2,ari_type);
	switch(ari_type)
	{
	case ByteType:
		result->value.int_val = op1->value.byte_val > op2->value.byte_val? 1: 0;
		break;
	case IntegerType:
	case DateType:
		result->value.int_val = op1->value.int_val > op2->value.int_val ? 1:0;
		break;
	case ShortType:
		result->value.short_val = op1->value.short_val > op2->value.short_val? 1 : 0;
		break;
	case FloatType:
		result->value.int_val = op1->value.float_val > op2->value.float_val ? 1: 0;
		break;
	case LongType:
		result->value.int_val = op1->value.long_val > op2->value.long_val ? 1:0;
		break;
	case DoubleType:
		result->value.int_val = op1->value.double_val > op2->value.double_val ? 1:0;
		break;
	}
	result->type = IntegerType;
	return 1;
}

//=========================================================================
// <=
//=========================================================================
int tVMValue_LTEQ(tVMValue* op1,tVMValue* op2,tVMValue* result)
{
	u8 ari_type = ARITH_RET_TYPE(op1->type,op2->type);
	tVMValue_ConvertType(op1,ari_type);
	tVMValue_ConvertType(op2,ari_type);
	switch(ari_type)
	{
	case ByteType:
		result->value.int_val = op1->value.byte_val <= op2->value.byte_val? 1: 0;
		break;
	case IntegerType:
	case DateType:
		result->value.int_val = op1->value.int_val <= op2->value.int_val ? 1:0;
		break;
	case ShortType:
		result->value.short_val = op1->value.short_val <= op2->value.short_val? 1 : 0;
		break;
	case FloatType:
		result->value.int_val = op1->value.float_val <= op2->value.float_val ? 1: 0;
		break;
	case LongType:
		result->value.int_val = op1->value.long_val <= op2->value.long_val ? 1:0;
		break;
	case DoubleType:
		result->value.int_val = op1->value.double_val <= op2->value.double_val ? 1:0;
		break;
	}
	result->type = IntegerType;
	return 1;
}

//=========================================================================
// >=
//=========================================================================
int tVMValue_LGEQ(tVMValue* op1,tVMValue* op2,tVMValue* result)
{
	u8 ari_type = ARITH_RET_TYPE(op1->type,op2->type);
	tVMValue_ConvertType(op1,ari_type);
	tVMValue_ConvertType(op2,ari_type);
	switch(ari_type)
	{
	case ByteType:
		result->value.int_val = op1->value.byte_val >= op2->value.byte_val? 1: 0;
		break;
	case IntegerType:
	case DateType:
		result->value.int_val = op1->value.int_val >= op2->value.int_val ? 1:0;
		break;
	case ShortType:
		result->value.short_val = op1->value.short_val >= op2->value.short_val? 1 : 0;
		break;
	case FloatType:
		result->value.int_val = op1->value.float_val >= op2->value.float_val ? 1: 0;
		break;
	case LongType:
		result->value.int_val = op1->value.long_val >= op2->value.long_val ? 1:0;
		break;
	case DoubleType:
		result->value.int_val = op1->value.double_val >= op2->value.double_val ? 1:0;
		break;
	}
	result->type = IntegerType;
	return 1;
}

//=========================================================================
// 类型转换
//=========================================================================
// WXC modify 2005.08.30 增加了对操作数为指针类型的处理

int tVMValue_ConvertType(tVMValue* op,u8 desttype)
{
	char tempstr[MAX_CHAR];
	char valuestr[MAX_CHAR];
	s32 int_value;

	if(op->type == desttype) return 1;

	if(op->type == StringType && op->value.string_val != NULL && desttype!=PtrType)
	{
		strcpy(valuestr,op->value.string_val);
		mem_free(op->value.string_val);	
	}

	switch(op->type)
	{
	case IntegerType:
	case LongType:
		{
			int_value = op->value.int_val;
			switch(desttype)
			{
			case ByteType:
				op->value.byte_val = (u8)int_value;
				break;
			case ShortType:
				op->value.short_val = (s16)int_value;
				break;
			case LongType:
				op->value.long_val = (s32)int_value;
				break;
			case FloatType:
				op->value.float_val = (f32)int_value;
				break;
			case DoubleType:
				op->value.double_val = (f64)int_value;
				break;
			case StringType:
				sprintf(tempstr,"%d",(int)int_value);
				op->value.string_val = str_copynew(tempstr);
				break;
			case DateType:
				op->value.int_val = int_value;
				break;
			case PtrType:
				*((s32*)(op->value.ptr_val)) = (s32)int_value;
				break;
			case TypeType:
				op->value.ptr_val = (ptr32)int_value;
				break;
			}
			break;
		}
	case ByteType:
		{
			u8 byte_value = op->value.byte_val;
			switch(desttype)
			{
			case IntegerType:
				op->value.int_val = byte_value;
				break;
			case ShortType:
				op->value.short_val = (s16)byte_value;
				break;
			case FloatType:
				op->value.float_val = (f32)byte_value;
				break;
			case LongType:
				op->value.long_val = byte_value;
				break;
			case DoubleType:
				op->value.double_val = (f64)byte_value;
				break;
			case StringType:
				sprintf(tempstr,"%d",byte_value);
				op->value.string_val = str_copynew(tempstr);
				break;
			}
			break;
		}
	case ShortType:
		{
			s16 short_value = op->value.short_val;
			switch(desttype)
			{
			case ByteType:
				op->value.byte_val = (u8)short_value;
				break;
			case IntegerType:
			case LongType:
				op->value.long_val = (s32)short_value;
				break;
			case FloatType:
				op->value.float_val = (f32)short_value;
				break;
			case DoubleType:
				op->value.double_val = (f64)short_value;
				break;
			case StringType:
				sprintf(tempstr,"%d",(int)short_value);
				op->value.string_val = str_copynew(tempstr);
				break;
			case DateType:
				op->value.int_val = (s32)short_value;
				break;
			}
			break;
		}
	case FloatType:
		{
			f32 float_value = op->value.float_val;
			switch(desttype)
			{
			case ByteType:
				op->value.byte_val = (u8)float_value;
				break;
			case ShortType:
				op->value.short_val = (s16)float_value;
				break;
			case IntegerType:
			case LongType:
				op->value.long_val = (s32)float_value;
				break;
			case DoubleType:
				op->value.double_val = (f64)float_value;
				break;
			case StringType:
				sprintf(tempstr,"%f",(float)float_value);
				op->value.string_val = str_copynew(tempstr);
				break;
			case PtrType:
				*((s32*)(op->value.ptr_val)) = (s32)float_value;
				break;
			}
			break;
		}
	case DoubleType:
		{
			f64 double_value = op->value.double_val;
			switch(desttype)
			{
			case IntegerType:
				op->value.int_val = (s32)double_value;
				break;
			case ByteType:
				op->value.byte_val = (u8)double_value;
				break;
			case ShortType:
				op->value.short_val = (s16)double_value;
				break;
			case LongType:
				op->value.long_val = (s32)double_value;
				break;
			case StringType:
				sprintf(tempstr,"%.02f",double_value);
				op->value.string_val = str_copynew(tempstr);
				break;
			case FloatType:
				op->value.float_val = (f32)double_value;
				break;
			case PtrType:
				*((f64*)(op->value.ptr_val)) = (f64)double_value;
				break;
			}
			break;
		}
	case StringType:
		{
			u8* str_value = (u8*)valuestr;
			switch(desttype)
			{
			case IntegerType:
				op->value.int_val = (s32)atoi((char*)str_value);
				break;
			case LongType:
				op->value.long_val = (s32)atol((char*)str_value);
				break;
			case ByteType:
				op->value.byte_val = (u8)atoi((char*)str_value);
				break;
			case ShortType:
				op->value.short_val = (s16)atoi((char*)str_value);
				break;
			case FloatType:
				op->value.float_val = (f32)atof((char*)str_value);
				break;
			case DoubleType:
				op->value.double_val = (f64)atof((char*)str_value);
				break;
			case DateType:
				op->value.int_val = (s32)tVMValue_StringConvertToDate(str_value);
				break;
			case PtrType:
				op->value.ptr_val = (ptr32)op->value.string_val;
				break;
			}
			break;
		}
	case DateType:
		{
			s32 date_value = op->value.int_val;
			switch(desttype)
			{
			case StringType:
				op->value.string_val = (ptr32)tVMValue_DateConvertToString(date_value);
				break;
			case IntegerType:
				op->value.int_val = date_value;
				break;
			case ShortType:
				op->value.short_val = (s16)date_value;
			case PtrType:
				*((s32*)(op->value.ptr_val)) = (s32)date_value;
				break;
			}
			break;
		}
	case PtrType:
		{
			switch(desttype)
			{
			case ByteType:
				op->value.byte_val = (u8)*((u8*)(op->value.ptr_val));
				break;
			case IntegerType:
			case LongType:
				op->value.long_val = (s32)*((s32*)(op->value.ptr_val));
				break;
			case DoubleType:
				op->value.double_val = (f64)*((f64*)(op->value.ptr_val));
				break;
			case StringType:
				op->value.string_val = (ptr32)str_copynew(op->value.ptr_val);
				break;
			case DateType:
				op->value.int_val = (s32)*((s32*)(op->value.ptr_val));
				break;
			}
			break;
		}
	}
	op->type = desttype;
	return 1;
}

//int tVMValue_ConvertType_OLD(tVMValue* op,u8 desttype)
//{
//	char tempstr[MAX_CHAR];
//	char valuestr[MAX_CHAR];
//
//	if(op->type == desttype) return 1;
//
//	if(op->type == StringType && op->value.string_val != NULL)
//	{
//		strcpy(valuestr,op->value.string_val);
//		mem_free(op->value.string_val);	
//	}
//
//	switch(op->type)
//	{
//	case IntegerType:
//	case LongType:
//		{
//			s32 int_value = op->value.int_val;
//			switch(desttype)
//			{
//			case ByteType:
//				op->value.byte_val = (u8)int_value;
//				break;
//			case ShortType:
//				op->value.short_val = (u16)int_value;
//				break;
//			case LongType:
//				op->value.long_val = (s32)int_value;
//				break;
//			case FloatType:
//				op->value.float_val = (f32)int_value;
//				break;
//			case DoubleType:
//				op->value.double_val = (f64)int_value;
//				break;
//			case StringType:
//				sprintf(tempstr,"%d",(int)int_value);
//				op->value.string_val = str_copynew(tempstr);
//				break;
//			case DateType:
//				op->value.int_val = int_value;
//				break;
//			case PtrType:
//			case TypeType:
//				op->value.ptr_val = (ptr32)int_value;
//				break;
//			}
//			break;
//		}
//	case ByteType:
//		{
//			u8 byte_value = op->value.byte_val;
//			switch(desttype)
//			{
//			case IntegerType:
//				op->value.int_val = byte_value;
//				break;
//			case ShortType:
//				op->value.short_val = (u16)byte_value;
//				break;
//			case FloatType:
//				op->value.float_val = (f32)int_value;
//				break;
//			case LongType:
//				op->value.long_val = byte_value;
//				break;
//			case DoubleType:
//				op->value.double_val = (f64)byte_value;
//				break;
//			case StringType:
//				sprintf(tempstr,"%c",byte_value);
//				op->value.string_val = str_copynew(tempstr);
//				break;
//			}
//			break;
//		}
//	case DoubleType:
//		{
//			f64 double_value = op->value.double_val;
//			switch(desttype)
//			{
//			case IntegerType:
//				op->value.int_val = (s32)double_value;
//				break;
//			case ByteType:
//				op->value.byte_val = (u8)double_value;
//				break;
//			case ShortType:
//				op->value.short_val = (u16)double_value;
//				break;
//			case FloatType:
//				op->value.float_val = (f32)double_value;
//				break;
//			case LongType:
//				op->value.long_val = (s32)double_value;
//				break;
//			case StringType:
//				sprintf(tempstr,"%.12lf",double_value);
//				op->value.string_val = str_copynew(tempstr);
//				break;
//			}
//			break;
//		}
//	case StringType:
//		{
//			u8* str_value = (u8*)valuestr;
//			switch(desttype)
//			{
//			case IntegerType:
//				op->value.int_val = (s32)atoi((char*)str_value);
//				break;
//			case LongType:
//				op->value.long_val = (s32)atol((char*)str_value);
//				break;
//			case ByteType:
//				op->value.byte_val = (u8)atoi((char*)str_value);
//				break;
//			case DoubleType:
//				op->value.double_val = (f64)atof((char*)str_value);
//				break;
//			case DateType:
//				op->value.int_val = (s32)tVMValue_StringConvertToDate(str_value);
//				break;
//			}
//			break;
//		}
//	case DateType:
//		{
//			s32 date_value = op->value.int_val;
//			switch(desttype)
//			{
//			case StringType:
//				op->value.string_val = (ptr32)tVMValue_DateConvertToString(date_value);
//				break;
//			case IntegerType:
//				op->value.int_val = date_value;
//				break;
//			}
//			break;
//		}
//	}
//	op->type = desttype;
//	return 1;
//}

/*=========================================================================
* FUNCTION:      tVMValue_StringConvertToDate
* TYPE:          private interface
* OVERVIEW:      convert string value to date value
* INTERFACE:
*   parameters:  
*               str_value u8*: string value
*   returns:   
*               s32 : date value. 0 denote the wrong date format
*=======================================================================*/
s32 tVMValue_StringConvertToDate(u8* str_value)
{
	s32 year,month,day;
	char str_year[8],str_month[4],str_day[4];
	s32 i = 0,j=0;

	if(str_value == NULL)
		return 0;

	j = 0;
	while(str_value[i] != '\0' && str_value[i] != '/' && str_value[i] !='-' && str_value[i] !='\\')
		str_year[j++] = str_value[i++];
	str_year[j] = '\0';
	year = atoi(str_year);
	i++;

	j = 0;
	while(str_value[i] != '\0' && str_value[i] != '/' && str_value[i] !='-' && str_value[i] !='\\')
		str_month[j++] = str_value[i++];
	str_month[j] = '\0';
	month = atoi(str_month);
	i++;

	j = 0;
	while(str_value[i] != '\0')
		str_day[j++] = str_value[i++];
	str_day[j] = '\0';
	day = atoi(str_day);

	return year*10000+month*100+day;
}

/*=========================================================================
* FUNCTION:      tVMValue_DateConvertToString
* TYPE:          private interface
* OVERVIEW:      convert date value to string
* INTERFACE:
*   parameters:  
*               date_value s32: date value
*   returns:   
*               u8* : the string value of the date
*=======================================================================*/
u8* tVMValue_DateConvertToString(s32 date_value)
{
	char tempstr[MAX_CHAR];
	s32 year,month,day;

	year = date_value / 10000;
	month= (date_value % 10000) /100;
	day  = date_value % 100;
	sprintf(tempstr,"%d/%d/%d",(int)year,(int)month,(int)day);

	return (u8*)str_copynew(tempstr);
}
