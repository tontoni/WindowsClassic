
#include "cclassicwnd.h"

#define IsPointOverTitlebar(mx, my, width)							(IsPointInArea(mx, my, 3, 3, width - 57, 18))
#define IsPointOverCloseButton(mx, my, width, wnd_closable)			(IsPointInArea(mx, my, width - 21, 5, 16, 14)) && (wnd_closable)
#define IsPointOverMaximizeButton(mx, my, width, wnd_resizable)		(IsPointInArea(mx, my, width - 39, 5, 16, 14)) && (wnd_resizable)
#define IsPointOverMinimizeButton(mx, my, width, wnd_minimizable)	(IsPointInArea(mx, my, width - 55, 5, 16, 14)) && (wnd_minimizable)

static WindowEdge IsPointInWindowEdge(int px, int py, int win_width, int win_height)
{
	// Top Border
	if (IsPointInArea(px, py, 3, 0, win_width - 6, 3))
	{
		return WE_TOPBORDER;
	}
	// Top Left Corner
	else if (IsPointInArea(px, py, 0, 0, 3, 3))
	{
		return WE_TOPLEFT;
	}
	// Top Right Corner
	else if (IsPointInArea(px, py, win_width - 3, 0, 3, 3))
	{
		return WE_TOPRIGHT;
	}
	// Left Border
	else if (IsPointInArea(px, py, 0, 3, 3, win_height - 6))
	{
		return WE_LEFTBORDER;
	}
	// Bottom Left Corner
	else if (IsPointInArea(px, py, 0, win_height - 3, 3, 3))
	{
		return WE_BOTTOMLEFT;
	}
	// Right Border
	else if (IsPointInArea(px, py, win_width - 3, 3, 3, win_height - 6))
	{
		return WE_RIGHTBORDER;
	}
	// Bottom Right Corner
	else if (IsPointInArea(px, py, win_width - 3, win_height - 3, 3, 3))
	{
		return WE_BOTTOMRIGHT;
	}
	// Bottom Border
	else if (IsPointInArea(px, py, 3, win_height - 3, win_width - 3, 3))
	{
		return WE_BOTTOMBORDER;
	}

	return WE_NOTHING;
}

#define WND_CLASSNAME			"Windows_Classic_Window"
#define WND_CLASSNAME_CLIENT	"Windows_Classic_Client"

RECT CClassicWnd::AREA_GetTitlebarBounds()
{
	RECT bounds = { 
		3, 3, 
		MAKEWIDTH(this->__bounds) - 57, 18
	};

	return bounds;
}

RECT CClassicWnd::AREA_GetCloseButtonBounds()
{
	RECT bounds = {
		MAKEWIDTH(this->__bounds) - 21, 5,
		16, 14
	};

	return bounds;
}

RECT CClassicWnd::AREA_GetMaximizeButtonBounds()
{
	RECT bounds = {
		MAKEWIDTH(this->__bounds) - 39, 5, 
		16, 14
	};

	return bounds;
}

RECT CClassicWnd::AREA_GetMinimizeButtonBounds()
{
	RECT bounds = {
		MAKEWIDTH(this->__bounds) - 55, 5,
		16, 14
	};

	return bounds;
}

