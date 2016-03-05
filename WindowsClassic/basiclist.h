
#ifndef _BASICLIST_H_
#define _BASICLIST_H_

// NOTE(toni): Modified this file for DLL exports
#include "types.h"

#ifdef __cplusplus // Not actually necessary but it's just a nice security measure.
	#define C_EXTERN extern "C" EXPORT
#else
	#define C_EXTERN extern EXPORT
#endif

#ifdef _MSC_VER
	#define C_INLINE static __inline
#else
	#define C_INLINE static inline
#endif

#define LIST_DEFAULT_SIZE 0x00

typedef struct __tagBasicList		BasicList,		*LPBasicList;
typedef struct __tagListEntry		List_Entry,		*LPList_Entry;

C_EXTERN BasicList *List_Create(T_INT32 size);

C_INLINE BasicList *List_CreateDefault()
{
	return List_Create(LIST_DEFAULT_SIZE);
}

C_EXTERN T_INT32 List_GetSize(BasicList *list);

C_EXTERN T_INT32 List_GetCount(BasicList *list);

C_EXTERN T_INT32 List_Contains(BasicList *list, T_PVOID data);

C_EXTERN void List_Set(BasicList *list, T_INT32 index, T_PVOID data);

C_EXTERN T_INT32 List_Add(BasicList *list, T_PVOID data);

C_EXTERN void List_RemoveByIndex(BasicList *list, T_INT32 index);

C_EXTERN void List_Remove(BasicList *list, T_PVOID data);

C_EXTERN void *List_Get(BasicList *list, T_INT32 index);

C_EXTERN void List_Clear(BasicList *list);

C_EXTERN void DeleteList(BasicList *list);

#endif