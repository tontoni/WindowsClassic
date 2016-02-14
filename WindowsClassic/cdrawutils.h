
#ifndef _CDRAWUTILS_H_
#define _CDRAWUTILS_H_

#include <Windows.h>

class CDrawUtils
{
	public:
		static DWORD			MakeColorDarker(DWORD color, 
												BYTE by_how_much);

		static DWORD			MakeColorBrighter(DWORD color, 
													BYTE by_how_much);

		static void				FillSolidRectangle(HDC context,
													int x,
													int y,
													int w,
													int h,
													DWORD color);

		static void				FillPolygon(HDC context,
											const POINT *vectors,
											int vector_cnt,
											DWORD color);

		static void				FillRectangle3DSmall(HDC context,
														int x,
														int y,
														int w,
														int h, 
														DWORD color, 
														bool raised);

		static void				FillRectangle3D(HDC context, 
												int x, 
												int y, 
												int w, 
												int h, 
												DWORD color, 
												bool raised);

		static void				DrawString(HDC context,
											int x,
											int y,
											int width, 
											int height, 
											char *string,
											DWORD background_color = -1,
											DWORD text_color = -1);
};

#endif // _CDRAWUTILS_H_