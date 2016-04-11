#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tbuffereader.h"
#include "global.h"
#include "tmem.h"
#include "tstring.h"


/*=========================================================================
* FUNCTION:      tbufferreader_newFromBuf
* TYPE:          public interface
* OVERVIEW:      create a tbufferreader data structure
* INTERFACE:
*   parameters:  buf const u8*: source buffer
*                size u32: source buffer size
*   returns:     the created new tbufferreader data structure
*=======================================================================*/
tbufferreader* tbufferreader_newFromBuf(const u8* buf,u32 size)
{
	tbufferreader *br = (tbufferreader*)mem_alloc(sizeof(tbufferreader));
	br->buf = buf;
	br->size= size;
	br->ptr = 0;
	return br;
}

/*=========================================================================
* FUNCTION:      tbufferreader_release
* TYPE:          public interface
* OVERVIEW:      release a tbufferreader
* INTERFACE:
*   parameters:  br tbufferreader*: the tbufferreader to release
*   returns:    
*=======================================================================*/
void tbufferreader_release(tbufferreader* br)
{
	mem_free(br);
}

/*=========================================================================
* FUNCTION:      tbufferreader_readU8
* TYPE:          public interface
* OVERVIEW:      read a u8 value from the tbufferreader
* INTERFACE:
*   parameters:  
*                br tbufferreader*: the tbufferreader to release
*   returns:     u8 value
*=======================================================================*/
u8 tbufferreader_readU8(tbufferreader* br)
{
	u8 value = *(u8*)(br->buf+br->ptr);
	br->ptr += sizeof(u8);
	return value;
}

/*=========================================================================
* FUNCTION:      tbufferreader_readU16
* TYPE:          public interface
* OVERVIEW:      read a u16 value from the tbufferreader
* INTERFACE:
*   parameters:  
*                br tbufferreader*: the tbufferreader to release
*   returns:     u16 value
*=======================================================================*/
u16 tbufferreader_readU16(tbufferreader* br)
{
	//u16 value = *(u16*)(br->buf+br->ptr);
	u16 value;
	memcpy(&value,(br->buf+br->ptr),sizeof(u16));
	br->ptr += sizeof(u16);
	return value;
}

/*=========================================================================
* FUNCTION:      tbufferreader_readU32
* TYPE:          public interface
* OVERVIEW:      read a u32 value from the tbufferreader
* INTERFACE:
*   parameters:  
*                br tbufferreader*: the tbufferreader to release
*   returns:     u32 value
*=======================================================================*/
u32 tbufferreader_readU32(tbufferreader* br)
{
	//u32 value = *(u32*)(br->buf+br->ptr);
	u32 value;
	memcpy(&value,(br->buf+br->ptr),sizeof(u32));
	br->ptr += sizeof(u32);
	return value;
}

/*=========================================================================
* FUNCTION:      tbufferreader_readS16
* TYPE:          public interface
* OVERVIEW:      read a S16 value from the tbufferreader
* INTERFACE:
*   parameters:  
*                br tbufferreader*: the tbufferreader to release
*   returns:     S16 value
*=======================================================================*/
s16 tbufferreader_readS16(tbufferreader* br)
{
	//s16 value = *(s16*)(br->buf+br->ptr);
	s16 value;
	memcpy(&value,(br->buf+br->ptr),sizeof(s16));	
	br->ptr += sizeof(s16);
	return value;
}

/*=========================================================================
* FUNCTION:      tbufferreader_readS32
* TYPE:          public interface
* OVERVIEW:      read a s32 value from the tbufferreader
* INTERFACE:
*   parameters:  
*                br tbufferreader*: the tbufferreader to release
*   returns:     s32 value
*=======================================================================*/
s32 tbufferreader_readS32(tbufferreader* br)
{
	//s32 value = *(s32*)(br->buf+br->ptr);
	s32 value;
	memcpy(&value,(br->buf+br->ptr),sizeof(s32));
	br->ptr += sizeof(s32);
	return value;
}

/*=========================================================================
* FUNCTION:      tbufferreader_readF64
* TYPE:          public interface
* OVERVIEW:      read a f64 value from the tbufferreader
* INTERFACE:
*   parameters:  
*                br tbufferreader*: the tbufferreader to release
*   returns:     f64 value
*=======================================================================*/
f64 tbufferreader_readF64(tbufferreader* br)
{
	//f64 value = *(f64*)(br->buf+br->ptr);
	f64 value;
	memcpy(&value,(br->buf+br->ptr),sizeof(f64));
	br->ptr += sizeof(f64);
	return value;
}

/*=========================================================================
* FUNCTION:      tbufferreader_readString
* TYPE:          public interface
* OVERVIEW:      read a string value from the tbufferreader
* INTERFACE:
*   parameters:  
*                br tbufferreader*: the tbufferreader to release
*   returns:     string value
*=======================================================================*/
u8* tbufferreader_readString(tbufferreader* br)
{
	u16 len = tbufferreader_readU16(br);
	u8* str = mem_alloc(len+1);
	memcpy(str,br->buf+br->ptr,len);
	str[len] = '\0';
	br->ptr += len;

	return str;
}
 
/*=========================================================================
* FUNCTION:      tbufferreader_read
* TYPE:          public interface
* OVERVIEW:      read a buffer from the tbufferreader
* INTERFACE:
*   parameters:  
*                br tbufferreader*: the tbufferreader to release
*                length u32: the length of the read buffer
*   returns:     read buffer
*=======================================================================*/
u8* tbufferreader_read(tbufferreader* br,u32 length)
{
	u8* buf = (u8*)mem_alloc(length);
	memcpy(buf,br->buf+br->ptr,length);
	br->ptr += length;

	return buf;
}

/*=========================================================================
* FUNCTION:      tbufferreader_readVMValue
* TYPE:          public interface
* OVERVIEW:      read a VM value from the tbufferreader
* INTERFACE:
*   parameters:  
*                br tbufferreader*: the tbufferreader to release
*                value tVMValue*: the output VM value
*   returns:     0: wrong
*                1: success
*=======================================================================*/
int tbufferreader_readVMValue(tbufferreader* br,tVMValue* value)
{
	u8 type = tbufferreader_readU8(br);
	value->type = type;
	switch(type)
	{
	case IntegerType:
		value->value.int_val = tbufferreader_readS32(br);
		break;
	case ByteType:
		value->value.byte_val = tbufferreader_readU8(br);
		break;
	case LongType:
		value->value.long_val = tbufferreader_readS32(br);
		break;
	case DoubleType:
		value->value.double_val = tbufferreader_readF64(br);
		break;
	case StringType:
		value->value.string_val = (ptr32)tbufferreader_readString((tbufferreader*)br);
		break;
	default:
		error_out("buffer read Unknown data type!\n");
		return 0;
	}
	return 1;
}



