
#ifndef _WINDOWSCLASSIC_H_
#define _WINDOWSCLASSIC_H_

#pragma once

#include <Windows.h>

#include "resource.h"
#include "types.h"
#include "cdrawutils.h"
#include "basiclist.h"

#define CLASSIC_DEFAULT_BASECOLOR							(DWORD)0xC0C0C0

#define MAKESIZE(a, b)										(a - b)
#define MAKEWIDTH(rect)										MAKESIZE(rect.right, rect.left)
#define MAKEHEIGHT(rect)									MAKESIZE(rect.bottom, rect.top)

#define MAKECOORDINATE(a, b)								(a + b)

#define MAKERELATIVEPOSX(owner_bounds, target_bounds)		((target_bounds.right / 2) - (MAKEWIDTH(owner_bounds) / 2))
#define MAKERELATIVEPOSY(owner_bounds, target_bounds)		((target_bounds.bottom / 2) - (MAKEHEIGHT(owner_bounds) / 2))

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
	#define DBG_ErrorExit(where)	\
	{								\
		MessageBox(					\
			NULL,					\
			(LPCTSTR)where,			\
			TEXT("Error"),			\
			MB_OK | MB_ICONERROR	\
		);							\
									\
		ExitProcess(1);				\
	}
#endif

typedef class EXPORT __tagCClassicWnd				CClassicWnd,			*LPCClassicWnd;
typedef class EXPORT __tagCClassicComponent			CClassicComponent,		*LPCClassicComponent;
typedef class EXPORT __tagCClassicTextComponent		CClassicTextComponent,	*LPCClassicTextComponent;
typedef class EXPORT __tagCClassicPanel				CClassicPanel,			*LPCClassicPanel;
typedef class EXPORT __tagCClassicBitmap			CClassicBitmap,			*LPCClassicBitmap;
typedef class EXPORT __tagCClassicIcon				CClassicIcon,			*LPCClassicIcon;
typedef class EXPORT __tagCClassicButton			CClassicButton,			*LPCClassicButton;
typedef class EXPORT __tagCClassicLabel				CClassicLabel,			*LPCClassicLabel;
typedef class EXPORT __tagCClassicTextbox			CClassicTextbox,		*LPCClassicTextbox;

typedef void(*WINDOWLISTENER)(LPCClassicWnd, UINT, WPARAM, LPARAM);
typedef void(*EVENTLISTENER)(LPCClassicComponent, UINT, WPARAM, LPARAM);

enum EXPORT WindowEdge
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

SINLINE HFONT CreateSimpleFontFromDC(HDC hdc, 
									const STRING font_name, 
									const long font_size, 
									LONG font_weight = FW_NORMAL)
{
	LOGFONT logfont = { 0 };
	logfont.lfHeight = -MulDiv(font_size, GetDeviceCaps(hdc, LOGPIXELSY), 72);
	logfont.lfWeight = font_weight;

	lstrcpy(logfont.lfFaceName, font_name);

	HFONT font = CreateFontIndirect(&logfont);

	return font;
}

SINLINE HFONT CreateSimpleFontIndependent(const STRING font_name,
											const long font_size, 
											LONG font_weight = FW_NORMAL)
{
	HDC temp_dc = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL);
	HFONT font = CreateSimpleFontFromDC(temp_dc, font_name, font_size, font_weight);
	DeleteDC(temp_dc);

	return font;
}

SINLINE HFONT CreateSimpleFont(HWND hWnd,
								const STRING font_name, 
								const long font_size, 
								LONG font_weight = FW_NORMAL)
{
	HDC hdc = GetDC(hWnd);
	HFONT font = CreateSimpleFontFromDC(hdc, font_name, font_size, font_weight);
	ReleaseDC(hWnd, hdc);

	return font;
}

extern EXPORT SIZE GetFontMetrics(HFONT font, STRING text);
extern EXPORT STRING GenerateNewClassName(STRING prefix = TEXT("Classic"));

