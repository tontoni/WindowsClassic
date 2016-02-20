
#ifndef _CDRAWUTILS_H_
#define _CDRAWUTILS_H_

#include <Windows.h>
#include "types.h"

typedef struct
{
	PAINTSTRUCT paintstruct;

	DWORD fill_color,
		  draw_color;
} DRAWCONTEXT;

class CDrawUtils
{
	public:
		static DWORD			MakeColorDarker(DWORD color, 
												BYTE by_how_much);

		static DWORD			MakeColorBrighter(DWORD color, 
													BYTE by_how_much);

		static void				FillSolidRectangle(DRAWCONTEXT *context,
													int x,
													int y,
													int w,
													int h);

		static void				FillPolygon(DRAWCONTEXT *context,
											const POINT *vectors,
											int vector_cnt);

		static void				FillRectangle3DSmall(DRAWCONTEXT *context,
														int x,
														int y,
														int w,
														int h, 
														bool raised);

		static void				FillRectangle3D(DRAWCONTEXT *context,
												int x, 
												int y, 
												int w, 
												int h, 
												bool raised);

		static void				DrawString(DRAWCONTEXT *context,
											TSTRING string,
											int x,
											int y,
											int width = -1, 
											int height = -1, 
											UINT text_format = DT_LEFT);
};

#endif // _CDRAWUTILS_H_