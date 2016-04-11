#ifndef _TLBASIC_RUNSTACK_H_
#define _TLBASIC_RUNSTACK_H_

#include "tbytecode.h"
#include "tVMValue.h"

#define SIZE_RUNSTACK (8*1024)	// 运行栈暂时设定为8k

typedef struct _tRunstack
{
	u8* buffer;
	u32 size;
	u32 top;
}tRunstack;

tRunstack* tRunstack_create(u32 size);
void       tRunstack_release(tRunstack* stack);

#define tRunstack_getTopPtr(stack) (stack->top)

#define tRunstack_pushU8(stack,value)  { *(u8*)(stack->buffer+stack->top)  = (value); stack->top += sizeof(u8);  }
#define tRunstack_pushU16(stack,value) { memcpy(stack->buffer+stack->top,&value,2); stack->top+=sizeof(u16); }
#define tRunstack_pushU32(stack,value) { memcpy(stack->buffer+stack->top,&value,4); stack->top += sizeof(u32); }
#define tRunstack_pushS16(stack,value) { memcpy(stack->buffer+stack->top,&value,2); stack->top += sizeof(s16); }
#define tRunstack_pushS32(stack,value) { memcpy(stack->buffer+stack->top,&value,4); stack->top += sizeof(s32); }
#define tRunstack_pushF32(stack,value) { memcpy(stack->buffer+stack->top,&value,4); stack->top += sizeof(f32); }
#define tRunstack_pushF64(stack,value) { memcpy(stack->buffer+stack->top,&value,8); stack->top += sizeof(f64); }


#define tRunstack_popU8(stack,value)   { stack->top -= sizeof(u8);  value=*(u8*)(stack->buffer+stack->top);    }
#define tRunstack_popU16(stack,value)  { stack->top -= sizeof(u16); memcpy(&value,stack->buffer+stack->top,2);   }
#define tRunstack_popU32(stack,value)  { stack->top -= sizeof(u32); memcpy(&value,stack->buffer+stack->top,4);   }
#define tRunstack_popS16(stack,value)  { stack->top -= sizeof(s16); memcpy(&value,stack->buffer+stack->top,2);   }
#define tRunstack_popS32(stack,value)  { stack->top -= sizeof(s32); memcpy(&value,stack->buffer+stack->top,4);   }
#define tRunstack_popF32(stack,value)  { stack->top -= sizeof(f32); memcpy(&value,stack->buffer+stack->top,4);   }
#define tRunstack_popF64(stack,value)  { stack->top -= sizeof(f64); memcpy(&value,stack->buffer+stack->top,8);   }



#define tRunstack_getU8(stack,index)   (*(u8*)(stack->buffer+index))

u16 tRunstack_getU16(tRunstack* stack,u32 index);
u32 tRunstack_getU32(tRunstack* stack,u32 index);
s16 tRunstack_getS16(tRunstack* stack,u32 index);
s32 tRunstack_getS32(tRunstack* stack,u32 index); 
f32 tRunstack_getF32(tRunstack* stack,u32 index);  
f64 tRunstack_getF64(tRunstack* stack,u32 index);

#define tRunstack_setU8(stack,index,value)  (*(u8*)(stack->buffer+index) = (value))
#define tRunstack_setU16(stack,index,value) memcpy(stack->buffer+index,&value,2)
#define tRunstack_setU32(stack,index,value) memcpy(stack->buffer+index,&value,4)
#define tRunstack_setS16(stack,index,value) memcpy(stack->buffer+index,&value,2)
#define tRunstack_setS32(stack,index,value) memcpy(stack->buffer+index,&value,4)
#define tRunstack_setF32(stack,index,value) memcpy(stack->buffer+index,&value,4)
#define tRunstack_setF64(stack,index,value) memcpy(stack->buffer+index,&value,8)

