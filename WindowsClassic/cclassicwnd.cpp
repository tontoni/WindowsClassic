
#include "windowsclassic.h"
#include "utils.h"

#include <stdio.h>

typedef enum WndContextMenuOption
{
	WND_CONTEXT_MENU_RESTORE = 0x00, 
	WND_CONTEXT_MENU_MOVE, 
	WND_CONTEXT_MENU_SIZE, 
	WND_CONTEXT_MENU_MINIMIZE, 
	WND_CONTEXT_MENU_MAXIMIZE, 
	WND_CONTEXT_MENU_CLOSE
} WndContextMenuOption;

typedef struct __tagWndContextMenuUserdata
{
	WndContextMenuOption	menu_option;
	LPCClassicWnd			parent_window;
} WndContextMenuUserdata, *LPWndContextMenuUserdata;

static LONGLONG GetSystemTime()
{
	LARGE_INTEGER s_freq;
	BOOL s_use_qpc = QueryPerformanceFrequency(&s_freq);

	if (s_use_qpc)
	{
		LARGE_INTEGER time;
		QueryPerformanceCounter(&time);
		return (1000LL * time.QuadPart) / s_freq.QuadPart;
	}
	else
	{
		return GetTickCount();
	}
}

STRING GenerateNewClassName(STRING prefix)
{
	TCHAR buffer[64];
	LONGLONG sys_time = GetSystemTime();

	// Make sure that our name is REALLY unique!
	// (Sorry but i can't think of a better solution :P)
	sys_time *= (LONGLONG)(rand() % 10000);
	
	PrintTo(buffer, TEXT("%s_%I64d"), prefix, sys_time);
	
	STRING class_name = (STRING)malloc(sizeof(TCHAR) * StrLen(buffer));
	lstrcpy(class_name, buffer);
	
	return class_name;
}

SIZE GetFontMetrics(HFONT font, STRING text)
{
	// I know, it might not be good to create so many DC's
	// but it's the only way i know to pre-calculate things!
	HDC temp_dc = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL);
	SelectObject(temp_dc, font);

	SIZE size;
	GetTextExtentPoint32(temp_dc, text, StrLen(text), &size);

	DeleteDC(temp_dc);

	return size;
}

static bool IsPointOverTitlebar(int mx, 
								int my, 
								int width, 
								SIZE title_size, 
								bool wnd_minimizable, 
								bool wnd_resizable, 
								bool wnd_closable, 
								LPRECT bounds = NULL)
{
	int tbar_btn_w = (title_size.cy + 3);

	int _width = (width - ((
		(
			wnd_minimizable ?
			tbar_btn_w : 0
			) + (
				wnd_resizable ?
				tbar_btn_w : 0
			) + (
				wnd_closable ?
				tbar_btn_w : 0
			)
		) + 2)), 
		_height = title_size.cy + 5;

	if (bounds)
	{
		bounds->left = 3;
		bounds->top = 3;
		bounds->right = MAKECOORDINATE(3, _width);
		bounds->bottom = MAKECOORDINATE(3, _height);
	}

	return (
		IsPointInArea(
			mx, my, 
			3, 3, 
			_width, _height
		)
	);
}

static bool IsPointOverCloseButton(int mx, 
									int my, 
									int width, 
									SIZE title_size, 
									bool wnd_closable,
									LPRECT bounds = NULL)
{
	int tbar_btn_w = (title_size.cy + 3),
		tbar_btn_h = (title_size.cy + 1);

	int _x = (width - (tbar_btn_w + 5)),
		_y = 5;

	if (bounds)
	{
		bounds->left = _x;
		bounds->top = _y;
		bounds->right = MAKECOORDINATE(_x, tbar_btn_w);
		bounds->bottom = MAKECOORDINATE(_y, tbar_btn_h);
	}

	return (
		(IsPointInArea(
			mx, my, 
			_x, _y, 
			tbar_btn_w, tbar_btn_h
		)) &&
		(wnd_closable)
	);
}

