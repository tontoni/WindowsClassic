
#ifndef _CCLASSICWND_H_
#define _CCLASSICWND_H_

#pragma once

#include <Windows.h>

#include "types.h"
#include "cdrawutils.h"
#include "basiclist.h"

#define SINLINE static inline

#define CLASSIC_DEFAULT_BASECOLOR							(DWORD)0xC0C0C0

#define MAKESIZE(a, b)										(a - b)
#define MAKEWIDTH(rect)										MAKESIZE(rect.right, rect.left)
#define MAKEHEIGHT(rect)									MAKESIZE(rect.bottom, rect.top)

#define MAKECOORDINATE(a, b)								(a + b)

#define MAKERELATIVEPOSX(owner_bounds, target_bounds)		((target_bounds.right / 2) - (MAKEWIDTH(owner_bounds) / 2))
#define MAKERELATIVEPOSY(owner_bounds, target_bounds)		((target_bounds.bottom / 2) - (MAKEHEIGHT(owner_bounds) / 2))

#define CONTAINSFLAG(bitmask, flag)							((bitmask & flag) == flag )

#define IsPointInArea(px, py, ax, ay, aw, ah)	\
					((px >= ax) &&				\
					 (py >= ay) &&				\
					 (px < (ax + aw)) &&		\
					 (py < (ay + ah)) )

#ifdef _DEBUG // Debug routines
	#include <strsafe.h>

	SINLINE void __DBG_ErrorExit(TSTRING where)
	{
		LPVOID lpMsgBuf;
		LPVOID lpDisplayBuf;
		DWORD dw = GetLastError();

		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			dw,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpMsgBuf,
			0, NULL
		);

		// Display the error message and exit the process

		lpDisplayBuf = (LPVOID)LocalAlloc(
			LMEM_ZEROINIT,
			(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)where) + 64) * sizeof(TCHAR)
		);

		StringCchPrintf(
			(LPTSTR)lpDisplayBuf,
			(LocalSize(lpDisplayBuf) / sizeof(TCHAR)),
			TEXT("%s failed with error %d:\n\n%s\n\nClick \"OK\" to exit the application."),
			where, 
			dw, 
			lpMsgBuf
		);

		MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK | MB_ICONERROR);
		
		LocalFree(lpMsgBuf);
		LocalFree(lpDisplayBuf);
		ExitProcess(dw);
	}

	#define DBG_ErrorExit(where) __DBG_ErrorExit(where)
#else
	#define DBG_ErrorExit(where) MessageBox(NULL, where, TEXT("Error"), MB_OK | MB_ICONERROR)
#endif

typedef class __tagCClassicComponent		CClassicComponent;
typedef class __tagCClassicTextComponent	CClassicTextComponent;
typedef class __tagCClassicPanel			CClassicPanel;
typedef class __tagCClassicIcon				CClassicIcon;
typedef class __tagCClassicButton			CClassicButton;
typedef class __tagCClassicLabel			CClassicLabel;

enum WindowEdge
{
	WE_NOTHING = -1,
	WE_TOPBORDER,
	WE_TOPLEFT,
	WE_TOPRIGHT,
	WE_LEFTBORDER,
	WE_BOTTOMLEFT,
	WE_RIGHTBORDER,
	WE_BOTTOMRIGHT,
	WE_BOTTOMBORDER
};

SINLINE HFONT CreateSimpleFont(HWND hWnd,
								const TSTRING font_name, 
								const long font_size, 
								LONG font_weight = FW_NORMAL)
{
	HDC hdc = GetDC(hWnd);

	LOGFONT logfont = { 0 };
	logfont.lfHeight = -MulDiv(font_size, GetDeviceCaps(hdc, LOGPIXELSY), 72);
	logfont.lfWeight = font_weight;

	strcpy(logfont.lfFaceName, font_name);

	HFONT font = CreateFontIndirect(&logfont);

	ReleaseDC(hWnd, hdc);

	return font;
}

