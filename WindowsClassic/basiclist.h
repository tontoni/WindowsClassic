
#ifndef _BASICLIST_H_
#define _BASICLIST_H_

#ifdef __cplusplus // Not actually necessary but it's just a nice security measure.
#define C_EXTERN extern "C"
#else
#define C_EXTERN extern
#endif

#ifdef _MSC_VER
#define C_INLINE static __inline
#else
#define C_INLINE static inline
#endif

#include "types.h"

#define LIST_DEFAULT_SIZE 0x00

typedef struct
{
	T_PVOID data;
} List_Entry;

typedef struct
{
	T_INT32 size, count;
	List_Entry *table;
} BasicList;

C_EXTERN BasicList *List_Create(T_INT32 size);

C_INLINE BasicList *List_CreateDefault()
{
	return List_Create(LIST_DEFAULT_SIZE);
}

C_INLINE T_INT32 List_GetSize(BasicList *list)
{
	if (!list)
		return -1;

	return list->size;
}

C_INLINE T_INT32 List_GetCount(BasicList *list)
{
	if (!list)
		return -1;

	return list->count;
}

C_EXTERN T_INT32 List_Contains(BasicList *list, T_PVOID data);

C_EXTERN void List_Set(BasicList *list, T_INT32 index, T_PVOID data);

C_EXTERN T_INT32 List_Add(BasicList *list, T_PVOID data);

C_EXTERN void List_RemoveByIndex(BasicList *list, T_INT32 index);

C_EXTERN void List_Remove(BasicList *list, T_PVOID data);

C_EXTERN void *List_Get(BasicList *list, T_INT32 index);

C_EXTERN void List_Clear(BasicList *list);

C_EXTERN void DeleteList(BasicList *list);

#endif