class __tagCClassicWnd
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

	////////////////////////////////////////////////////////////////////////
	/////// ! NOTE: THE CRAPPY MICROSOFT INTELLISENSE MAY COMPLAIN ! ///////
	/////// ! ABOUT NOT FINDING THE DEFINITION FOR THE FUNCTIONS   ! ///////
	/////// ! BELOW. BUT DON'T BELIEVE IT, BECAUSE IT'S LYING      ! ///////
	////////////////////////////////////////////////////////////////////////

	public:
		__tagCClassicWnd(HINSTANCE hInst, 
						STRING wndclass_name, 
						STRING wndclass_cl_name, 
						HICON icon = NULL, 
						HICON icon_small = NULL);

		~__tagCClassicWnd();

		int						CreateAndShow(int xPos        = -1, 
												int yPos      = -1, 
												int width     = -1, 
												int height    = -1, 
												STRING title  = NULL);

		void					Destroy();
		
		void					SetBounds(int xPos, 
											int yPos, 
											int width, 
											int height);

		void					SetPosition(int xPos, int yPos);
		void					SetPositionRelativeTo(RECT bounds);
		void					SetSize(int width, int height);

		void					SetTitle(STRING new_title);
		void					SetEnabled(bool enabled);
		void					SetClosable(bool closable);
		void					SetCloseButtonEnabled(bool enabled);
		void					SetResizable(bool resizable);
		void					SetMinimizable(bool minimizable);
		void					SetVisible(bool visible);

		void					SetTitlebarColorActiveLeft(DWORD color);
		void					SetTitlebarColorActiveRight(DWORD color);
		void					SetTitlebarColorInactiveLeft(DWORD color);
		void					SetTitlebarColorInactiveRight(DWORD color);

		void					SetBackgroundColor(DWORD color);
		void					SetForegroundColor(DWORD color);

		void					SetTitlebarFont(HFONT font);
		void					SetFont(HFONT font);

		void					RepaintWindow();
		void					RepaintClientArea();

		inline bool				IsEnabled()								{ return this->enabled; }
		inline bool				IsReady()								{ return ((this->hWnd) && (this->hWnd_client) /*&& (this->enabled)*/ && (this->visible)); }
		inline bool				IsClosable()							{ return this->closable; }
		inline bool				IsCloseButtonEnabled()					{ return this->close_button_enabled; }
		inline bool				IsResizable()							{ return this->resizable; }
		inline bool				IsMinimizable()							{ return this->minimizable; }
		inline bool				IsVisible()								{ return this->visible; }
		inline bool				IsActivated()							{ return this->activated; }
		inline bool				IsMaximized()							{ return this->maximized; }
		inline bool				IsMinimized()							{ return this->minimized; }

		RECT					GetWindowBounds();
		RECT					GetClientBounds();
		POINT					GetPosition();
		SIZE					GetWindowSize();
		SIZE					GetClientSize();

		STRING					GetTitle()								{ return this->__title; }

		DWORD					GetTitlebarColorActiveLeft()			{ return this->color_titlebar_active_l; }
		DWORD					GetTitlebarColorActiveRight()			{ return this->color_titlebar_active_r; }
		DWORD					GetTitlebarColorInactiveLeft()			{ return this->color_titlebar_inactive_l; }
		DWORD					GetTitlebarColorInactiveRight()			{ return this->color_titlebar_inactive_r; }

		DWORD					GetBackgroundColor()					{ return this->__background_color; }
		DWORD					GetForegroundColor()					{ return this->__foreground_color; }

		HFONT					GetTitlebarFont()						{ return this->font_titlebar; }
		HFONT					GetFont()								{ return this->font_element; }

		HDC						GetWindowDrawContext();
		HDC						GetDrawContext();

		void					ReleaseWindowDrawContext(HDC hdc);
		void					ReleaseDrawContext(HDC hdc);

		void					AddWindowListener(WINDOWLISTENER listener);
		void					RemoveWindowListener(WINDOWLISTENER listener);
		void					RemoveAllWindowListeners();

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

		DWORD					color_titlebar_active_l			= 0x800000, 
								color_titlebar_active_r			= 0xCC820C,
								color_titlebar_inactive_l		= 0x808080,
								color_titlebar_inactive_r		= CLASSIC_DEFAULT_BASECOLOR;

		const HBRUSH			classic_default_brush			= CreateSolidBrush(CLASSIC_DEFAULT_BASECOLOR);

		//////////////////////////////
		//	CLIENT AREA PROPERTIES	//
		//////////////////////////////
		DWORD					__background_color				= CLASSIC_DEFAULT_BASECOLOR, 
								__foreground_color				= 0x000000;

		STRING					__title							= NULL;
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

		BasicList				*__window_listeners				= NULL;
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

		__tagCClassicComponent(HINSTANCE hInst, STRING name);
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
		inline __tagCClassicTextComponent(HINSTANCE hInst, STRING name, STRING text = NULL)
						: CClassicComponent(hInst, name)
		{
			this->text = text;
		}

		inline ~__tagCClassicTextComponent() {}

		void				SetText(STRING string);
		inline STRING		GetText() { return this->text; }
	protected:
		STRING				text;
};