/*
#define tRunstack_pushU8(stack,value)  { *(u8*)(stack->buffer+stack->top)  = (value); stack->top += sizeof(u8);  }
#define tRunstack_pushU16(stack,value) { *(u16*)(stack->buffer+stack->top) = (value); stack->top += sizeof(u16); }
#define tRunstack_pushU32(stack,value) { *(u32*)(stack->buffer+stack->top) = (value); stack->top += sizeof(u32); }
#define tRunstack_pushS16(stack,value) { *(s16*)(stack->buffer+stack->top) = (value); stack->top += sizeof(s16); }
#define tRunstack_pushS32(stack,value) { *(s32*)(stack->buffer+stack->top) = (value); stack->top += sizeof(s32); }
#define tRunstack_pushF32(stack,value) { *(f32*)(stack->buffer+stack->top) = (value); stack->top += sizeof(f32); }
#define tRunstack_pushF64(stack,value) { *(f64*)(stack->buffer+stack->top) = (value); stack->top += sizeof(f64); }


#define tRunstack_popU8(stack,value)   { stack->top -= sizeof(u8);  value=*(u8*)(stack->buffer+stack->top);    }
#define tRunstack_popU16(stack,value)  { stack->top -= sizeof(u16); value=*(u16*)(stack->buffer+stack->top);   }
#define tRunstack_popU32(stack,value)  { stack->top -= sizeof(u32); value=*(u32*)(stack->buffer+stack->top);   }
#define tRunstack_popS16(stack,value)  { stack->top -= sizeof(s16); value=*(s16*)(stack->buffer+stack->top);   }
#define tRunstack_popS32(stack,value)  { stack->top -= sizeof(s32); value=*(s32*)(stack->buffer+stack->top);   }
#define tRunstack_popF32(stack,value)  { stack->top -= sizeof(f32); value=*(f32*)(stack->buffer+stack->top);   }
#define tRunstack_popF64(stack,value)  { stack->top -= sizeof(f64); value=*(f64*)(stack->buffer+stack->top);   }



#define tRunstack_getU8(stack,index)   (*(u8*)(stack->buffer+index))
#define tRunstack_getU16(stack,index)  (*(u16*)(stack->buffer+index))
#define tRunstack_getU32(stack,index)  (*(u32*)(stack->buffer+index))
#define tRunstack_getS16(stack,index)  (*(s16*)(stack->buffer+index))
#define tRunstack_getS32(stack,index)  (*(s32*)(stack->buffer+index))
#define tRunstack_getF32(stack,index)  (*(f32*)(stack->buffer+index))
#define tRunstack_getF64(stack,index)  (*(f64*)(stack->buffer+index))


#define tRunstack_setU8(stack,index,value)  (*(u8*)(stack->buffer+index) = (value))
#define tRunstack_setU16(stack,index,value) (*(u16*)(stack->buffer+index)= (value))
#define tRunstack_setU32(stack,index,value) (*(u32*)(stack->buffer+index)= (value))
#define tRunstack_setS16(stack,index,value) (*(s16*)(stack->buffer+index)= (value))
#define tRunstack_setS32(stack,index,value) (*(s32*)(stack->buffer+index)= (value))
#define tRunstack_setF32(stack,index,value) (*(f32*)(stack->buffer+index)= (value))
#define tRunstack_setF64(stack,index,value) (*(f64*)(stack->buffer+index)= (value))
*/
#define tRunstack_freePtr(stack,index) ( free((void*)tRunstack_getU32(stack,index)) )

void tRunstack_pushVMValue(tRunstack* stack,tVMValue* value);
void tRunstack_popVMValue(tRunstack* stack,tVMValue* value);
void tRunstack_loadVMValue(tRunstack* stack,u32 index,tVMValue* value);
void tRunstack_storeVMValue(tRunstack* stack,u32 index,tVMValue* value);
void tRunstack_forceloadVMValue(tRunstack* stack,u32 index,u8 type,tVMValue* value);

#endif
