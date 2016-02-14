
#include "cdrawutils.h"
#include "types.h"

// Color Bitmask: ( Blue | Green | Red )
DWORD CDrawUtils::MakeColorDarker(DWORD color, 
									BYTE by_how_much)
{
	BYTE b = (color >> 16) & 0xFF;
	BYTE g = (color >> 8) & 0xFF;
	BYTE r = (color >> 0) & 0xFF;

	b = max(b - (BYTE)abs(by_how_much), 0);
	g = max(g - (BYTE)abs(by_how_much), 0);
	r = max(r - (BYTE)abs(by_how_much), 0);

	return (DWORD)(b << 16 | g << 8 | r << 0);
}

DWORD CDrawUtils::MakeColorBrighter(DWORD color,
									BYTE by_how_much)
{
	BYTE b = (color >> 16) & 0xFF;
	BYTE g = (color >> 8) & 0xFF;
	BYTE r = (color >> 0) & 0xFF;

	b = min(b + (BYTE)abs(by_how_much), 255);
	g = min(g + (BYTE)abs(by_how_much), 255);
	r = min(r + (BYTE)abs(by_how_much), 255);

	return (DWORD)(b << 16 | g << 8 | r << 0);
}

// This might not be the best function to draw a simple rectangle
// but it should do for now.
void CDrawUtils::FillSolidRectangle(HDC context,
									int x,
									int y,
									int w,
									int h,
									DWORD color)
{
	RECT rect;
	rect.left = x;
	rect.top = y;
	rect.right = x + w;
	rect.bottom = y + h;

	HBRUSH brush = CreateSolidBrush(color);

	FillRect(context, &rect, brush);

	DeleteObject(brush);
}

void CDrawUtils::FillPolygon(HDC context,
							const POINT *vectors,
							int vector_cnt,
							DWORD color)
{
	HBRUSH brush = CreateSolidBrush(color);
	SelectObject(context, brush);

	Polygon(context, vectors, vector_cnt);

	DeleteObject(brush);
}

void CDrawUtils::FillRectangle3DSmall(HDC context,
										int x,
										int y,
										int w,
										int h, 
										DWORD color, 
										bool raised)
{
	DWORD col_0 = 0xFFFFFF;	// Should always be white
	DWORD col_1 = 0x000000; // Shadow - always black
	DWORD col_2 = MakeColorBrighter(color, 32);
	DWORD col_3 = MakeColorDarker(color, 64);

	FillSolidRectangle(context, x    , y    , w    , h    , (raised ? col_1 : col_0));
	FillSolidRectangle(context, x    , y    , w - 1, h - 1, (raised ? col_0 : col_1));
	FillSolidRectangle(context, x + 1, y + 1, w - 2, h - 2, (raised ? col_3 : col_2));
	FillSolidRectangle(context, x + 1, y + 1, w - 3, h - 3, (raised ? col_2 : col_3));
	FillSolidRectangle(context, x + 2, y + 2, w - 4, h - 4, color);
}

void CDrawUtils::FillRectangle3D(HDC context, 
								int x, 
								int y, 
								int w, 
								int h, 
								DWORD color, 
								bool raised)
{
	DWORD col_0 = MakeColorBrighter(color, 32); // 0xE0E0E0
	DWORD col_1 = 0x000000;						// Shadow - should always be black no matter what
	DWORD col_2 = 0xFFFFFF;						// Should always be white
	DWORD col_3 = MakeColorDarker(color, 64);	// 0x808080

	// First layer
	FillSolidRectangle(context, x    , y    , w    , h    , (raised ? col_1 : col_0));
	FillSolidRectangle(context, x    , y    , w - 1, h - 1, (raised ? col_0 : col_1));
	FillSolidRectangle(context, x + 1, y + 1, w - 2, h - 2, (raised ? col_3 : col_2));
	FillSolidRectangle(context, x + 1, y + 1, w - 3, h - 3, (raised ? col_2 : col_3));
	FillSolidRectangle(context, x + 2, y + 2, w - 4, h - 4, color);
}

void CDrawUtils::DrawString(HDC context,
							int x,
							int y,
							int width, 
							int height, 
							char *string,
							DWORD background_color,
							DWORD text_color)
{
	if (background_color != -1)
		SetBkColor(context, background_color);

	if (text_color != -1)
		SetTextColor(context, text_color);

	if ((width > -1) && 
		(height > -1))
	{
		RECT bounds;
		bounds.left = x;
		bounds.top = y;
		bounds.right = (x + width);
		bounds.bottom = (y + height);

		DrawText(context, string, StrLen(string), &bounds, DT_LEFT);
	}
	else
	{
		TextOut(context, x, y, string, StrLen(string));
	}
}