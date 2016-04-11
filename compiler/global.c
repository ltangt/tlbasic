#include <string.h>
#include "global.h"
FILE* g_listfp;
FILE* g_codefp;

int sizeofData(DataType type)
{
	switch(type)
	{
	case datatypeType:
		return sizeof(u8);
	case accessflagType:
		return sizeof(u8);
	case IndexType:
		return sizeof(s32);
	case IntegerType:
	case DateType:
		return sizeof(s32);
	case StringType:
		return sizeof(u8*); 
	case TypeType:
		return sizeof(ptr32);
	case PtrType:
		return sizeof(ptr32);
	case ByteType:
		return sizeof(u8);
	case ShortType:
		return sizeof(s16);
		break;
	case LongType:
		return sizeof(s32);
	case DoubleType:
		return sizeof(f64);
	case FloatType:
		return sizeof(f32);
		break;
	}
	return -1;
}


