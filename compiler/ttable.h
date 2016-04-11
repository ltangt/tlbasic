#ifndef _TLBASIC_TABLE_H_
#define _TLBASIC_TABLE_H_
#include "global.h"

typedef struct _ttable_Item
{
	char  key[MAX_CHAR];
	void* item;
	long  itemSize;
	struct _ttable_Item *next;
}ttable_Item;

typedef struct _ttable_Tab
{
	int tabsize; // num of hash item( or head item)
	ttable_Item *hashItems;
}ttable_Tab;

ttable_Tab* ttable_create(int tabsize); // create hash table
int         ttable_insert(ttable_Tab* table,const char *key,const void *item,long itemSize); // insert a item
void*       ttable_lookup(ttable_Tab* table,const char *key); // look for a item
int         ttable_delete(ttable_Tab* table,const char *key); // delete a item
int         ttable_release(ttable_Tab* table); // free all memory of the hash table


#endif	  // _TLBASIC_TABLE_H_
