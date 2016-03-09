
#include "utils.h"
#include <stdlib.h>
#include <stdarg.h>

T_UINT32 StrLenA(TSTRING string)
{
	T_UINT32 len = 0;

	while (*string != '\0')
	{
		++string;
		++len;
	}

	return len;
}

T_UINT32 StrPtrArrLenA(TSTRING *ptr_arr)
{
	T_UINT32 len = 0;

	while (*ptr_arr)
	{
		++ptr_arr;
		++len;
	}

	return len;
}

T_INT32 PrintToA(TSTRING dest, const TSTRING format, ...)
{
	va_list args;

	va_start(args, format);
	T_INT32 ret = sprintf(dest, format, args);
	va_end(args);

	return ret;
}

// ------------------------ UNICODE FUNCTIONS ------------------------ //

T_UINT32 StrLenW(WSTRING string)
{
	T_UINT32 len = 0;

	while (*string != '\0')
	{
		++string;
		++len;
	}

	return len;
}

T_UINT32 StrPtrArrLenW(WSTRING *ptr_arr)
{
	T_UINT32 len = 0;

	while (*ptr_arr)
	{
		++ptr_arr;
		++len;
	}

	return len;
}

T_INT32 PrintToW(WSTRING dest, const WSTRING format, ...)
{
	va_list args;

	va_start(args, format);
	T_INT32 ret = _swprintf(dest, format, args);
	va_end(args);

	return ret;
}

WSTRING AllocWStr(TSTRING source)
{
	WSTRING _alloc = NULL;
	T_UINT32 _len;

	_len = StrLenA(source);
	_alloc = (WSTRING)malloc(sizeof(wchar_t) * _len);

	return _alloc;
}