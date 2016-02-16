
#include "basiclist.h"

#include <stdlib.h>
#include <stdbool.h>

#define TABLE_STARTSIZE 1021

static bool isPrime(T_UINT64 val)
{
	T_INT32 i, p, exp, a;

	for (i = 9; i--;)
	{
		a = (rand() % (val - 4)) + 2;
		p = 1;
		exp = val - 1;

		while (exp)
		{
			if (exp & 1)
				p = (p * a) % val;

			a = (a * a) % val;

			exp >>= 1;
		}

		if (p != 1)
			return 0;
	}

	return 1;
}

static int findPrimeGreaterThan(T_INT32 val)
{
	if (val & 1)
		val += 2;
	else
		val++;

	while (!isPrime(val))
		val += 2;

	return val;
}

static void __Rehash(BasicList *list)
{
	List_Entry *new_table = (List_Entry *)calloc(sizeof(List_Entry), list->size);
	T_INT32 new_count     = 0;

	for (T_INT32 i = 0; i < list->size; ++i)
	{
		if (list->table[i].data)
		{
			new_table[i] = list->table[i];
			++new_count;
		}
	}

	free(list->table);
	list->table = new_table;
	list->count = new_count;
}

BasicList *List_Create(T_INT32 size)
{
	BasicList *list = (BasicList *)malloc(sizeof(BasicList));

	if (size == LIST_DEFAULT_SIZE)
		size = TABLE_STARTSIZE;
	else
		size = findPrimeGreaterThan(size - 2);

	list->table = (List_Entry *)calloc(sizeof(List_Entry), size);
	list->size = size;
	list->count = 0;
	
	return list;
}

int List_Contains(BasicList *list, T_PVOID data)
{
	for (T_INT32 i = 0; i < list->size; ++i)
	{
		List_Entry entry = list->table[i];

		if (entry.data == data)
		{
			return i;
		}
	}

	return 0;
}

void List_Set(BasicList *list, T_INT32 index, T_PVOID data)
{
	if ((index >= list->size) || 
		(index < 0) || 
		(!list->table[index].data)) // If data at position in table is empty (0x00000000)
	{
		return;
	}

	list->table[index].data = (T_PVOID)data;
	__Rehash(list);
}

T_INT32 List_Add(BasicList *list, T_PVOID data)
{
	if ((list->count >= list->size) || 
		(!data))
	{
		return -1;
	}

	T_INT32 index;
	list->table[index = list->count].data = (T_PVOID)data;
	__Rehash(list);

	return index;
}

void List_RemoveByIndex(BasicList *list, T_INT32 index)
{
	List_Set(list, index, NULL);
}

void List_Remove(BasicList *list, T_PVOID data)
{
	T_INT32 index;

	if (!(index = List_Contains(list, data)))
	{
		return;
	}

	List_RemoveByIndex(list, index);
}

void *List_Get(BasicList *list, T_INT32 index)
{
	if ((index >= list->size) || 
		(index < 0))
	{
		return NULL;
	}

	return list->table[index].data;
}

void List_Clear(BasicList *list)
{
	if (list->table)
		free(list->table);
	
	list->table = (List_Entry *)calloc(sizeof(List_Entry), list->size);
	list->count = 0;
}

void DeleteList(BasicList *list)
{
	if (list->table)
		free(list->table);
	
	free(list);
}