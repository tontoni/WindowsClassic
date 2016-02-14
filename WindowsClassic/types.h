
// Just some basic Type definitions

#ifndef _TYPES_H_
#define _TYPES_H_

#include <tchar.h>

typedef TCHAR	*TSTRING;

static inline UINT StrLen(TSTRING string)
{
	UINT len = 0;

	while (*string != '\0')
	{
		++string;
		++len;
	}

	return len;
}

#endif // _TYPES_H_