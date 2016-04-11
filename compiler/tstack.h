#ifndef _TLBASIC_STACK_H_
#define _TLBASIC_STACK_H_

typedef struct _tstack
{
	char* buf;
	long  top;
	long  size;
}tstack;

#define MAX_STACK_SIZE         1*1024*1024
#define NORMAL_STACK_SIZE      640*1024
#define MINI_STACK_SIZE        64*1024
#define SMART_STACK_SIZE       16*1024

#define tstack_getTop(stack)  (stack->top)
#define tstack_getPtr(stack,ptr) (stack->buf + (ptr))

tstack* tstack_create(long bufsize);
void    tstack_release(tstack* stack);
int     tstack_IsEmpty(tstack* stack);

void  tstack_PushInt(tstack* stack,int value);
void  tstack_PushLong(tstack* stack,long value);
void  tstack_PushBuffer(tstack* stack,void* buf,long size);

int   tstack_PopInt(tstack* stack);
long  tstack_PopLong(tstack* stack);
void* tstack_PopBuffer(tstack* stack,long size);

void* tstack_Get(tstack* stack,long TopIndex,long size);

#endif
