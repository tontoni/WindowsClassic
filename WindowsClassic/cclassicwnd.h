
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

typedef class __tagCClassicComponent	CClassicComponent;
typedef class __tagCClassicPanel		CClassicPanel;
typedef class __tagCClassicButton		CClassicButton;

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

SINLINE bool IsPointInArea(int px,
							int py,
							int ax,
							int ay,
							int aw,
							int ah)
{
	return (
		(px >= ax) &&
		(py >= ay) &&
		(px < (ax + aw)) &&
		(py < (ay + ah))
	);
}

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
		CClassicWnd(HINSTANCE hInst, HICON icon = NULL, HICON icon_small = NULL);
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
		void					SetClosable(bool closable);
		void					SetResizable(bool resizable);
		void					SetMinimizable(bool minimizable);
		void					SetVisible(bool visible);

		void					SetBackgroundColor(DWORD color);
		void					SetForegroundColor(DWORD color);

		void					SetTitlebarFont(HFONT font);
		void					SetFont(HFONT font);

		void					RepaintWindow();
		void					RepaintClientArea();

		inline bool				IsReady()						{ return ((this->hWnd) && (this->hWnd_client) && (this->visible)); }
		inline bool				IsClosable()					{ return this->closable; }
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
		bool					activated						= false, 
								closable						= true, 
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

		void						OnAdd(HWND parent);
		void						OnRemove(HWND parent);

		///////////////////////////
		// OVERRIDABLE FUNCTIONS //
		//         :-)           //
		///////////////////////////
		virtual void				OnCreate(void) = 0;
		virtual void				PaintComponent(DRAWCONTEXT *context) = 0;

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

//////////////////////////////
//		Classic Panel		//
//////////////////////////////
class __tagCClassicPanel : public CClassicComponent
{
	public:
		inline __tagCClassicPanel(HINSTANCE hInst)
								: CClassicComponent(hInst, "ClassicPanel")
		{
		}

		inline ~__tagCClassicPanel() {}

		void				SetRaised(bool raised);
		
		inline bool			IsRaised() { return this->raised; }

		void				OnCreate(void);
		void				PaintComponent(DRAWCONTEXT *context);

		LRESULT CALLBACK	HandleMessage(HWND hWnd,
											UINT message,
											WPARAM wParam,
											LPARAM lParam);

	protected:
		bool				raised = true;
};

//////////////////////////////
//		Classic Button		//
//////////////////////////////
class __tagCClassicButton : public CClassicComponent
{
	public:
		inline __tagCClassicButton(HINSTANCE hInst, TSTRING text = NULL)
								: CClassicComponent(hInst, "ClassicButton")
		{
			this->button_text = text;
		}

		inline ~__tagCClassicButton() {}

		void				SetText(TSTRING string);

		inline TSTRING		GetText() { return this->button_text; }

		void				OnCreate(void);
		void				PaintComponent(DRAWCONTEXT *context);

		LRESULT CALLBACK	HandleMessage(HWND hWnd,
											UINT message,
											WPARAM wParam,
											LPARAM lParam);

	protected:
		TSTRING				button_text;
		bool				pressed = false;
};

SINLINE UINT MessageBoxClassic(HWND parent,
								HINSTANCE hInst, 
								TSTRING message, 
								TSTRING title, 
								HICON icon = NULL)
{
	CClassicWnd *window = new CClassicWnd(hInst, NULL, NULL);

	window->SetTitle(title);
	
	window->SetMinimizable(false);
	window->SetResizable(false);

	// Set the size first!
	// Currently the size is only hardcoded, but i wanna make it dynamic eventually!
	window->SetSize(223, 118);

	RECT parent_bounds;

	// If parent is NULL, just use the Desktop window
	GetWindowRect((parent) ? parent : GetDesktopWindow(), &parent_bounds);
	window->SetPositionRelativeTo(parent_bounds);

	// Here i'll probably make different message box types for different purposes i the future!
	// ( Yes No - Yes No Cancel - OK Cancel - OK - etc... )
	// But for now this message box will only contain the infamous little "OK" button
	CClassicButton *button_ok = new CClassicButton(hInst, "OK");
	button_ok->SetSize(75, 23);

	RECT client_bounds = window->GetClientBounds();

	button_ok->SetXPositionRelativeTo(client_bounds);
	button_ok->SetYPosition(client_bounds.bottom - 36);

	window->AddComponent(button_ok);

	// TODO(toni): In the future this function should return the users decision(s)!
	return (UINT)window->CreateAndShow();
}

#endif // _CCLASSICWND_H_