#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tmem.h"

void tmem_init()
{
}

int tmem_exit()
{
	return 0;
}

void tmem_copy(void* dest,void* src,long size)
{
	memcpy(dest,src,size);
}

void* tmem_copynew(void* srcmem,long size)
{
	void* newmem = (void*)tmem_alloc(size);

	if(newmem == NULL)
		return NULL;
	memcpy(newmem,srcmem,size);
	return newmem;
}

void* tmem_alloc(long size)
{
	return malloc(size);
}

void  tmem_free(void* p)
{
	free(p);
}