//////////////////////////////
//		Classic Panel		//
//////////////////////////////
class __tagCClassicPanel : public CClassicComponent
{
	public:
		inline __tagCClassicPanel(HINSTANCE hInst, STRING name)
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
//		Classic Bitmap		//
//////////////////////////////
class __tagCClassicBitmap : public CClassicComponent
{
	public:
		inline __tagCClassicBitmap(HINSTANCE hInst, STRING name, HBITMAP bitmap = NULL)
								: CClassicComponent(hInst, name)
		{
			this->bitmap = bitmap;
		}

		inline ~__tagCClassicBitmap() {}

		void				SetBitmap(HBITMAP new_bitmap);
		HBITMAP				GetBitmap() { return this->bitmap; };

		void				SetBitmapPosition(int x, int y);

		void				OnCreate(void);
		void				PaintComponent(LPDRAWCONTEXT context);

		LRESULT CALLBACK	HandleMessage(HWND hWnd,
											UINT message,
											WPARAM wParam,
											LPARAM lParam);
	protected:
		HBITMAP				bitmap;
		POINT				bitmap_pos;
};

//////////////////////////////
//		Classic Icon		//
//////////////////////////////
class __tagCClassicIcon : public CClassicComponent
{
	public:
		inline __tagCClassicIcon(HINSTANCE hInst, STRING name, HICON icon = NULL)
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
		inline __tagCClassicButton(HINSTANCE hInst, STRING name, STRING text = NULL)
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
		inline __tagCClassicLabel(HINSTANCE hInst, STRING name, STRING text = NULL)
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

//////////////////////////////
//		Classic Textbox		//
//////////////////////////////
class __tagCClassicTextbox : public CClassicTextComponent
{
	public:
		inline __tagCClassicTextbox(HINSTANCE hInst, STRING name, STRING text = NULL)
								: CClassicTextComponent(hInst, name, text)
		{
		}

		inline ~__tagCClassicTextbox() {}
		
		void				OnCreate(void);
		void				PaintComponent(LPDRAWCONTEXT context);