static bool IsPointOverMaximizeButton(int mx, 
										int my, 
										int width, 
										SIZE title_size, 
										bool wnd_resizable,
										LPRECT bounds = NULL)
{
	int tbar_btn_w = (title_size.cy + 3),
		tbar_btn_h = (title_size.cy + 1);

	int _x = (width - ((tbar_btn_w * 2) + 7)),
		_y = 5;

	if (bounds)
	{
		bounds->left = _x;
		bounds->top = _y;
		bounds->right = MAKECOORDINATE(_x, tbar_btn_w);
		bounds->bottom = MAKECOORDINATE(_y, tbar_btn_h);
	}

	return (
		(IsPointInArea(
			mx, my, 
			_x, _y, 
			tbar_btn_w, tbar_btn_h
		)) &&
		(wnd_resizable)
	);
}

static bool IsPointOverMinimizeButton(int mx, 
										int my, 
										int width, 
										SIZE title_size, 
										bool wnd_minimizable,
										LPRECT bounds = NULL)
{
	int tbar_btn_w = (title_size.cy + 3),
		tbar_btn_h = (title_size.cy + 1);
	
	int _x = (width - ((tbar_btn_w * 3) + 7)),
		_y = 5;

	if (bounds)
	{
		bounds->left = _x;
		bounds->top = _y;
		bounds->right = MAKECOORDINATE(_x, tbar_btn_w);
		bounds->bottom = MAKECOORDINATE(_y, tbar_btn_h);
	}

	return (
		(IsPointInArea(
			mx, my, 
			_x, _y, 
			tbar_btn_w, tbar_btn_h
		)) &&
		(wnd_minimizable)
	);
}

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

static void WndPopupProc(HPOPUP popup, UINT message, LPVOID param);