CClassicWnd::CClassicWnd(HINSTANCE hInst, HICON icon, HICON icon_small)
{
	this->wnd_class.cbSize				= sizeof(WNDCLASSEX);
	this->wnd_class.style				= (CS_HREDRAW | CS_VREDRAW);
	this->wnd_class.lpfnWndProc			= Internal_WndProc;
	this->wnd_class.cbClsExtra			= 0;
	this->wnd_class.cbWndExtra			= 0;
	this->wnd_class.hInstance			= hInst;
	this->wnd_class.hIcon				= icon;
	this->wnd_class.hIconSm				= icon_small;
	this->wnd_class.hCursor				= LoadCursor(NULL, IDC_ARROW);
	this->wnd_class.hbrBackground		= this->classic_default_brush;
	this->wnd_class.lpszMenuName		= NULL;
	this->wnd_class.lpszClassName		= WND_CLASSNAME;

	if (!RegisterClassEx(&this->wnd_class))
	{
		MessageBox(NULL, "Call to RegisterClassEx failed!", "Win32", NULL);
		return;
	}

	this->wnd_class_client.cbSize			= sizeof(WNDCLASSEX);
	this->wnd_class_client.style			= (CS_HREDRAW | CS_VREDRAW);
	this->wnd_class_client.lpfnWndProc		= Internal_WndProc_Client;
	this->wnd_class_client.cbClsExtra		= 0;
	this->wnd_class_client.cbWndExtra		= 0;
	this->wnd_class_client.hInstance		= hInst;
	this->wnd_class_client.hIcon			= NULL;
	this->wnd_class_client.hIconSm			= NULL;
	this->wnd_class_client.hCursor			= this->wnd_class.hCursor;
	this->wnd_class_client.hbrBackground	= this->classic_default_brush;
	this->wnd_class_client.lpszMenuName		= NULL;
	this->wnd_class_client.lpszClassName	= WND_CLASSNAME_CLIENT;
	
	if (!RegisterClassEx(&this->wnd_class_client))
	{
		MessageBox(NULL, "Call to RegisterClassEx failed!", "Win32", NULL);
		return;
	}

	this->__components = List_Create(256);
}

CClassicWnd::~CClassicWnd()
{
}