class CClassicWnd
{
	SINLINE LRESULT CALLBACK Internal_WndProc(HWND hWnd,
												UINT message,
												WPARAM wParam,
												LPARAM lParam)
	{
		CClassicWnd *window = (CClassicWnd *)GetWindowLong(hWnd, GWLP_USERDATA);

		if (!window)
			return DefWindowProc(hWnd, message, wParam, lParam);

		return window->WndProc(hWnd, message, wParam, lParam);
	}

	SINLINE LRESULT CALLBACK Internal_WndProc_Client(HWND hWnd,
														UINT message, 
														WPARAM wParam, 
														LPARAM lParam)
	{
		CClassicWnd *window = (CClassicWnd *)GetWindowLong(hWnd, GWLP_USERDATA);

		if (!window)
			return DefWindowProc(hWnd, message, wParam, lParam);

		return window->WndProc_Client(hWnd, message, wParam, lParam);
	}

	RECT						AREA_GetTitlebarBounds();
	RECT						AREA_GetCloseButtonBounds();
	RECT						AREA_GetMaximizeButtonBounds();
	RECT						AREA_GetMinimizeButtonBounds();

	public:
		CClassicWnd(HINSTANCE hInst, 
					TSTRING wndclass_name, 
					TSTRING wndclass_cl_name, 
					HICON icon = NULL, 
					HICON icon_small = NULL);

		~CClassicWnd();

		int						CreateAndShow(int xPos        = -1, 
												int yPos      = -1, 
												int width     = -1, 
												int height    = -1, 
												TSTRING title = NULL);

		void					Destroy();
		
		void					SetBounds(int xPos, 
											int yPos, 
											int width, 
											int height);

		void					SetPosition(int xPos, int yPos);
		void					SetPositionRelativeTo(RECT bounds);
		void					SetSize(int width, int height);

		void					SetTitle(TSTRING new_title);
		void					SetEnabled(bool enabled);
		void					SetClosable(bool closable);
		void					SetCloseButtonEnabled(bool enabled);
		void					SetResizable(bool resizable);
		void					SetMinimizable(bool minimizable);
		void					SetVisible(bool visible);

		void					SetBackgroundColor(DWORD color);
		void					SetForegroundColor(DWORD color);

		void					SetTitlebarFont(HFONT font);
		void					SetFont(HFONT font);

		void					RepaintWindow();
		void					RepaintClientArea();

		inline bool				IsEnabled()						{ return this->enabled; }
		inline bool				IsReady()						{ return ((this->hWnd) && (this->hWnd_client) /*&& (this->enabled)*/ && (this->visible)); }
		inline bool				IsClosable()					{ return this->closable; }
		inline bool				IsCloseButtonEnabled()			{ return this->close_button_enabled; }
		inline bool				IsResizable()					{ return this->resizable; }
		inline bool				IsMinimizable()					{ return this->minimizable; }
		inline bool				IsVisible()						{ return this->visible; }
		inline bool				IsActivated()					{ return this->activated; }
		inline bool				IsMaximized()					{ return this->maximized; }
		inline bool				IsMinimized()					{ return this->minimized; }

		RECT					GetWindowBounds();
		RECT					GetClientBounds();
		POINT					GetPosition();
		SIZE					GetWindowSize();
		SIZE					GetClientSize();

		TSTRING					GetTitle()						{ return this->__title; }

		DWORD					GetBackgroundColor()			{ return this->__background_color; }
		DWORD					GetForegroundColor()			{ return this->__foreground_color; }

		HFONT					GetTitlebarFont()				{ return this->font_titlebar; }
		HFONT					GetFont()						{ return this->font_element; }

		void					AddComponent(CClassicComponent *component);
		void					RemoveComponent(CClassicComponent *component);
		void					RemoveAll();

	protected:
		WNDCLASSEX				wnd_class, 
								wnd_class_client;

