#include "tstack.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

tstack* tstack_create(long bufsize)
{
	tstack* newstack = (tstack*)malloc(sizeof(tstack));
	newstack->buf = (char*)malloc(bufsize);
	newstack->top = 0;

	return newstack;
}

void tstack_release(tstack* stack)
{
	free(stack->buf);
	free(stack); 
}

int tstack_IsEmpty(tstack* stack)
{
	return stack->top == 0? 1: 0;
}


void tstack_PushInt(tstack* stack,int value)
{
	*(int*)(stack->buf+stack->top) = value;
	stack->top += sizeof(value);
}

void tstack_PushLong(tstack* stack,long value)
{
	*(long*)(stack->buf+stack->top) = value;
	stack->top += sizeof(value);
}

void tstack_PushBuffer(tstack* stack,void* buf,long size)
{
	memcpy(stack->buf+stack->top,buf,size);
	stack->top += size;
}

int tstack_PopInt(tstack* stack)
{
	stack->top -= sizeof(int);
	return *(int*)(stack->buf+stack->top);
}

long tstack_PopLong(tstack* stack)
{
	stack->top -= sizeof(long);
	return *(long*)(stack->buf+stack->top);
}

void* tstack_PopBuffer(tstack* stack,long size)
{
	void* newptr = malloc(size);
	stack->top -= size;
	memcpy(newptr,stack->buf+stack->top,size);
	return newptr;
}

void* tstack_Get(tstack* stack,long TopIndex,long size)
{
	if(stack->top - size*(TopIndex+1) < 0 )
		return NULL;

	return stack->buf + stack->top - size*(TopIndex+1);
}
