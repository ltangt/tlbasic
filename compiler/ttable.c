#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ttable.h"
#include "global.h"
#include "tstring.h"

#define HASH_NO_CASE

ttable_Item* ttable_newItem(ttable_Tab* table,const char *key,const void *item,long itemSize);
int          ttable_getHashIndex(ttable_Tab* table,const char *key);
int          ttable_freeItem(ttable_Tab* table,ttable_Item* TabItem);
//////////////////////////////////////////////////////////////////////////
// create hash table
ttable_Tab* ttable_create(int tabsize)
{
	ttable_Tab *table = (ttable_Tab*)malloc(sizeof(ttable_Tab));
	table->hashItems  = (ttable_Item*)malloc(tabsize*sizeof(ttable_Item)); 
	if(table->hashItems == NULL)
		return NULL;
	// all the head item is reserved empty
	memset(table->hashItems,0,tabsize*sizeof(ttable_Item));
	table->tabsize = tabsize;
	return table;
}

//////////////////////////////////////////////////////////////////////////
// create a hash table item
ttable_Item* ttable_newItem(ttable_Tab* table,const char *key,const void *item,long itemSize)
{
	ttable_Item *newItem = (ttable_Item*)malloc(sizeof(ttable_Item));
	if(newItem == NULL)
		return NULL;
	strcpy(newItem->key,key);
	newItem->item = malloc(itemSize);
	if(newItem->item == NULL)
		return NULL;
	memcpy(newItem->item,item,itemSize);
	newItem->itemSize = itemSize;
	newItem->next = NULL;
	return newItem;
}


//////////////////////////////////////////////////////////////////////////
// Hash function
int ttable_getHashIndex (ttable_Tab* table,const char *key)
{
	unsigned int nHash = 0;
	while(*key)
		nHash = (nHash<<5) + nHash + *key++;
	return nHash % table->tabsize;
}

//////////////////////////////////////////////////////////////////////////
// insert a item
int ttable_insert(ttable_Tab* table,const char *key,const void *item,long itemSize)
{
	int Index;
	ttable_Item *newItem,*TabItem;
#ifdef	HASH_NO_CASE
	char lowcase_key[MAX_CHAR];
	strcpy(lowcase_key,key);
	str_lowercase(lowcase_key);
	key = lowcase_key;
#endif

	newItem = ttable_newItem(table,key,item,itemSize);
	if(newItem == NULL)
		return 0;

	Index= ttable_getHashIndex(table,key);
	TabItem = &table->hashItems[Index];
	while(TabItem->next != NULL)
		TabItem = TabItem->next;
	TabItem->next = newItem;
	return 1;
}

//////////////////////////////////////////////////////////////////////////
// look for a item
void* ttable_lookup(ttable_Tab* table,const char *key)
{
	int Index;
	ttable_Item *TabItem;
#ifdef	HASH_NO_CASE
	char lowcase_key[MAX_CHAR];
	strcpy(lowcase_key,key);
	str_lowercase(lowcase_key);
	key = lowcase_key;
#endif

	Index= ttable_getHashIndex(table,key);
	TabItem = &table->hashItems[Index];
	TabItem = TabItem->next; // all the head item is reserved empty
	while(TabItem != NULL)
	{
		if(strcmp(TabItem->key, key) == 0)
			return TabItem->item;
		TabItem = TabItem->next;
	}
	// if not found the item
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
// delete a item
int ttable_delete(ttable_Tab* table,const char *key)
{
	int Index;
	ttable_Item *TabItem,*LastItem;

	Index= ttable_getHashIndex(table,key);
	TabItem = &table->hashItems[Index];
	LastItem= TabItem;
	TabItem = TabItem->next; // all the head item is reserved empty
	while(TabItem != NULL)
	{
		if(strcmp(TabItem->key, key) == 0)
		{
			LastItem->next = TabItem->next;
			if(TabItem->item != NULL)
				free(TabItem->item);
			free(TabItem);
			return 1;
		}
		LastItem= TabItem;
		TabItem = TabItem->next;
	}
	// if not found the item
	return 0;
}

//////////////////////////////////////////////////////////////////////////
// free all memory of the hash table
int ttable_release(ttable_Tab* table)
{
	int i;
	for(i = 0; i<table->tabsize; i++)
		ttable_freeItem(table,table->hashItems[i].next);
	free(table->hashItems);
	return 1;
}

//////////////////////////////////////////////////////////////////////////
// free a Hash Table Item
int ttable_freeItem(ttable_Tab* table,ttable_Item* TabItem)
{
	if(TabItem == NULL)
		return 1;

	ttable_freeItem(table,TabItem->next);

	if(TabItem->item != NULL)
		free(TabItem->item);
	free(TabItem);
	return 1;
}

