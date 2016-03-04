
#ifndef _CDRAWUTILS_H_
#define _CDRAWUTILS_H_

#pragma once

#include <Windows.h>
#include "types.h"

#define RECT_RAISED		0x0000000FL
#define RECT_OUTLINED	0x000000F0L

typedef struct __tagDRAWCONTEXT
{
	PAINTSTRUCT paintstruct;

	DWORD fill_color,
		  draw_color;
} DRAWCONTEXT, *LPDRAWCONTEXT;

class CDrawUtils
{
	public:
		static DWORD			MakeColorDarker(DWORD color, 
												BYTE by_how_much);

		static DWORD			MakeColorBrighter(DWORD color, 
													BYTE by_how_much);

		static void				FillSolidRectangle(LPDRAWCONTEXT context,
													int x,
													int y,
													int w,
													int h);

		static void				FillPolygon(LPDRAWCONTEXT context,
											const POINT *vectors,
											int vector_cnt);

		static void				FillRectangle3DSmall(LPDRAWCONTEXT context,
														int x,
														int y,
														int w,
														int h, 
														UINT flags);

		static void				FillRectangle3D(LPDRAWCONTEXT context,
												int x, 
												int y, 
												int w, 
												int h, 
												UINT flags);

		static void				DrawString(LPDRAWCONTEXT context,
											TSTRING string,
											int x,
											int y,
											int width = -1, 
											int height = -1, 
											UINT text_format = DT_LEFT);
};

#endif // _CDRAWUTILS_H_