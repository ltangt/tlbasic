#include "tRunstack.h"

#include "tbytecode.h"
#include "global.h"
#include "tmem.h"
#include "tstring.h"


/*=========================================================================
* FUNCTION:      tRunstack_create
* TYPE:          public interface
* OVERVIEW:      create a run stack for VM
* INTERFACE:
*   parameters:  
*               size s32: size of the run stack
*   returns:   
*               tRunstack*: the created run stack
*=======================================================================*/
tRunstack* tRunstack_create(u32 size)
{
	tRunstack* stack = (tRunstack*)mem_alloc(sizeof(tRunstack));
	stack->buffer = (u8*)mem_alloc(size);
	stack->size   = size;
	stack->top    = 0;

	return stack;
}

/*=========================================================================
* FUNCTION:      tRunstack_release
* TYPE:          public interface
* OVERVIEW:      release a run stack
* INTERFACE:
*   parameters:  
*               stack tRunstack*: the run stack
*   returns:   
*=======================================================================*/
void tRunstack_release(tRunstack* stack)
{
	mem_free(stack->buffer);
	mem_free(stack);
}

/*=========================================================================
* FUNCTION:      tRunstack_pushVMValue
* TYPE:          public interface
* OVERVIEW:      push VM Value into run stack
* INTERFACE:
*   parameters:  
*               stack tRunstack*: the run stack
*               value tVMValue*: the VM value
*   returns:   
*=======================================================================*/
void tRunstack_pushVMValue(tRunstack* stack,tVMValue* value)
{
	switch(value->type)
	{
	case NullType:
		tRunstack_pushS32(stack,value->value.int_val);
		break;
	case IntegerType:
	case DateType:
		tRunstack_pushS32(stack,value->value.int_val);
		break;
	case StringType:
		tRunstack_pushU32(stack,(u32)value->value.string_val);
		break;
	case ByteType:
		tRunstack_pushU8(stack,(u8)value->value.byte_val);
		break;
	case ShortType:
		tRunstack_pushS16(stack,(s16)value->value.short_val);
		break;
	case FloatType:
		tRunstack_pushF32(stack,(f32)value->value.float_val);
		break;
	case LongType:
		tRunstack_pushS32(stack,value->value.long_val);
		break;
	case DoubleType:
		tRunstack_pushF64(stack,value->value.double_val);
		break;
	case PtrType:
	case TypeType:
		tRunstack_pushU32(stack,(u32)value->value.ptr_val);
		break;
	}
	tRunstack_pushU8(stack,value->type);
}

/*=========================================================================
* FUNCTION:      tRunstack_popVMValue
* TYPE:          public interface
* OVERVIEW:      pop VM Value from run stack top
* INTERFACE:
*   parameters:  
*               stack tRunstack*: the run stack
*               value tVMValue*: the VM value
*   returns:   
*=======================================================================*/
void tRunstack_popVMValue(tRunstack* stack,tVMValue* value)
{
	u32 addr;
	tRunstack_popU8(stack,value->type);
	switch(value->type)
	{
	case NullType:
		tRunstack_popS32(stack,value->value.int_val);
		break;
	case IntegerType:
	case DateType:
		tRunstack_popS32(stack,value->value.int_val);
		break;
	case StringType:
		tRunstack_popU32(stack,addr);
		if((void*)addr != NULL)
		{ 
			value->value.string_val = str_copynew((char*)addr);
			mem_free((void*)addr);
		}
		else
			value->value.string_val = NULL;
		break;
	case ByteType:
		tRunstack_popU8(stack,value->value.byte_val);
		break;
	case ShortType:
		tRunstack_popS16(stack,value->value.short_val);
		break;
	case FloatType:
		tRunstack_popF32(stack,value->value.float_val);
		break;
	case LongType:
		tRunstack_popS32(stack,value->value.long_val);
		break;
	case DoubleType:
		tRunstack_popF64(stack,value->value.double_val);
		break;
	case PtrType:
	case TypeType:
		tRunstack_popU32(stack,addr);
		value->value.ptr_val = (ptr32)addr;
		break;
	}
}

/*=========================================================================
* FUNCTION:      tRunstack_loadVMValue
* TYPE:          public interface
* OVERVIEW:      load VM Value from run stack
* INTERFACE:
*   parameters:  
*               stack tRunstack*: the run stack
*               index u32: the address from the run stack bottom
*               value tVMValue*: the VM value
*   returns:   
*=======================================================================*/
void tRunstack_loadVMValue(tRunstack* stack,u32 index,tVMValue* value)
{
	u32 addr;
	value->type = tRunstack_getU8(stack,index);
	index -= sizeofData(value->type); 
	switch(value->type)
	{
	case NullType:
		value->value.int_val = tRunstack_getS32(stack,index);
		break;
	case IntegerType:
	case DateType:
		value->value.int_val = tRunstack_getS32(stack,index);
		break;
	case StringType:
		value->value.string_val = (ptr32)tRunstack_getU32(stack,index);
		break;
	case ByteType:
		value->value.byte_val = tRunstack_getU8(stack,index);
		break;
	case ShortType:
		value->value.short_val = tRunstack_getS16(stack,index);
		break;
	case FloatType:
		value->value.float_val = tRunstack_getF32(stack,index);
		break;
	case LongType:
		value->value.long_val = tRunstack_getS32(stack,index);
		break;
	case DoubleType:
		value->value.double_val = tRunstack_getF64(stack,index);
		break;
	case PtrType:
	case TypeType:
		addr = tRunstack_getU32(stack,index);
		value->value.ptr_val = (ptr32)addr;
		break;
	}
}