		HWND					hWnd, 
								hWnd_client;

		HFONT					font_titlebar,
								font_element;

		const HBRUSH			classic_default_brush			= CreateSolidBrush(CLASSIC_DEFAULT_BASECOLOR);

		//////////////////////////////
		//	CLIENT AREA PROPERTIES	//
		//////////////////////////////
		DWORD					__background_color				= CLASSIC_DEFAULT_BASECOLOR, 
								__foreground_color				= 0x000000;

		TSTRING					__title							= NULL;
		RECT					__bounds;

		int						prev_mx							= 0, 
								prev_my							= 0;

		// Various important flags...
		bool					enabled							= true, 
								activated						= false, 
								closable						= true, 
								close_button_enabled			= true, 
								resizable						= true, 
								minimizable						= true, 
								visible							= false, 
								drag_window						= false, 
								pressed_button_close			= false, 
								pressed_button_maximize			= false, 
								maximized						= false, 
								pressed_button_minimize			= false, 
								minimized						= false;

		WindowEdge				resize_edge						= WE_NOTHING;

		BasicList				*__components					= NULL;

		LRESULT CALLBACK		WndProc(HWND hWnd,
										UINT message,
										WPARAM wParam,
										LPARAM lParam);

		LRESULT CALLBACK		WndProc_Client(HWND hWnd, 
												UINT message, 
												WPARAM wParam, 
												LPARAM lParam);
};

//////////////////////////////////////
//		Component Main Class		//
//////////////////////////////////////
typedef void (*EVENTLISTENER)(CClassicComponent *, UINT, WPARAM, LPARAM);

class __tagCClassicComponent
{
	SINLINE LRESULT CALLBACK Internal_WndProc(HWND hWnd,
												UINT message,
												WPARAM wParam,
												LPARAM lParam)
	{
		CClassicComponent *comp = (CClassicComponent *)GetWindowLong(hWnd, GWLP_USERDATA);

		if (!comp)
			return DefWindowProc(hWnd, message, wParam, lParam);

		return comp->WndProc(hWnd, message, wParam, lParam);
	}

	public:
		EVENTLISTENER				event_listener = NULL;

		__tagCClassicComponent(HINSTANCE hInst, TSTRING name);
		~__tagCClassicComponent();

		void						PostComponentMessage(UINT msg, WPARAM wParam, LPARAM lParam);

		void						OnAdd(HWND parent);
		void						OnRemove(HWND parent);

		///////////////////////////
		// OVERRIDABLE FUNCTIONS //
		//         :-)           //
		///////////////////////////
		virtual void				OnCreate(void) = 0;
		virtual void				PaintComponent(LPDRAWCONTEXT context) = 0;

		virtual LRESULT CALLBACK	HandleMessage(HWND hWnd, 
													UINT message, 
													WPARAM wParam, 
													LPARAM lParam) = 0;

		void						SetBounds(int x, 
												int y, 
												int width, 
												int height);

		void						SetPosition(int x, int y);
		void						SetXPosition(int x);
		void						SetYPosition(int y);
		void						SetXPositionRelativeTo(RECT rect);
		void						SetYPositionRelativeTo(RECT rect);
		void						SetPositionRelativeTo(RECT rect);
		void						SetSize(int width, int height);

		void						SetFont(HFONT font);

		void						SetBackgroundColor(DWORD color);
		void						SetForegroundColor(DWORD color);

		RECT						GetBounds() { return this->bounds; }

		HFONT						GetFont() { return this->font; }

		DWORD						GetBackgroundColor() { return this->background_color; }
		DWORD						GetForegroundColor() { return this->foreground_color; }

		void						RepaintComponent();

		inline bool					IsReady() { return ((this->hWnd) && (this->visible)); }
		inline bool					IsFocused() { return (GetFocus() == this->hWnd); }

		LRESULT CALLBACK			WndProc(HWND hWnd, 
											UINT message, 
											WPARAM wParam, 
											LPARAM lParam);
		
