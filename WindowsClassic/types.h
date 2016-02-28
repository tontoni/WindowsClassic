
// Just some basic Type definitions

#ifndef _TYPES_H_
#define _TYPES_H_

#include <tchar.h>

#define SINLINE				static inline

typedef char				T_INT8;
typedef short				T_INT16;
typedef int					T_INT32;
typedef long				T_INT64;

typedef float				T_REAL32;
typedef double				T_REAL64;
typedef long double			T_REAL80;

typedef unsigned char		T_UINT8;
typedef unsigned short		T_UINT16;
typedef unsigned int		T_UINT32;
typedef unsigned long		T_UINT64;

typedef T_UINT8				T_BYTE;
typedef T_UINT16			T_WORD;
typedef T_UINT64			T_DWORD;

typedef void *				T_PVOID;

typedef TCHAR *				TSTRING;

// Unicode support (currently not really used though)
#if (defined(UNICODE)) || (defined(_UNICODE))
	#include <wchar.h>
	
	typedef WCHAR *				WSTRING;
#endif // UNICODE || _UNICODE

#endif // _TYPES_H_