/*=========================================================================
* FUNCTION:      tRunstack_forceloadVMValue
* TYPE:          public interface
* OVERVIEW:      load VM Value from run stack and force change to a data type
* INTERFACE:
*   parameters:  
*               stack tRunstack*: the run stack
*                      index u32: the address from the run stack bottom
*                        u8 type: the value type that force to change after loading
*                value tVMValue*: the VM value
*   returns:   
*=======================================================================*/
void tRunstack_forceloadVMValue(tRunstack* stack,u32 index,u8 type,tVMValue* value)
{
	u32 addr;
	value->type = type;
	index -= sizeofData(type); 
	switch(type)
	{
	case NullType:
		value->value.int_val = tRunstack_getS32(stack,index);
		break;
	case IntegerType:
	case DateType:
		value->value.int_val = tRunstack_getS32(stack,index);
		break;
	case StringType:
		addr = tRunstack_getU32(stack,index);
		value->value.string_val = str_copynew((char*)addr);
		break;
	case ByteType:
		value->value.byte_val = tRunstack_getU8(stack,index);
		break;
	case ShortType:
		value->value.short_val = tRunstack_getS16(stack,index);
		break;
	case FloatType:
		value->value.float_val = tRunstack_getF32(stack,index);
		break;
	case LongType:
		value->value.long_val = tRunstack_getS32(stack,index);
		break;
	case DoubleType:
		value->value.double_val = tRunstack_getF64(stack,index);
		break;
	case PtrType:
	case TypeType:
		addr = tRunstack_getU32(stack,index);
		value->value.ptr_val = (ptr32)addr;
		break;
	}
}

/*=========================================================================
* FUNCTION:      tRunstack_storeVMValue
* TYPE:          public interface
* OVERVIEW:      store VM Value into run stack
* INTERFACE:
*   parameters:  
*               stack tRunstack*: the run stack
*               index u32: the address from the run stack bottom
*               value tVMValue*: the VM value
*   returns:   
*=======================================================================*/
void tRunstack_storeVMValue(tRunstack* stack,u32 index,tVMValue* value)
{
	u32 addr;
	u8  storedtype = tRunstack_getU8(stack,index); /* get the type of dest store value */
	/* if type of value is not compatible, then convert the value type first */
	if(storedtype != value->type)
	{
		tVMValue_ConvertType(value,storedtype);
	}
	/* store value */
	index -= sizeofData(value->type);
	switch(value->type)
	{
	case NullType:
		tRunstack_setS32(stack,index,value->value.int_val);
		break;
	case IntegerType:
	case DateType:
		tRunstack_setS32(stack,index,value->value.int_val);
		break;
	case StringType:
		if((u8*)tRunstack_getU32(stack,index) != NULL)
			tRunstack_freePtr(stack,index);
		addr = (u32)value->value.string_val;
		tRunstack_setU32(stack,index,addr);
		break;
	case ByteType:
		tRunstack_setU8(stack,index,value->value.byte_val);
		break;
	case ShortType:
		tRunstack_setS16(stack,index,value->value.short_val);
		break;
	case FloatType:
		tRunstack_setF32(stack,index,value->value.float_val);
		break;
	case LongType:
		tRunstack_setS32(stack,index,value->value.long_val);
		break;
	case DoubleType:
		tRunstack_setF64(stack,index,value->value.double_val);
		break;
	case PtrType:
	case TypeType:
		addr = (u32)value->value.ptr_val;
		tRunstack_setU32(stack,index,addr);
		break;
	}
}

u16 tRunstack_getU16(tRunstack* stack,u32 index)
{
	u16 ret;
	memcpy(&ret,stack->buffer+index,2);
	return ret;
}
u32 tRunstack_getU32(tRunstack* stack,u32 index)
{
	u32 ret;
	memcpy(&ret,stack->buffer+index,4);
	return ret;
}
s16 tRunstack_getS16(tRunstack* stack,u32 index)
{
	s16 ret;
	memcpy(&ret,stack->buffer+index,2);
	return ret;
}
s32 tRunstack_getS32(tRunstack* stack,u32 index)  
{
	s32 ret;
	memcpy(&ret,stack->buffer+index,4);
	return ret;
}
f32 tRunstack_getF32(tRunstack* stack,u32 index)  
{
	f32 ret;
	memcpy(&ret,stack->buffer+index,4);
	return ret;
}
f64 tRunstack_getF64(tRunstack* stack,u32 index)  
{
	f64 ret;
	memcpy(&ret,stack->buffer+index,8);
	return ret;
}

