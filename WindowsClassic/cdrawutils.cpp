
#include "cdrawutils.h"
#include "utils.h"

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

void CDrawUtils::GetColorInfo(DWORD color_mask,
								const LPCOLORRGBINFO lpcolor)
{
	lpcolor->Alpha	= (color_mask >> 24) & 0xFF;
	lpcolor->Blue	= (color_mask >> 16) & 0xFF;
	lpcolor->Green	= (color_mask >>  8) & 0xFF;
	lpcolor->Red	= (color_mask >>  0) & 0xFF;
}

// "fill_color" acts here as the first color
// (in this case the left side)
// and "draw_color" is the opposite color 
// (in this case, right)
void CDrawUtils::FillGradientRectangleLTR(LPDRAWCONTEXT context,
											int x,
											int y,
											int w,
											int h)
{
	COLORRGBINFO col1, 
				 col2;

	GetColorInfo(context->fill_color, &col1);
	GetColorInfo(context->draw_color, &col2);

	TRIVERTEX vertices[] = 
	{
		{ x    , y    , col1.Red << 8, col1.Green << 8, col1.Blue << 8, col1.Alpha << 8 },
		{ x + w, y + h, col2.Red << 8, col2.Green << 8, col2.Blue << 8, col2.Alpha << 8 }
	};

	GRADIENT_RECT matrix_mesh[] = 
	{
		{ 0, 1 }
	};

	GradientFill(
		context->paintstruct.hdc, 
		vertices, 
		ARRAYSIZE(vertices), 
		matrix_mesh,
		ARRAYSIZE(matrix_mesh),
		GRADIENT_FILL_RECT_H
	);
}

void CDrawUtils::FillGradientRectangleTTB(LPDRAWCONTEXT context,
											int x,
											int y,
											int w,
											int h)
{
	COLORRGBINFO col1,
				 col2;

	GetColorInfo(context->fill_color, &col1);
	GetColorInfo(context->draw_color, &col2);

	TRIVERTEX vertices[] =
	{
		{ x    , y    , col1.Red << 8, col1.Green << 8, col1.Blue << 8, col1.Alpha << 8 },
		{ x + w, y + h, col2.Red << 8, col2.Green << 8, col2.Blue << 8, col2.Alpha << 8 }
	};

	GRADIENT_RECT matrix_mesh[] =
	{
		{ 0, 1 }
	};

	GradientFill(
		context->paintstruct.hdc,
		vertices,
		ARRAYSIZE(vertices),
		matrix_mesh,
		ARRAYSIZE(matrix_mesh),
		GRADIENT_FILL_RECT_V
	);
}

// This might not be the best function to draw a simple rectangle
// but it should do for now.
void CDrawUtils::FillSolidRectangle(LPDRAWCONTEXT context,
									int x,
									int y,
									int w,
									int h)
{
	SetDCBrushColor(context->paintstruct.hdc, context->fill_color);
	SetDCPenColor(context->paintstruct.hdc, context->fill_color);

	SelectObject(context->paintstruct.hdc, (HBRUSH)GetStockObject(DC_BRUSH));
	SelectObject(context->paintstruct.hdc, (HPEN)GetStockObject(DC_PEN));

	Rectangle(context->paintstruct.hdc, x, y, x + w, y + h);
}

void CDrawUtils::FillPolygon(LPDRAWCONTEXT context,
							const POINT *vectors,
							int vector_cnt)
{
	// No we don't want any outline, Microsoft -.-
	SetDCBrushColor(context->paintstruct.hdc, context->fill_color);
	SetDCPenColor(context->paintstruct.hdc, context->fill_color);

	SelectObject(context->paintstruct.hdc, (HBRUSH)GetStockObject(DC_BRUSH));
	SelectObject(context->paintstruct.hdc, (HPEN)GetStockObject(DC_PEN));

	Polygon(context->paintstruct.hdc, vectors, vector_cnt);
}

void CDrawUtils::FillRectangle3DSmall(LPDRAWCONTEXT context,
										int x,
										int y,
										int w,
										int h, 
										UINT flags)
{
	DWORD col_0 = 0xFFFFFF;	// Should always be white
	DWORD col_1 = 0x000000; // Shadow - always black
	DWORD col_2 = MakeColorBrighter(context->fill_color, 32);
	DWORD col_3 = MakeColorDarker(context->fill_color, 64);

	// Save the previous background color to restore it later
	DWORD prev_bgr_col = context->fill_color;

	UINT raised = (flags & RECT_RAISED);
	
	if (flags & RECT_OUTLINED)
	{
		context->fill_color = col_1;
		FillSolidRectangle(context, x, y, w, h);

		++x;
		++y;
		w -= 2;
		h -= 2;
	}

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

void CDrawUtils::FillRectangle3D(LPDRAWCONTEXT context,
								int x, 
								int y, 
								int w, 
								int h, 
								UINT flags)
{
	DWORD col_0 = MakeColorBrighter(context->fill_color, 32);	// 0xE0E0E0
	DWORD col_1 = 0x000000;										// Shadow - should always be black no matter what
	DWORD col_2 = 0xFFFFFF;										// Should always be white
	DWORD col_3 = MakeColorDarker(context->fill_color, 64);		// 0x808080

	DWORD prev_bgr_col = context->fill_color;

	UINT raised = (flags & RECT_RAISED);
	
	if (flags & RECT_OUTLINED)
	{
		context->fill_color = col_1;
		FillSolidRectangle(context, x, y, w, h);

		++x;
		++y;
		w -= 2;
		h -= 2;
	}

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

void CDrawUtils::DrawString(LPDRAWCONTEXT context,
							STRING string,
							int x,
							int y,
							int width, 
							int height, 
							UINT text_format)
{
	SetBkColor(context->paintstruct.hdc, context->fill_color);
	SetTextColor(context->paintstruct.hdc, context->draw_color);

	if ((width > -1) && 
		(height > -1))
	{
		RECT bounds;
		bounds.left = x;
		bounds.top = y;
		bounds.right = (x + width);
		bounds.bottom = (y + height);

		DrawText(context->paintstruct.hdc, string, StrLen(string), &bounds, text_format);
	}
	else
	{
		TextOut(context->paintstruct.hdc, x, y, string, StrLen(string));
	}
}