int CClassicWnd::CreateAndShow(int xPos, 
								int yPos, 
								int width, 
								int height, 
								TSTRING title)
{
	this->hWnd = CreateWindow(
		WND_CLASSNAME,
		(!title) ? this->__title : title,
		WS_POPUP,
		(xPos < 0) ? this->__bounds.left : xPos, 
		(yPos < 0) ? this->__bounds.top : yPos,
		(width < 0) ? (this->__bounds.right - this->__bounds.left) : width,
		(height < 0) ? (this->__bounds.bottom - this->__bounds.top) : height,
		NULL,
		NULL,
		this->wnd_class.hInstance,
		NULL
	);

	if (!this->hWnd)
	{
		MessageBox(NULL, "Call to CreateWindow failed!", "Win32", NULL);
		return 1;
	}

	SetWindowLong(this->hWnd, GWLP_USERDATA, (long)this);

	// NOTE(toni): Usually the "lParam" argument should 
	// be a "CREATESTRUCT" according to Microsoft
	// But we don't need it in this case!
	SendMessage(this->hWnd, WM_CREATE, 0, 0);

	this->SetVisible(true);

	// Main message loop:
	MSG msg;

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

void CClassicWnd::Destroy()
{
	DestroyWindow(this->hWnd_client);
	DestroyWindow(this->hWnd);
}

// Private function
void __SetBounds(CClassicWnd *inst, 
				HWND hWnd, 
				RECT &bounds, 
				int xPos, 
				int yPos, 
				int width, 
				int height, 
				UINT flags)
{
	if (xPos > -1)
		bounds.left = xPos;
	
	if (yPos > -1)
		bounds.top = yPos;
	
	if (width > -1)
		bounds.right = MAKECOORDINATE(bounds.left, width);
	
	if (height > -1)
		bounds.bottom = MAKECOORDINATE(bounds.top, height);

	if (!inst->IsReady())
		return;

	SetWindowPos(
		hWnd,
		NULL,
		xPos, yPos,
		width, height,
		flags
	);
}

void CClassicWnd::SetBounds(int xPos, 
							int yPos, 
							int width, 
							int height)
{
	__SetBounds(
		this, 
		this->hWnd, 
		this->__bounds, 
		xPos, yPos, 
		width, height, 
		SWP_NOZORDER
	);
}

void CClassicWnd::SetPosition(int xPos, int yPos)
{
	__SetBounds(
		this,
		this->hWnd,
		this->__bounds,
		xPos, yPos,
		-1, -1,
		SWP_NOZORDER | SWP_NOSIZE
	);
}

void CClassicWnd::SetSize(int width, int height)
{
	__SetBounds(
		this,
		this->hWnd,
		this->__bounds,
		-1, -1,
		width, height,
		SWP_NOZORDER | SWP_NOMOVE
	);
}

void CClassicWnd::SetTitle(TSTRING new_title)
{
	this->__title = new_title;

	if (!this->IsReady())
		return;

	SetWindowText(this->hWnd, new_title);
	this->RepaintWindow();
}

void CClassicWnd::SetClosable(bool closable)
{
	this->closable = closable;
	this->RepaintWindow();
}

void CClassicWnd::SetResizable(bool resizable)
{
	this->resizable = resizable;
	this->RepaintWindow();
}

void CClassicWnd::SetMinimizable(bool minimizable)
{
	this->minimizable = minimizable;
	this->RepaintWindow();
}

void CClassicWnd::SetVisible(bool visible)
{
	this->visible = visible;
	
	if (this->hWnd)
	{
		ShowWindow(this->hWnd, (visible ? SW_SHOW : SW_HIDE));
		UpdateWindow(this->hWnd);
	}
}

void CClassicWnd::SetBackgroundColor(DWORD color)
{
	this->__background_color = color;
	this->RepaintClientArea();
}

void CClassicWnd::SetForegroundColor(DWORD color)
{
	this->__foreground_color = color;
	this->RepaintClientArea();
}

void CClassicWnd::RepaintWindow()
{
	if (!this->IsReady())
		return;

	RedrawWindow(this->hWnd, NULL, NULL, RDW_INVALIDATE);
}

void CClassicWnd::RepaintClientArea()
{
	if (!this->IsReady())
		return;

	RedrawWindow(this->hWnd_client, NULL, NULL, RDW_INVALIDATE);
}

RECT CClassicWnd::GetWindowBounds()
{
	/*
	RECT bounds;
	GetWindowRect(this->hWnd, &bounds);

	return bounds;
	*/
	return this->__bounds;
}

POINT CClassicWnd::GetPosition()
{
	RECT _rect = this->GetWindowBounds();
	POINT p = { _rect.left, _rect.top };

	return p;
}

SIZE CClassicWnd::GetWindowSize()
{
	RECT _rect = this->GetWindowBounds();
	SIZE s = { MAKEWIDTH(_rect), MAKEHEIGHT(_rect) };
	return s;
}

SIZE CClassicWnd::GetClientSize()
{
	RECT _rect;
	GetWindowRect(this->hWnd_client, &_rect);

	SIZE s = { MAKEWIDTH(_rect), MAKEHEIGHT(_rect) };

	return s;
}

void CClassicWnd::AddComponent(CClassicComponent *component)
{
	List_Add(this->__components, component);
	
	// Doesn't work since the Parent HWND hasn't been created yet!
	// component->OnAdd(this->hWnd_client);
}

void CClassicWnd::RemoveComponent(CClassicComponent *component)
{
	List_Remove(this->__components, component);
	component->OnRemove(this->hWnd_client);
}

void CClassicWnd::RemoveAll()
{
	for (int i = 0; 
		i < List_GetCount(this->__components); 
		++i)
	{
		CClassicComponent *comp = (CClassicComponent *)List_Get(this->__components, i);
		comp->OnRemove(this->hWnd_client);
	}

	List_Clear(this->__components);
}

LRESULT CALLBACK CClassicWnd::WndProc(HWND hWnd, 
										UINT message, 
										WPARAM wParam, 
										LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT paint_strct;

	RECT wnd_bounds;
	GetWindowRect(hWnd, &wnd_bounds);

	int width = MAKEWIDTH(wnd_bounds),
		height = MAKEHEIGHT(wnd_bounds);

	switch (message)
	{
		case WM_CREATE:
		{
			const TSTRING font_name = "MS Sans Serif";
			const long font_size = 8;

			hdc = GetDC(hWnd);
			
			LOGFONT logfont = { 0 };
			logfont.lfHeight = -MulDiv(font_size, GetDeviceCaps(hdc, LOGPIXELSY), 72);
			logfont.lfWeight = FW_BOLD;

			strcpy(logfont.lfFaceName, font_name);

			this->titlebar_font = CreateFontIndirect(&logfont);

			ReleaseDC(hWnd, hdc);
			
			// TODO(toni): Probably gonna change this in the future...

			this->hWnd_client = CreateWindow(
				WND_CLASSNAME_CLIENT,
				NULL, 
				WS_CHILD | WS_VISIBLE,
				3, 
				22, 
				0, 
				0, 
				this->hWnd, 
				NULL, 
				this->wnd_class_client.hInstance,
				NULL
			);
			
			if (!this->hWnd_client)
			{
				MessageBox(NULL, "Call to CreateWindow failed!", "Win32", NULL);
				return 1;
			}
			
			SetWindowLong(this->hWnd_client, GWLP_USERDATA, (long)this);
			SendMessage(this->hWnd_client, WM_CREATE, 0, 0);

			ShowWindow(this->hWnd_client, SW_SHOW);
			UpdateWindow(this->hWnd_client);

			// Initial resize
			SendMessage(this->hWnd, WM_SIZE, 0, MAKELPARAM(width, height));
		} break;
		case WM_SIZE:
		{
			int new_w = (int)((short)LOWORD(lParam)),
				new_h = (int)((short)HIWORD(lParam));

			// If the client area window is ready...
			if (this->hWnd_client)
			{
				SetWindowPos(
					this->hWnd_client, 
					NULL, 
					-1, 
					-1, 
					new_w - 6, // Remember: Always double the value
					new_h - 25, 
					SWP_NOZORDER | SWP_NOMOVE
				);
			}
		} break;
		case WM_PAINT:
		{
			hdc = BeginPaint(hWnd, &paint_strct);

			// Selecting our own font
			SelectObject(hdc, this->titlebar_font);

			// Window Border draw code
			DRAWCONTEXT context;
			context.paintstruct = paint_strct;
			
			// The base color is that nice Windows Classic gray
			context.fill_color = CLASSIC_DEFAULT_BASECOLOR;
			context.draw_color = 0x000000;

			CDrawUtils::FillRectangle3D(&context, 0, 0, width, height, true);

			// Window titlebar drawcode

			DWORD col_active = (activated ? 0x800000 : 0x808080);
			context.fill_color = col_active;

			CDrawUtils::FillSolidRectangle(
				&context, 
				3, 
				3, 
				width - 6, 
				18
			);

			int title_x = 5;

			if (wnd_class.hIconSm)
			{
				// Draw the fucking Window Icon in the top left corner
				// (if there is one)
				DrawIconEx(hdc, 5, 4, wnd_class.hIconSm, 16, 16, 0, NULL, DI_NORMAL);

				title_x += 18; // Icon Width + 2 pixel extra
			}

			int title_w_div = 57;

			if (!this->minimizable)
				title_w_div -= 16;

			if (!this->resizable)
				title_w_div -= 18;

			if (!this->closable)
				title_w_div -= 18;

			TCHAR wnd_title[128];
			GetWindowText(this->hWnd, wnd_title, ARRAYSIZE(wnd_title));

			context.fill_color = col_active;
			context.draw_color = 0xFFFFFF;

			CDrawUtils::DrawString(
				&context, 
				wnd_title, 
				title_x, 5, 
				width - (title_w_div + title_x), 18
			);

			// X button
			// Hardcoded colors - Not great i know!
			if (this->closable)
			{
				context.fill_color = CLASSIC_DEFAULT_BASECOLOR;
				CDrawUtils::FillRectangle3DSmall(&context, width - 21, 5, 16, 14, !pressed_button_close);

				POINT x_pnt_1[4];
				x_pnt_1[0].x = width - 17;
				x_pnt_1[0].y = 8;
				x_pnt_1[1].x = width - 16;
				x_pnt_1[1].y = 8;
				x_pnt_1[2].x = width - 11;
				x_pnt_1[2].y = 14;
				x_pnt_1[3].x = width - 10;
				x_pnt_1[3].y = 14;

				POINT x_pnt_2[4];
				x_pnt_2[0].x = width - 10;
				x_pnt_2[0].y = 8;
				x_pnt_2[1].x = width - 11;
				x_pnt_2[1].y = 8;
				x_pnt_2[2].x = width - 16;
				x_pnt_2[2].y = 14;
				x_pnt_2[3].x = width - 17;
				x_pnt_2[3].y = 14;

				context.fill_color = 0x000000;

				CDrawUtils::FillPolygon(&context, x_pnt_1, ARRAYSIZE(x_pnt_1));
				CDrawUtils::FillPolygon(&context, x_pnt_2, ARRAYSIZE(x_pnt_2));
			}

			// Maximize button
			if ((this->resizable) || 
				(this->minimizable))
			{
				context.fill_color = CLASSIC_DEFAULT_BASECOLOR;
				CDrawUtils::FillRectangle3DSmall(&context, width - 39, 5, 16, 14, !this->pressed_button_maximize);

				if (maximized)
				{
					context.fill_color = 0x0000000;
					CDrawUtils::FillSolidRectangle(&context, width - 34, 7, 6, 6);

					context.fill_color = CLASSIC_DEFAULT_BASECOLOR;
					CDrawUtils::FillSolidRectangle(&context, width - 33, 9, 4, 3);

					context.fill_color = 0x0000000;
					CDrawUtils::FillSolidRectangle(&context, width - 36, 10, 6, 6);

					context.fill_color = CLASSIC_DEFAULT_BASECOLOR;
					CDrawUtils::FillSolidRectangle(&context, width - 35, 12, 4, 3);
				}
				else if (!this->resizable)
				{
					// When the maximize button is disabled...
					// Unmaximized state

					context.fill_color = 0xFFFFFF;
					CDrawUtils::FillSolidRectangle(&context, width - 35, 8, 9, 9);

					context.fill_color = 0x808080;
					CDrawUtils::FillSolidRectangle(&context, width - 36, 7, 9, 9);

					context.fill_color = 0xFFFFFF;
					CDrawUtils::FillSolidRectangle(&context, width - 35, 9, 7, 6);

					context.fill_color = CLASSIC_DEFAULT_BASECOLOR;
					CDrawUtils::FillSolidRectangle(&context, width - 34, 10, 6, 5);
				}
				else
				{
					context.fill_color = 0x000000;
					CDrawUtils::FillSolidRectangle(&context, width - 36, 7, 9, 9);

					context.fill_color = CLASSIC_DEFAULT_BASECOLOR;
					CDrawUtils::FillSolidRectangle(&context, width - 35, 9, 7, 6);
				}
			}
			
			// Minimize button
			if (this->minimizable)
			{
				context.fill_color = CLASSIC_DEFAULT_BASECOLOR;
				CDrawUtils::FillRectangle3DSmall(&context, width - 55, 5, 16, 14, !this->pressed_button_minimize);

				context.fill_color = 0x000000;
				CDrawUtils::FillSolidRectangle(&context, width - 51, 14, 6, 2);
			}

			EndPaint(hWnd, &paint_strct);
		} break;
		case WM_LBUTTONDOWN:
		{
			int mx = (int)((short)LOWORD(lParam)),
				my = (int)((short)HIWORD(lParam));

			// X button
			if (IsPointOverCloseButton(mx, my, width, this->closable))
			{
				this->pressed_button_close = true;
				this->RepaintWindow();
			}
			// Maximize button
			else if (IsPointOverMaximizeButton(mx, my, width, this->resizable))
			{
				this->pressed_button_maximize = true;
				this->RepaintWindow();
			}
			// Minimize button
			else if (IsPointOverMinimizeButton(mx, my, width, this->minimizable))
			{
				this->pressed_button_minimize = true;
				this->RepaintWindow();
			}
			else if (!this->maximized)
			{
				// Titlebar
				if (IsPointOverTitlebar(mx, my, width))
				{
					this->drag_window = true;

					this->prev_mx = mx;
					this->prev_my = my;
				}
				// Window Edges
				else if (this->resizable)
				{
					WindowEdge edge = IsPointInWindowEdge(mx, my, width, height);

					if (edge != WE_NOTHING)
					{
						this->resize_edge = edge;

						this->prev_mx = mx;
						this->prev_my = my;
					}
				}
			}

			SetCapture(hWnd);
		} break;
		case WM_LBUTTONUP:
		{
			int mx = (int)((short)LOWORD(lParam)),
				my = (int)((short)HIWORD(lParam));

			drag_window = false;
			resize_edge = WE_NOTHING;

			if (pressed_button_close)
			{
				pressed_button_close = false;
				
				if (IsPointOverCloseButton(mx, my, width, this->closable))
				{
					// User clicked and released on the X button
					this->Destroy();
					break;
				}

				this->RepaintWindow();
			}

			if (this->pressed_button_maximize)
			{
				this->pressed_button_maximize = false;
				
				if (this->maximized)
				{
					ShowWindow(hWnd, SW_RESTORE);
					this->maximized = false;
				}
				else if (IsPointOverMaximizeButton(mx, my, width, this->resizable))
				{
					ShowWindow(hWnd, SW_MAXIMIZE);
					this->maximized = true;
				}

				this->RepaintWindow();
			}

			if (this->pressed_button_minimize)
			{
				this->pressed_button_minimize = false;
				
				if (IsPointOverMinimizeButton(mx, my, width, this->minimizable))
				{
					ShowWindow(hWnd, SW_MINIMIZE);
					this->minimized = true;
				}

				this->RepaintWindow();
			}
			
			ReleaseCapture();
		} break;
		case WM_MOUSEMOVE:
		{
			int mx = (int)((short)LOWORD(lParam)), 
				my = (int)((short)HIWORD(lParam));

			// If the window currently isn't being dragged and maximized...
			if ((!this->drag_window) &&
				(!this->maximized) && 
				(this->resizable))
			{
				WindowEdge edge = (
					(this->resize_edge != WE_NOTHING) ?
					this->resize_edge : IsPointInWindowEdge(mx, my, width, height)
				);

				switch (edge)
				{
					case WE_TOPBORDER:
					case WE_BOTTOMBORDER:
					{
						SetCursor(LoadCursor(NULL, IDC_SIZENS));
					} break;
					case WE_TOPLEFT:
					case WE_BOTTOMRIGHT:
					{
						SetCursor(LoadCursor(NULL, IDC_SIZENWSE));
					} break;
					case WE_TOPRIGHT:
					case WE_BOTTOMLEFT:
					{
						SetCursor(LoadCursor(NULL, IDC_SIZENESW));
					} break;
					case WE_LEFTBORDER:
					case WE_RIGHTBORDER:
					{
						SetCursor(LoadCursor(NULL, IDC_SIZEWE));
					} break;
					default:
					{
						// Standard cursor
						// SetCursor(LoadCursor(NULL, IDC_ARROW));
					} break;
				}
			}

			if (this->drag_window)
			{
				mx = (wnd_bounds.left - prev_mx) + mx;
				my = (wnd_bounds.top - prev_my) + my;

				POINT cl_pos;
				cl_pos.x = mx;
				cl_pos.y = my;

				ClientToScreen(hWnd, &cl_pos);
				
				this->SetPosition(mx, my);
			}
			// Resize the Window
			else if (this->resize_edge != WE_NOTHING)
			{
				mx = (wnd_bounds.left - prev_mx) + mx, 
				my = (wnd_bounds.top - prev_my) + my;
				
				RECT new_wnd_bounds;

				switch (this->resize_edge)
				{
					case WE_TOPBORDER:
					{
						new_wnd_bounds.left = wnd_bounds.left;
						new_wnd_bounds.right = wnd_bounds.right;
						new_wnd_bounds.top = my;
						new_wnd_bounds.bottom = wnd_bounds.bottom;
					} break;
					case WE_TOPLEFT:
					{
						new_wnd_bounds.left = mx;
						new_wnd_bounds.right = wnd_bounds.right;
						new_wnd_bounds.top = my;
						new_wnd_bounds.bottom = wnd_bounds.bottom;
					} break;
					case WE_TOPRIGHT:
					{
						new_wnd_bounds.left = wnd_bounds.left;
						new_wnd_bounds.right = mx + prev_mx;
						new_wnd_bounds.top = my;
						new_wnd_bounds.bottom = wnd_bounds.bottom;
					} break;
					case WE_LEFTBORDER:
					{
						new_wnd_bounds.left = mx;
						new_wnd_bounds.right = wnd_bounds.right;
						new_wnd_bounds.top = wnd_bounds.top;
						new_wnd_bounds.bottom = wnd_bounds.bottom;
					} break;
					case WE_RIGHTBORDER:
					{
						new_wnd_bounds.left = wnd_bounds.left;
						new_wnd_bounds.right = mx + prev_mx;
						new_wnd_bounds.top = wnd_bounds.top;
						new_wnd_bounds.bottom = wnd_bounds.bottom;
					} break;
					case WE_BOTTOMLEFT:
					{
						new_wnd_bounds.left = mx;
						new_wnd_bounds.right = wnd_bounds.right;
						new_wnd_bounds.top = wnd_bounds.top;
						new_wnd_bounds.bottom = my + prev_my;
					} break;
					case WE_BOTTOMRIGHT:
					{
						new_wnd_bounds.left = wnd_bounds.left;
						new_wnd_bounds.right = mx + prev_mx;
						new_wnd_bounds.top = wnd_bounds.top;
						new_wnd_bounds.bottom = my + prev_my;
					} break;
					case WE_BOTTOMBORDER:
					{
						new_wnd_bounds.left = wnd_bounds.left;
						new_wnd_bounds.right = wnd_bounds.right;
						new_wnd_bounds.top = wnd_bounds.top;
						new_wnd_bounds.bottom = my + prev_my;
					} break;
					default:
						break;
				}

				int new_width = (new_wnd_bounds.right - new_wnd_bounds.left), 
					new_height = (new_wnd_bounds.bottom - new_wnd_bounds.top);

				if (new_width < 200)
				{
					new_width = 200;
					new_wnd_bounds.left = wnd_bounds.left;
				}

				if (new_height < 24)
				{
					new_height = 24;
					new_wnd_bounds.top = wnd_bounds.top;
				}

				UINT flags = SWP_NOZORDER;
				
				// The size hasn't changed, so it's not necessary to tell
				// Windows to re-set the X and Y position of our Window!
				if ((new_wnd_bounds.left == wnd_bounds.left) && 
					(new_wnd_bounds.top == wnd_bounds.top))
				{
					flags |= SWP_NOMOVE;
				}

				// ... the same goes for resizing!
				if ((new_width == width) && 
					(new_height == height))
				{
					flags |= SWP_NOSIZE;
				}

				SetWindowPos(
					hWnd, 
					NULL, 
					new_wnd_bounds.left, 
					new_wnd_bounds.top, 
					new_width, 
					new_height, 
					flags
				);
			}
		} break;
		case WM_ACTIVATE:
		{
			this->activated = (wParam > 0);

			if (this->activated)
				this->minimized = false;

			// Make the Window repaint
			this->RepaintWindow();
		} break;
		case WM_DESTROY:
		{			
			PostQuitMessage(0);
		} break;
		default:
		{
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}

	return 0;
}

LRESULT CALLBACK CClassicWnd::WndProc_Client(HWND hWnd,
												UINT message,
												WPARAM wParam,
												LPARAM lParam)
{
	RECT bounds;
	GetWindowRect(hWnd, &bounds);

	int width = MAKEWIDTH(bounds),
		height = MAKEHEIGHT(bounds);

	switch (message)
	{
		case WM_CREATE:
		{
			// Do something here ?

			for (int i = 0; 
				i < List_GetCount(this->__components); 
				++i)
			{
				CClassicComponent *comp = (CClassicComponent *)List_Get(this->__components, i);
				comp->OnAdd(hWnd);
			}
		} break;
		case WM_DESTROY:
		{
			this->RemoveAll();

			DeleteList(this->__components);
			this->__components = NULL;
		} break;
		case WM_PAINT:
		{
			HDC hdc;
			PAINTSTRUCT paintstrct;

			hdc = BeginPaint(hWnd, &paintstrct);
			
			DRAWCONTEXT context;
			context.paintstruct = paintstrct;
			context.fill_color = this->__background_color;
			context.draw_color = this->__foreground_color;

			CDrawUtils::FillSolidRectangle(&context, 0, 0, width, height);

			// This draw code is just for debugging here!
			// In the future this will probably go away
			CDrawUtils::DrawString(&context, "Microsoft doesn\'t wanna give us Windows Classic", 15, 40, -1, -1);
			CDrawUtils::DrawString(&context, "So we give Windows Classic to Microsoft!", 15, 60, -1, -1);
			CDrawUtils::DrawString(&context, ":P", 15, 80, -1, -1);

			EndPaint(hWnd, &paintstrct);
		} break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}