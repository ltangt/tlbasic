#ifndef _TLBASIC_BUFFERREADER_H_
#define _TLBASIC_BUFFERREADER_H_

#include "tbytecode.h"
#include "tVM.h"

/*=========================================================================
* buffer reader is to help reading byte code
*=======================================================================*/
typedef struct _tbufferreader
{
	const u8* buf;
	u32       size;
	u32       ptr;
}tbufferreader;

tbufferreader*   tbufferreader_newFromBuf(const u8* buf,u32 size);
void             tbufferreader_release(tbufferreader* br);
u8               tbufferreader_readU8(tbufferreader* br);
u16              tbufferreader_readU16(tbufferreader* br);
u32              tbufferreader_readU32(tbufferreader* br);
s16              tbufferreader_readS16(tbufferreader* br);
s32              tbufferreader_readS32(tbufferreader* br);
f64              tbufferreader_readF64(tbufferreader* br);
u8*              tbufferreader_readString(tbufferreader* br);
u8*              tbufferreader_read(tbufferreader* br,u32 length);
int              tbufferreader_readVMValue(tbufferreader* br,tVMValue* value);
#endif