	protected:
		WNDCLASSEX					wnd_class;
		HWND						hWnd;

		RECT						bounds;

		HFONT						font = NULL;;

		DWORD						background_color = CLASSIC_DEFAULT_BASECOLOR, 
									foreground_color = 0x000000;

		bool						enabled = false, 
									visible = true;
};

//////////////////////////////////////
//		Classic Text Component		//
//////////////////////////////////////
class __tagCClassicTextComponent : public CClassicComponent
{
	public:
		inline __tagCClassicTextComponent(HINSTANCE hInst, TSTRING name, TSTRING text = NULL)
						: CClassicComponent(hInst, name)
		{
			this->text = text;
		}

		inline ~__tagCClassicTextComponent() {}

		void				SetText(TSTRING string);
		inline TSTRING		GetText() { return this->text; }
	protected:
		TSTRING				text;
};

//////////////////////////////
//		Classic Panel		//
//////////////////////////////
class __tagCClassicPanel : public CClassicComponent
{
	public:
		inline __tagCClassicPanel(HINSTANCE hInst, TSTRING name)
								: CClassicComponent(hInst, name)
		{
		}

		inline ~__tagCClassicPanel() {}

		void				SetRaised(bool raised);
		
		inline bool			IsRaised() { return this->raised; }

		void				OnCreate(void);
		void				PaintComponent(LPDRAWCONTEXT context);

		LRESULT CALLBACK	HandleMessage(HWND hWnd,
											UINT message,
											WPARAM wParam,
											LPARAM lParam);

	protected:
		bool				raised = true;
};

//////////////////////////////
//		Classic Icon		//
//////////////////////////////
class __tagCClassicIcon : public CClassicComponent
{
	public:
		inline __tagCClassicIcon(HINSTANCE hInst, TSTRING name, HICON icon = NULL)
								: CClassicComponent(hInst, name)
		{
			this->icon = icon;
		}

		inline ~__tagCClassicIcon() {}

		void				SetIcon(HICON new_icon);
		HICON				GetIcon() { return this->icon; };

		void				OnCreate(void);
		void				PaintComponent(LPDRAWCONTEXT context);

		LRESULT CALLBACK	HandleMessage(HWND hWnd,
											UINT message,
											WPARAM wParam,
											LPARAM lParam);
	protected:
		HICON				icon;
};

//////////////////////////////
//		Classic Button		//
//////////////////////////////
class __tagCClassicButton : public CClassicTextComponent
{
	public:
		inline __tagCClassicButton(HINSTANCE hInst, TSTRING name, TSTRING text = NULL)
								: CClassicTextComponent(hInst, name, text)
		{
		}

		inline ~__tagCClassicButton() {}

		void				OnCreate(void);
		void				PaintComponent(LPDRAWCONTEXT context);

		LRESULT CALLBACK	HandleMessage(HWND hWnd,
											UINT message,
											WPARAM wParam,
											LPARAM lParam);

	protected:
		bool				pressed = false;
};

//////////////////////////////
//		Classic Label		//
//////////////////////////////
class __tagCClassicLabel : public CClassicTextComponent
{
	public:
		inline __tagCClassicLabel(HINSTANCE hInst, TSTRING name, TSTRING text = NULL)
								: CClassicTextComponent(hInst, name, text)
		{
		}

		inline ~__tagCClassicLabel() {}

		void				SetTextFormatFlags(UINT new_flags);

		void				OnCreate(void);
		void				PaintComponent(LPDRAWCONTEXT context);

		LRESULT CALLBACK	HandleMessage(HWND hWnd,
											UINT message,
											WPARAM wParam,
											LPARAM lParam);
	protected:
		UINT				format_flags = 0;
};

extern int MessageBoxClassic(HWND parent,
								HINSTANCE hInst,
								TSTRING message,
								TSTRING title,
								UINT flags);

#endif // _CCLASSICWND_H_