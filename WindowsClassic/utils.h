
#ifndef _UTILS_H_
#define _UTILS_H_

#include "types.h"

#define BITMASK_SWITCH(x) \
    for (T_UINT64 bit = 1; x >= bit; bit *= 2) if (x & bit) switch (bit)

#define HANDLE_WNDMESSAGE(hWnd, message, wParam, lParam, function)	\
case message:														\
{																	\
	function(hWnd, wParam, lParam);									\
} break;

EXPORT extern T_UINT32 StrLenA(TSTRING string);

// Unicode support (currently not really used though)
#if (defined(UNICODE)) || (defined(_UNICODE))
	EXPORT extern T_UINT32 StrLenW(WSTRING string);
#endif // UNICODE || _UNICODE

#endif // _UTILS_H_