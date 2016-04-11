#ifndef _TLBASIC_MEM_H_
#define _TLBASIC_MEM_H_

#include "tbytecode.h"

void  mem_init();
int   mem_exit();
u8*   mem_copy(u8* srcmem,u32 size);
void* mem_alloc(long size);
void  mem_free(void* p);

#endif
