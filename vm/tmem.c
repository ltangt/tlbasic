#include <stdio.h>
#include <stdlib.h>
#include "global.h"
#include "tbytecode.h"
#include "tmem.h"

#define MEM_LIST_SIZE 1024*1024
void* memalloclist[MEM_LIST_SIZE];

u8* mem_copy(u8* srcmem,u32 size)
{
	u8* newmem = (u8*)mem_alloc(size);

	if(newmem == NULL)
		return NULL;
	memcpy(newmem,srcmem,size);
	return newmem;
}

void* mem_alloc(long size)
{
	int i;
	void* p = malloc(size);
	for(i=0;i<MEM_LIST_SIZE;i++)
	{
		if(memalloclist[i] == NULL)
		{
			memalloclist[i] = p;
			return p;
		}
	}
	printf("\nMemory leak in allocating !\n");
	return NULL;
}

void mem_free(void* p)
{
	int i;
	for(i=0;i<MEM_LIST_SIZE;i++)
	{
		if(memalloclist[i] == p)
		{
			free(p);
			memalloclist[i] = NULL;
			return;
		}
	}
	printf("\nMemory leak int freeing!\n");
}

void mem_init()
{
	memset(memalloclist,0,sizeof(memalloclist));
}

int mem_exit()
{
	int i;
	for(i=0;i<MEM_LIST_SIZE;i++)
	{
		if(memalloclist[i] != NULL)
		{
			printf("\nMemory leak int exiting! %d\n",i);
 			return 1;
		}
	}
	return 0;
}
