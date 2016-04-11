#ifndef _TLBASIC_MEM_H_
#define _TLBASIC_MEM_H_


void  tmem_init();
int   tmem_exit();
void  tmem_copy(void* dest,void* src,long size);
void* tmem_copynew(void* srcmem,long size);
void* tmem_alloc(long size);
void  tmem_free(void* p);

#endif
