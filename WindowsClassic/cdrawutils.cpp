
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
void CDrawUtils::FillSolidRectangle(DRAWCONTEXT *context,
									int x,
									int y,
									int w,
									int h)
{
	RECT rect;
	rect.left = x;
	rect.top = y;
	rect.right = x + w;
	rect.bottom = y + h;
	
	HBRUSH brush = CreateSolidBrush(context->fill_color);
	FillRect(context->hdc, &rect, brush);
	DeleteObject(brush);
}

void CDrawUtils::FillPolygon(DRAWCONTEXT *context,
							const POINT *vectors,
							int vector_cnt)
{
	HBRUSH brush = CreateSolidBrush(context->fill_color);
	SelectObject(context->hdc, brush);

	Polygon(context->hdc, vectors, vector_cnt);

	DeleteObject(brush);
}

void CDrawUtils::FillRectangle3DSmall(DRAWCONTEXT *context,
										int x,
										int y,
										int w,
										int h, 
										bool raised)
{
	DWORD col_0 = 0xFFFFFF;	// Should always be white
	DWORD col_1 = 0x000000; // Shadow - always black
	DWORD col_2 = MakeColorBrighter(context->fill_color, 32);
	DWORD col_3 = MakeColorDarker(context->fill_color, 64);

	// Save the previous background color to restore it later
	DWORD prev_bgr_col = context->fill_color;

	context->fill_color = (raised ? col_1 : col_0);
	FillSolidRectangle(context, x    , y    , w    , h    );

	context->fill_color = (raised ? col_0 : col_1);
	FillSolidRectangle(context, x    , y    , w - 1, h - 1);

	context->fill_color = (raised ? col_3 : col_2);
	FillSolidRectangle(context, x + 1, y + 1, w - 2, h - 2);

	context->fill_color = (raised ? col_2 : col_3);
	FillSolidRectangle(context, x + 1, y + 1, w - 3, h - 3);

	context->fill_color = prev_bgr_col;
	FillSolidRectangle(context, x + 2, y + 2, w - 4, h - 4);
}

void CDrawUtils::FillRectangle3D(DRAWCONTEXT *context,
								int x, 
								int y, 
								int w, 
								int h, 
								bool raised)
{
	DWORD col_0 = MakeColorBrighter(context->fill_color, 32); // 0xE0E0E0
	DWORD col_1 = 0x000000;											// Shadow - should always be black no matter what
	DWORD col_2 = 0xFFFFFF;											// Should always be white
	DWORD col_3 = MakeColorDarker(context->fill_color, 64);	// 0x808080

	DWORD prev_bgr_col = context->fill_color;

	// First layer
	context->fill_color = (raised ? col_1 : col_0);
	FillSolidRectangle(context, x    , y    , w    , h    );

	context->fill_color = (raised ? col_0 : col_1);
	FillSolidRectangle(context, x    , y    , w - 1, h - 1);

	context->fill_color = (raised ? col_3 : col_2);
	FillSolidRectangle(context, x + 1, y + 1, w - 2, h - 2);

	context->fill_color = (raised ? col_2 : col_3);
	FillSolidRectangle(context, x + 1, y + 1, w - 3, h - 3);

	context->fill_color = prev_bgr_col;
	FillSolidRectangle(context, x + 2, y + 2, w - 4, h - 4);
}

void CDrawUtils::DrawString(DRAWCONTEXT *context,
							int x,
							int y,
							int width, 
							int height, 
							TSTRING string)
{
	SetTextColor(context->hdc, context->draw_color);
	SetBkColor(context->hdc, context->fill_color);

	if ((width > -1) && 
		(height > -1))
	{
		RECT bounds;
		bounds.left = x;
		bounds.top = y;
		bounds.right = (x + width);
		bounds.bottom = (y + height);

		DrawText(context->hdc, string, StrLen(string), &bounds, DT_LEFT);
	}
	else
	{
		TextOut(context->hdc, x, y, string, StrLen(string));
	}
}