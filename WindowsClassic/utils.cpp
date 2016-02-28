
#include "utils.h"
#include <stdlib.h>

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

#if (defined(UNICODE)) || (defined(_UNICODE))
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
#endif // UNICODE || _UNICODE