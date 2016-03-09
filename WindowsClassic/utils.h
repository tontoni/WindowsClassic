
#ifndef _UTILS_H_
#define _UTILS_H_

#include "types.h"
#include <stdio.h>

#define BITMASK_SWITCH(x) \
    for (T_UINT64 bit = 1; x >= bit; bit *= 2) if (x & bit) switch (bit)

#define HANDLE_WNDMESSAGE(hWnd, message, wParam, lParam, function)	\
case message:														\
{																	\
	function(hWnd, wParam, lParam);									\
} break;

extern EXPORT T_UINT32 StrLenA(TSTRING string);
extern EXPORT T_UINT32 StrPtrArrLenA(TSTRING *ptr_arr);
extern EXPORT T_INT32 PrintToA(TSTRING dest, const TSTRING format, ...);

// ------------------------ UNICODE FUNCTIONS ------------------------ //
extern EXPORT T_UINT32 StrLenW(WSTRING string);
extern EXPORT T_UINT32 StrPtrArrLenW(WSTRING *ptr_arr);
extern EXPORT T_INT32 PrintToW(WSTRING dest, const WSTRING format, ...);

extern EXPORT WSTRING AllocWStr(TSTRING source);

// Unicode support (currently not really used though)
#if (defined(UNICODE)) || (defined(_UNICODE))
	#define StrLen(string)					StrLenW(string)
	#define StrPtrArrLen(ptr_arr)			StrPtrArrLenW(ptr_arr)
	#define StrDupl(str)					_wcsdup(str)
	#define StrToken(str, delim)			wcstok(str, delim)
	
	#define PrintTo(dest, format, ...)		PrintToW(dest, format, ##__VA_ARGS__)
#else
	#define StrLen(string)					StrLenA(string)
	#define StrPtrArrLen(ptr_arr)			StrPtrArrLenA(ptr_arr)
	#define StrDupl(str)					_strdup(str)
	#define StrToken(str, delim)			strtok(str, delim)
	
	#define PrintTo(dest, format, ...)		PrintToA(dest, format, ##__VA_ARGS__)
#endif // UNICODE || _UNICODE

#endif // _UTILS_H_