CClassicWnd::__tagCClassicWnd(HINSTANCE hInst,
								STRING wndclass_name, 
								STRING wndclass_cl_name, 
								HICON icon, 
								HICON icon_small)
{
	this->wnd_class.cbSize					= sizeof(WNDCLASSEX);
	this->wnd_class.style					= (CS_HREDRAW | CS_VREDRAW);
	this->wnd_class.lpfnWndProc				= Internal_WndProc;
	this->wnd_class.cbClsExtra				= 0;
	this->wnd_class.cbWndExtra				= 0;
	this->wnd_class.hInstance				= hInst;
	this->wnd_class.hIcon					= icon;
	this->wnd_class.hIconSm					= icon_small;
	this->wnd_class.hCursor					= LoadCursor(NULL, IDC_ARROW);
	this->wnd_class.hbrBackground			= this->classic_default_brush;
	this->wnd_class.lpszMenuName			= NULL;
	this->wnd_class.lpszClassName			= wndclass_name;

	if (!RegisterClassEx(&this->wnd_class))
	{
		DBG_ErrorExit("Register WNDCLASSEX (Window)");
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
	this->wnd_class_client.lpszClassName	= wndclass_cl_name;

	if (!RegisterClassEx(&this->wnd_class_client))
	{
		DBG_ErrorExit("Register WNDCLASSEX (Window Client Area)");
	}

	this->__window_listeners = List_Create(64);
	this->__components = List_Create(256);

	this->font_titlebar = CreateSimpleFontIndependent(TEXT("MS Sans Serif"), 8, FW_BOLD);
	this->font_element = CreateSimpleFontIndependent(TEXT("MS Sans Serif"), 8);
}

CClassicWnd::~CClassicWnd()
{
}

int CClassicWnd::CreateAndShow(int xPos, 
								int yPos, 
								int width, 
								int height, 
								STRING title)
{
	this->hWnd = CreateWindow(
		this->wnd_class.lpszClassName,
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
		DBG_ErrorExit("Create HWND (Window)");
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
	
	// Post Cleanup
	UnregisterClass(
		this->wnd_class_client.lpszClassName, 
		this->wnd_class_client.hInstance
	);

	UnregisterClass(
		this->wnd_class.lpszClassName, 
		this->wnd_class.hInstance
	);

	return (int)msg.wParam;
}

void CClassicWnd::Destroy()
{
	/* !!! NOT THREADSAFE !!!
	DestroyWindow(this->hWnd_client);
	DestroyWindow(this->hWnd);
	*/

	SendMessage(this->hWnd_client, WM_CLOSE, 0, 0);
	SendMessage(this->hWnd, WM_CLOSE, 0, 0);
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
	bounds.left = xPos;
	bounds.top = yPos;

	if (width > -1)
		bounds.right = MAKECOORDINATE(bounds.left, width);
	
	if (height > -1)
		bounds.bottom = MAKECOORDINATE(bounds.top, height);

	if (!inst->IsReady())
		return;
	
	if ((width <= -1) &&
		(height <= -1))
		flags |= SWP_NOSIZE;

	SetWindowPos(
		hWnd,
		NULL,
		xPos, 
		yPos,
		(width > -1) ? width : MAKEWIDTH(bounds), 
		(height > -1) ? height : MAKEHEIGHT(bounds),
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
		SWP_NOZORDER | SWP_NOACTIVATE
	);
}

void CClassicWnd::SetPosition(int xPos, int yPos)
{
	__SetBounds(
		this,
		this->hWnd,
		this->__bounds,
		xPos, 
		yPos,
		MAKEWIDTH(this->__bounds), 
		MAKEHEIGHT(this->__bounds), 
		SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE
	);
}

void CClassicWnd::SetPositionRelativeTo(RECT bounds)
{
	int xPos = MAKERELATIVEPOSX(this->__bounds, bounds),
		yPos = MAKERELATIVEPOSY(this->__bounds, bounds);

	__SetBounds(
		this, 
		this->hWnd, 
		this->__bounds, 
		xPos, 
		yPos, 
		MAKEWIDTH(this->__bounds), 
		MAKEHEIGHT(this->__bounds), 
		SWP_NOZORDER | SWP_NOACTIVATE
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
		SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE
	);
}

void CClassicWnd::SetTitle(STRING new_title)
{
	this->__title = new_title;

	if (!this->IsReady())
		return;

	SetWindowText(this->hWnd, new_title);
	this->RepaintWindow();
}

void CClassicWnd::SetEnabled(bool enabled)
{
	this->enabled = enabled;

	if (!this->IsReady())
		return;

	EnableWindow(this->hWnd, enabled);
	EnableWindow(this->hWnd_client, enabled);
	this->RepaintWindow();
}

void CClassicWnd::SetClosable(bool closable)
{
	this->closable = closable;
	this->RepaintWindow();
}

void CClassicWnd::SetCloseButtonEnabled(bool enabled)
{
	this->close_button_enabled = enabled;
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

void CClassicWnd::SetTitlebarColorActiveLeft(DWORD color)
{
	this->color_titlebar_active_l = color;
	this->RepaintWindow();
}

void CClassicWnd::SetTitlebarColorActiveRight(DWORD color)
{
	this->color_titlebar_active_r = color;
	this->RepaintWindow();
}

void CClassicWnd::SetTitlebarColorInactiveLeft(DWORD color)
{
	this->color_titlebar_inactive_l = color;
	this->RepaintWindow();
}

void CClassicWnd::SetTitlebarColorInactiveRight(DWORD color)
{
	this->color_titlebar_inactive_r = color;
	this->RepaintWindow();
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

void CClassicWnd::SetTitlebarFont(HFONT font)
{
	this->font_titlebar = font;

	SendMessage(
		this->hWnd, 
		WM_SIZE, 
		0, 
		MAKELPARAM(
			MAKEWIDTH(this->__bounds), 
			MAKEHEIGHT(this->__bounds)
		)
	);

	this->RepaintWindow();
}

void CClassicWnd::SetFont(HFONT font)
{
	this->font_element = font;
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

RECT CClassicWnd::GetClientBounds()
{
	// RECT _rect;
	// GetWindowRect(this->hWnd_client, &_rect);

	RECT _rect = {
		0, // Left and Top are both always zero
		0, // simply because the client area is relative to the window itself!
		MAKEWIDTH(this->__bounds) - 6, 
		MAKEHEIGHT(this->__bounds) - 25
	};

	return _rect;
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
	RECT _rect = this->GetClientBounds();
	SIZE s = { MAKEWIDTH(_rect), MAKEHEIGHT(_rect) };

	return s;
}

HDC CClassicWnd::GetWindowDrawContext()
{
	return GetDC(this->hWnd);
}

HDC CClassicWnd::GetDrawContext()
{
	return GetDC(this->hWnd_client);
}

void CClassicWnd::ReleaseWindowDrawContext(HDC hdc)
{
	ReleaseDC(this->hWnd, hdc);
}

void CClassicWnd::ReleaseDrawContext(HDC hdc)
{
	ReleaseDC(this->hWnd_client, hdc);
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
		free(comp);
	}

	List_Clear(this->__components);
}

void CClassicWnd::AddWindowListener(WINDOWLISTENER listener)
{
	List_Add(this->__window_listeners, listener);
}

void CClassicWnd::RemoveWindowListener(WINDOWLISTENER listener)
{
	List_Remove(this->__window_listeners, listener);
}

void CClassicWnd::RemoveAllWindowListeners()
{
	List_Clear(this->__window_listeners);
}

LRESULT CALLBACK CClassicWnd::WndProc(HWND hWnd, 
										UINT message, 
										WPARAM wParam, 
										LPARAM lParam)
{
	RECT wnd_bounds;
	GetWindowRect(hWnd, &wnd_bounds);

	int width = MAKEWIDTH(wnd_bounds),
		height = MAKEHEIGHT(wnd_bounds);

	switch (message)
	{
		case WM_CREATE:
		{
			SIZE title_size = GetFontMetrics(this->font_titlebar, this->__title);

			this->hWnd_client = CreateWindow(
				this->wnd_class_client.lpszClassName,
				NULL, 
				WS_CHILD | WS_VISIBLE,
				3, 
				title_size.cy + 9,
				0, 
				0, 
				this->hWnd, 
				NULL, 
				this->wnd_class_client.hInstance,
				NULL
			);
			
			if (!this->hWnd_client)
			{
				DBG_ErrorExit("Create HWND (Window Client Area)");
				return 1;
			}
			
			SetWindowLong(this->hWnd_client, GWLP_USERDATA, (long)this);
			SendMessage(this->hWnd_client, WM_CREATE, 0, 0);

			ShowWindow(this->hWnd_client, SW_SHOW);
			UpdateWindow(this->hWnd_client);

			// Initial resize
			SendMessage(hWnd, WM_SIZE, 0, MAKELPARAM(width, height));
		} break;
		case WM_SIZE:
		{
			// If the client area window is ready...
			if (this->hWnd_client)
			{
				int new_w = (int)((short)LOWORD(lParam)),
					new_h = (int)((short)HIWORD(lParam));

				SIZE title_size = GetFontMetrics(this->font_titlebar, this->__title);

				SetWindowPos(
					this->hWnd_client, 
					NULL, 
					3, 
					title_size.cy + 9,
					new_w - 6, // Remember: Always double the value
					new_h - (title_size.cy + 12),
					SWP_NOZORDER | SWP_NOACTIVATE
				);
			}
		} break;
		case WM_PAINT:
		{
			HDC hdc;
			PAINTSTRUCT paint_strct;

			hdc = BeginPaint(hWnd, &paint_strct);

			// Selecting our own font
			SelectObject(hdc, this->font_titlebar);

			// Window Border draw code
			DRAWCONTEXT context;
			context.paintstruct = paint_strct;
			
			// The base color is that nice Windows Classic gray
			context.fill_color = CLASSIC_DEFAULT_BASECOLOR;
			context.draw_color = 0x000000;

			CDrawUtils::FillRectangle3D(&context, 0, 0, width, height, RECT_RAISED);

			// Window titlebar drawcode

			context.fill_color = (
				this->activated ? 
				this->color_titlebar_active_l : 
				this->color_titlebar_inactive_l
			);
			
			context.draw_color = (
				this->activated ? 
				this->color_titlebar_active_r : 
				this->color_titlebar_inactive_r
			);

			SIZE title_size = GetFontMetrics(this->font_titlebar, this->__title);

			CDrawUtils::FillGradientRectangleLTR(
				&context,
				3,
				3,
				width - 6,
				title_size.cy + 5
			);

			int title_x = 5;

			if (wnd_class.hIconSm)
			{
				int icon_size = (title_size.cy + 3);

				// Draw the fucking Window Icon in the top left corner
				// (if there is one)
				DrawIconEx(
					hdc, 
					5, 4,
					wnd_class.hIconSm, 
					icon_size, icon_size,
					0, 
					NULL, 
					DI_NORMAL
				);

				title_x += (icon_size + 2); // Icon Width + 2 pixel extra
			}

			int title_w_div = 57;

			if (!this->minimizable)
				title_w_div -= 16;

			if (!this->resizable)
				title_w_div -= 18;

			if (!this->closable)
				title_w_div -= 18;
			
			context.draw_color = (this->activated ? 0xFFFFFF : CLASSIC_DEFAULT_BASECOLOR);

			SetBkMode(hdc, TRANSPARENT);

			CDrawUtils::DrawString(
				&context, 
				this->__title,
				title_x, 5, 
				width - (title_w_div + title_x), title_size.cy
			);

			SetBkMode(hdc, OPAQUE);

			int tbar_btn_w = (title_size.cy + 3),
				tbar_btn_h = (title_size.cy + 1);

			// X button
			// Hardcoded colors - Not great i know!
			if (this->closable)
			{
				context.fill_color = CLASSIC_DEFAULT_BASECOLOR;

				int xbtn_x = (width - (tbar_btn_w + 5)),
					xbtn_y = 5;

				CDrawUtils::FillRectangle3DSmall(&context, xbtn_x, xbtn_y, tbar_btn_w, tbar_btn_h, (!pressed_button_close) ? RECT_RAISED : 0);

				if (!this->close_button_enabled)
				{
					POINT x_pnt_3[4];
					x_pnt_3[0].x = xbtn_x + 5;
					x_pnt_3[0].y = xbtn_y + 4;
					x_pnt_3[1].x = xbtn_x + 6;
					x_pnt_3[1].y = xbtn_y + 4;
					x_pnt_3[2].x = xbtn_x + (tbar_btn_w - 5);
					x_pnt_3[2].y = xbtn_y + (tbar_btn_h - 4);
					x_pnt_3[3].x = xbtn_x + (tbar_btn_w - 4);
					x_pnt_3[3].y = xbtn_y + (tbar_btn_h - 4);

					POINT x_pnt_4[4];
					x_pnt_4[0].x = xbtn_x + (tbar_btn_w - 4);
					x_pnt_4[0].y = xbtn_y + 4;
					x_pnt_4[1].x = xbtn_x + (tbar_btn_w - 5);
					x_pnt_4[1].y = xbtn_y + 4;
					x_pnt_4[2].x = xbtn_x + 6;
					x_pnt_4[2].y = xbtn_y + (tbar_btn_h - 4);
					x_pnt_4[3].x = xbtn_x + 5;
					x_pnt_4[3].y = xbtn_y + (tbar_btn_h - 4);

					context.fill_color = 0xFFFFFF;

					CDrawUtils::FillPolygon(&context, x_pnt_3, ARRAYSIZE(x_pnt_3));
					CDrawUtils::FillPolygon(&context, x_pnt_4, ARRAYSIZE(x_pnt_4));
				}

				POINT x_pnt_1[4];
				x_pnt_1[0].x = xbtn_x + 4;
				x_pnt_1[0].y = xbtn_y + 3;
				x_pnt_1[1].x = xbtn_x + 5;
				x_pnt_1[1].y = xbtn_y + 3;
				x_pnt_1[2].x = xbtn_x + (tbar_btn_w - 6);
				x_pnt_1[2].y = xbtn_y + (tbar_btn_h - 5);
				x_pnt_1[3].x = xbtn_x + (tbar_btn_w - 5);
				x_pnt_1[3].y = xbtn_y + (tbar_btn_h - 5);

				POINT x_pnt_2[4];
				x_pnt_2[0].x = xbtn_x + (tbar_btn_w - 5);
				x_pnt_2[0].y = xbtn_y + 3;
				x_pnt_2[1].x = xbtn_x + (tbar_btn_w - 6);
				x_pnt_2[1].y = xbtn_y + 3;
				x_pnt_2[2].x = xbtn_x + 5;
				x_pnt_2[2].y = xbtn_y + (tbar_btn_h - 5);
				x_pnt_2[3].x = xbtn_x + 4;
				x_pnt_2[3].y = xbtn_y + (tbar_btn_h - 5);

				context.fill_color = (this->close_button_enabled) ? 0x000000 : 0x808080;

				CDrawUtils::FillPolygon(&context, x_pnt_1, ARRAYSIZE(x_pnt_1));
				CDrawUtils::FillPolygon(&context, x_pnt_2, ARRAYSIZE(x_pnt_2));
			}

			// Maximize button
			if ((this->resizable) || 
				(this->minimizable))
			{
				context.fill_color = CLASSIC_DEFAULT_BASECOLOR;
				CDrawUtils::FillRectangle3DSmall(&context, width - ((tbar_btn_w * 2) + 7), 5, tbar_btn_w, tbar_btn_h, (!this->pressed_button_maximize) ? RECT_RAISED : 0);

				if (maximized)
				{
					context.fill_color = 0x000000;
					CDrawUtils::FillSolidRectangle(&context, width - 34, 7, 6, 6);

					context.fill_color = CLASSIC_DEFAULT_BASECOLOR;
					CDrawUtils::FillSolidRectangle(&context, width - 33, 9, 4, 3);

					context.fill_color = 0x000000;
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
				CDrawUtils::FillRectangle3DSmall(&context, width - ((tbar_btn_w * 3) + 7), 5, tbar_btn_w, tbar_btn_h, (!this->pressed_button_minimize) ? RECT_RAISED : 0);

				context.fill_color = 0x000000;
				CDrawUtils::FillSolidRectangle(&context, width - 51, 14, 6, 2);
			}

			EndPaint(hWnd, &paint_strct);
		} break;
		case WM_LBUTTONDOWN:
		{
			int mx = (int)((short)LOWORD(lParam)),
				my = (int)((short)HIWORD(lParam));

			SIZE title_size = GetFontMetrics(this->font_titlebar, this->__title);
			RECT rdw_bounds;

			int icon_size = (title_size.cy + 3);

			// Window Icon
			if ((this->wnd_class.hIconSm) && 
				(IsPointInArea(mx, my, 5, 4, icon_size, icon_size)))
			{
				HPOPUP wnd_context_menu = CreatePopupMenuClassic(this->wnd_class.hInstance, WndPopupProc);

				HMENUITEM item_restore = CreateAndAppendMenuItemClassic(
					wnd_context_menu,
					CPM_ITEM_TEXT | CPM_ITEM_STATE_DISABLED,
					TEXT("Restore")
				);

				HMENUITEM item_move = CreateAndAppendMenuItemClassic(
					wnd_context_menu,
					CPM_ITEM_TEXT,
					TEXT("Move")
				);

				HMENUITEM item_size = CreateAndAppendMenuItemClassic(
					wnd_context_menu,
					CPM_ITEM_TEXT,
					TEXT("Size")
				);

				HMENUITEM item_minimize = CreateAndAppendMenuItemClassic(
					wnd_context_menu,
					CPM_ITEM_TEXT,
					TEXT("Minimize")
				);

				HMENUITEM item_maximize = CreateAndAppendMenuItemClassic(
					wnd_context_menu,
					CPM_ITEM_TEXT,
					TEXT("Maximize")
				);

				CreateAndAppendMenuItemClassic(wnd_context_menu, CPM_ITEM_SEPARATOR, NULL);

				HMENUITEM item_close = CreateAndAppendMenuItemClassic(
					wnd_context_menu,
					CPM_ITEM_TEXT | CPM_ITEM_DEFAULT,
					TEXT("Close")
				);

				// Yes i know, allocating all that memory just for the different
				// Userdata may be a waste and bogus, though as soon as
				// the handle to a menu item gets destroyed, it automatically
				// also frees occupied userdata.
				LPWndContextMenuUserdata option_restore = (LPWndContextMenuUserdata)malloc(sizeof(WndContextMenuUserdata));
				option_restore->parent_window = this;
				option_restore->menu_option = WND_CONTEXT_MENU_RESTORE;
				SetMenuItemUserdata(item_restore, option_restore);

				LPWndContextMenuUserdata option_move = (LPWndContextMenuUserdata)malloc(sizeof(WndContextMenuUserdata));
				option_move->parent_window = this;
				option_move->menu_option = WND_CONTEXT_MENU_MOVE;
				SetMenuItemUserdata(item_move, option_move);

				LPWndContextMenuUserdata option_size = (LPWndContextMenuUserdata)malloc(sizeof(WndContextMenuUserdata));
				option_size->parent_window = this;
				option_size->menu_option = WND_CONTEXT_MENU_SIZE;
				SetMenuItemUserdata(item_size, option_size);

				LPWndContextMenuUserdata option_minimize = (LPWndContextMenuUserdata)malloc(sizeof(WndContextMenuUserdata));
				option_minimize->parent_window = this;
				option_minimize->menu_option = WND_CONTEXT_MENU_MINIMIZE;
				SetMenuItemUserdata(item_minimize, option_minimize);

				LPWndContextMenuUserdata option_maximize = (LPWndContextMenuUserdata)malloc(sizeof(WndContextMenuUserdata));
				option_maximize->parent_window = this;
				option_maximize->menu_option = WND_CONTEXT_MENU_MAXIMIZE;
				SetMenuItemUserdata(item_maximize, option_maximize);

				LPWndContextMenuUserdata option_close = (LPWndContextMenuUserdata)malloc(sizeof(WndContextMenuUserdata));
				option_close->parent_window = this;
				option_close->menu_option = WND_CONTEXT_MENU_CLOSE;
				SetMenuItemUserdata(item_close, option_close);

				ShowPopupMenuClassic(wnd_context_menu);
			}
			// X button
			else if (IsPointOverCloseButton(mx, my, width, title_size, (this->closable) && (this->close_button_enabled), &rdw_bounds))
			{
				this->pressed_button_close = true;
				RedrawWindow(hWnd, &rdw_bounds, NULL, RDW_INVALIDATE);
			}
			// Maximize button
			else if (IsPointOverMaximizeButton(mx, my, width, title_size, this->resizable, &rdw_bounds))
			{
				this->pressed_button_maximize = true;
				RedrawWindow(hWnd, &rdw_bounds, NULL, RDW_INVALIDATE);
			}
			// Minimize button
			else if (IsPointOverMinimizeButton(mx, my, width, title_size, this->minimizable, &rdw_bounds))
			{
				this->pressed_button_minimize = true;
				RedrawWindow(hWnd, &rdw_bounds, NULL, RDW_INVALIDATE);
			}
			else if (!this->maximized)
			{
				// Titlebar
				if (IsPointOverTitlebar(mx, my, width, title_size, this->minimizable, this->resizable, this->closable))
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

			SIZE title_size = GetFontMetrics(this->font_titlebar, this->__title);
			RECT rdw_bounds;
			
			drag_window = false;
			resize_edge = WE_NOTHING;

			if (pressed_button_close)
			{
				pressed_button_close = false;
				
				if (IsPointOverCloseButton(mx, my, width, title_size, this->closable, &rdw_bounds))
				{
					// User clicked and released on the X button
					this->Destroy();
					break;
				}

				RedrawWindow(hWnd, &rdw_bounds, NULL, RDW_INVALIDATE);
			}

			if (this->pressed_button_maximize)
			{
				this->pressed_button_maximize = false;
				
				if (IsPointOverMaximizeButton(mx, my, width, title_size, this->resizable, &rdw_bounds))
				{
					ShowWindow(hWnd, (this->maximized ? SW_RESTORE : SW_MAXIMIZE));
					this->maximized = !this->maximized;
				}

				RedrawWindow(hWnd, &rdw_bounds, NULL, RDW_INVALIDATE);
			}

			if (this->pressed_button_minimize)
			{
				this->pressed_button_minimize = false;
				
				if (IsPointOverMinimizeButton(mx, my, width, title_size, this->minimizable, &rdw_bounds))
				{
					ShowWindow(hWnd, SW_MINIMIZE);
					this->minimized = true;
				}

				RedrawWindow(hWnd, &rdw_bounds, NULL, RDW_INVALIDATE);
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
				(this->resizable) && 
				(!this->pressed_button_minimize) && 
				(!this->pressed_button_maximize) && 
				(!this->pressed_button_close))
			{
				WindowEdge edge = (
					(this->resize_edge != WE_NOTHING) ?
					this->resize_edge : IsPointInWindowEdge(mx, my, width, height)
				);

				// Set the right cursor to the corresponding Window Edge
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

				UINT flags = SWP_NOZORDER | SWP_NOACTIVATE;
				
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

				this->__bounds = new_wnd_bounds;

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

			for (int i = 0; 
				i < List_GetCount(this->__components); 
				++i)
			{
				CClassicComponent *comp = (CClassicComponent *)List_Get(this->__components, i);
				comp->PostComponentMessage(WM_ACTIVATE, wParam, lParam);
			}
		} break;
		case WM_DESTROY:
		{
			DeleteObject(this->font_element);
			DeleteObject(this->font_titlebar);
			DeleteObject(this->classic_default_brush);

			PostQuitMessage(0);
		} break;
		default:
		{
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}

	// We got to notify all the window listeners (if there are some)
	for (int i = 0; 
		i < List_GetCount(this->__window_listeners); 
		++i)
	{
		WINDOWLISTENER listener = (WINDOWLISTENER)List_Get(this->__window_listeners, i);
		listener(this, message, wParam, lParam);
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
				comp->SetFont(this->font_element);
			}
		} break;
		case WM_DESTROY:
		{
			DeleteList(this->__window_listeners);
			this->__window_listeners = NULL;

			// Destroying all the components too?
			// I don't really know, but for now we just do it, so that we don't waste any memory!
			// Though, the remaining pointer addresses should not be reused, but there is no way to make sure that
			// they're all zeroed out!
			// So just please don't re-use the same addresses.
			for (int i = 0; 
				i < List_GetCount(this->__components); 
				++i)
			{
				CClassicComponent *comp = (CClassicComponent *)List_Get(this->__components, i);
				
				HFONT font = comp->GetFont();

				if ((font != this->font_titlebar) && 
					(font != this->font_element))
				{
					// If the component has a different allocated font element
					// delete it as well.
					DeleteObject(font);
				}

				comp->OnRemove(hWnd);
				free(comp);
			}

			DeleteList(this->__components);
			this->__components = NULL;
		} break;
		case WM_PAINT:
		{
			HDC hdc;
			PAINTSTRUCT paintstrct;

			hdc = BeginPaint(hWnd, &paintstrct);
			
			SelectObject(hdc, this->font_element);

			DRAWCONTEXT context;
			context.paintstruct = paintstrct;
			context.fill_color = this->__background_color;
			context.draw_color = this->__foreground_color;

			CDrawUtils::FillSolidRectangle(&context, 0, 0, width, height);
			
			EndPaint(hWnd, &paintstrct);
		} break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

void WndPopupProc(HPOPUP popup, UINT message, LPVOID param)
{
	if (message == CPM_ITEMSELECTED)
	{
		LPCPM_ITEMINFO info = (LPCPM_ITEMINFO)param;
		LPWndContextMenuUserdata userdata = (LPWndContextMenuUserdata)GetMenuItemUserdata(info->menu_item);

		switch (userdata->menu_option)
		{
			// TODO(toni): Handle all the other menu options!
			case WND_CONTEXT_MENU_CLOSE:
			{
				userdata->parent_window->Destroy();
			} break;
		}
	}
}