		LRESULT CALLBACK	HandleMessage(HWND hWnd,
											UINT message,
											WPARAM wParam,
											LPARAM lParam);
};

extern EXPORT int MessageBoxClassicA(HWND parent,
									TSTRING message,
									TSTRING title,
									UINT flags);

extern EXPORT int MessageBoxClassicW(HWND parent, 
									WSTRING message, 
									WSTRING title, 
									UINT flags);

// Checking for Unicode Support
#if (defined(UNICODE)) || (defined(_UNICODE))
	#define MessageBoxClassic(parent, message, title, flags) \
			MessageBoxClassicW(parent, message, title, flags)
#else
	#define MessageBoxClassic(parent, message, title, flags) \
			MessageBoxClassicA(parent, message, title, flags)
#endif

//////////////////////////////////////////////////////////////////
// Okay so i've decided to make the popup menu API structure	//
// similar to Windows'. I know it might be stupid not to use	//
// Classes like as i did for the other GUI Elements, but		//
// i think it's quite okay. After all, the goal here is to		//
// sort of re-create a part of the old Win32 API. Plus			//
// there is really not that much to those little Popup Menus.	//
// They're very straightforward.								//
//////////////////////////////////////////////////////////////////

typedef struct CLASSICPOPUP					*HPOPUP;
typedef struct CLASSICMENUITEM				*HMENUITEM;

typedef struct CPM_ITEM_EXTRADATA_TEXT		*LPCPM_ITEM_EXTRADATA_TEXT;

typedef struct __tagCPM_ITEMINFO
{
	HMENUITEM menu_item;
	int menu_index;
} CPM_ITEMINFO, *LPCPM_ITEMINFO;

// ClassicPopupMenu
#define CPM_CREATE							0x00
#define CPM_SHOW							0x01
#define CPM_ITEMSELECTED					0x02
#define CPM_DESTROY							0x03

#define CPM_ITEM_TYPEMASK					0x0000000FL
#define CPM_ITEM_TEXT						0x00000000L
// TODO(toni): Not implemented yet
#define CPM_ITEM_ICONTEXT					0x00000001L
#define CPM_ITEM_SEPARATOR					0x00000002L

// This flag defines whether a Menu item is the default option or not.
#define CPM_ITEM_DEFAULT					0x000000F0L

#define CPM_ITEM_STATEMASK					0xF0000000L
#define CPM_ITEM_STATE_SELECTED				0x10000000L
// Is the menu item disabled or not?
#define CPM_ITEM_STATE_DISABLED				0x20000000L

typedef void(*CLASSIC_MENU_PROC)			(HPOPUP, UINT, LPVOID);

extern EXPORT HPOPUP		CreatePopupMenuClassic(HINSTANCE hInst, CLASSIC_MENU_PROC proc);
extern EXPORT bool			ShowPopupMenuClassic(HPOPUP popup);
extern EXPORT HMENUITEM		CreateMenuItem(HPOPUP parent, UINT style, LPVOID param);
extern EXPORT void			AppendMenuItemFlags(HMENUITEM item, UINT flags);
extern EXPORT void			RemoveMenuItemFlags(HMENUITEM item, UINT flags);
extern EXPORT void			SetMenuItemUserdata(HMENUITEM item, LPVOID data);
extern EXPORT LPVOID		GetMenuItemUserdata(HMENUITEM item);
/*
	This function returns the first found Menu item that has been flagged as "Default"
	Returns "NULL" when there is no default item.
*/
extern EXPORT HMENUITEM		GetDefaultMenuItem(HPOPUP popup);
extern EXPORT void			AppendMenuItemClassic(HPOPUP popup, HMENUITEM item);
extern EXPORT HMENUITEM		CreateAndAppendMenuItemClassic(HPOPUP parent, UINT style, LPVOID param);
extern EXPORT void			InsertMenuItemClassic(HPOPUP popup, HMENUITEM item, int index);
extern EXPORT HMENUITEM		CreateAndInsertMenuItemClassic(HPOPUP parent, UINT style, LPVOID param, int index);
extern EXPORT void			RemoveMenuItemClassic(HPOPUP popup, HMENUITEM item);
extern EXPORT void			RemoveMenuitemClassicByIndex(HPOPUP popup, int index);
extern EXPORT void			DestroyMenuItem(HMENUITEM item);
extern EXPORT void			DestroyPopupMenuClassic(HPOPUP popup);

#endif // _WINDOWSCLASSIC_H_