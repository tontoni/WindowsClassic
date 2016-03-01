
#include "cclassicwnd.h"

struct CLASSICPOPUP
{
	RECT					bounds;

	LPBasicList				menu_items;

	WNDCLASS				menu_wndclass;
	HWND					menu_wnd;

	HFONT					menu_item_font, 
							menu_item_font_selected;

	CLASSIC_MENU_PROC		menu_proc;
};

struct CLASSICMENUITEM
{
	SIZE					item_size;

	UINT					item_style;

	LPVOID					item_extradata;
	LPVOID					userdata;
};

static DWORD WINAPI PopupWorker(LPVOID param);

static LRESULT CALLBACK Internal_WndProc(HWND hWnd,
										UINT message,
										WPARAM wParam,
										LPARAM lParam);

HPOPUP CreatePopupMenuClassic(HINSTANCE hInst)
{
	HPOPUP popup = (HPOPUP)malloc(sizeof(CLASSICPOPUP));
	memset(popup, 0, sizeof(CLASSICPOPUP));

	popup->menu_items = List_Create(64);

	memset(&popup->menu_wndclass, 0, sizeof(WNDCLASS));
	popup->menu_wndclass.hInstance = hInst;

	popup->menu_item_font = 
		CreateSimpleFontIndependent("MS Sans Serif", 8);
	
	popup->menu_item_font_selected = 
		CreateSimpleFontIndependent("MS Sans Serif", 8, FW_BOLD);
	
	return popup;
}

bool ShowPopupMenuClassic(HPOPUP popup)
{
	DWORD worker_thread_id;
	HANDLE worker_thread_handle;

	worker_thread_handle = CreateThread(
		NULL,
		0,
		PopupWorker,
		popup,
		0,
		&worker_thread_id
	);

	WaitForSingleObject(worker_thread_handle, INFINITE);

	CloseHandle(worker_thread_handle);
	free(popup);

	return true;
}

HMENUITEM CreateMenuItem(UINT style, LPVOID param)
{
	HMENUITEM item = (HMENUITEM)malloc(sizeof(CLASSICMENUITEM));
	memset(item, 0, sizeof(CLASSICMENUITEM));

	item->item_style = style;

	switch (style & CPM_ITEM_TYPEMASK)
	{
		case CPM_ITEM_TEXT:
		{
			item->item_extradata = param;

			// Test size
			item->item_size.cx = 100;
			item->item_size.cy = 17;
		} break;
		case CPM_ITEM_SEPARATOR:
		{
		} break;
		default:
			break;
	}

	return item;
}

void SetMenuItemUserdata(HMENUITEM item, LPVOID data)
{
	item->userdata = data;
}

LPVOID GetMenuItemUserdata(HMENUITEM item)
{
	return item->userdata;
}

void AppendMenuItemClassic(HPOPUP popup, HMENUITEM item)
{
	List_Add(popup->menu_items, item);
}

void InsertMenuItemClassic(HPOPUP popup, HMENUITEM item, int index)
{
	List_Set(popup->menu_items, index, item);
}

void RemoveMenuItemClassic(HPOPUP popup, HMENUITEM item)
{
	List_Remove(popup->menu_items, item);
}

void RemoveMenuitemClassicByIndex(HPOPUP popup, int index)
{
	List_RemoveByIndex(popup->menu_items, index);
}

void DestroyMenuItem(HMENUITEM item)
{
	free(item->userdata);
	free(item);
}

DWORD WINAPI PopupWorker(LPVOID param)
{
	HPOPUP popup = (HPOPUP)param;

	if (!List_GetCount(popup->menu_items))
	{
		// If there have no menu items been inserted yet 
		// just fail the call and close everything immediately
		DBG_ErrorExit("Popup Menu creation - No menu items!");
		return 1;
	}

	popup->menu_wndclass.style			= (CS_HREDRAW | CS_VREDRAW);
	popup->menu_wndclass.lpfnWndProc	= Internal_WndProc;
	popup->menu_wndclass.hCursor		= LoadCursor(NULL, IDC_ARROW);
	popup->menu_wndclass.lpszClassName	= GenerateNewClassName("Classic_Popup");

	if (!RegisterClass(&popup->menu_wndclass))
	{
		DBG_ErrorExit("Popup Menu creation (In Worker thread)");
		return 1;
	}

	POINT mpos;

	if (!GetCursorPos(&mpos))
	{
		DBG_ErrorExit("Popup Menu creation - Unable to retrieve cursor positon!");
		return 1;
	}

	int popup_w = 0,
		popup_h = 0;

	for (int i = 0; 
		i < List_GetCount(popup->menu_items); 
		++i)
	{
		HMENUITEM item = (HMENUITEM)List_Get(popup->menu_items, i);

		// Compute the absolute size of our popup menu
		if (popup_w < item->item_size.cx)
			popup_w = item->item_size.cx;

		popup_h += item->item_size.cy;
	}

	// Adding some extra pixels
	// Think about the border too!
	popup_w += 3;
	popup_h += 3;

	popup->menu_wnd = CreateWindowEx(
		WS_EX_TOOLWINDOW, 
		popup->menu_wndclass.lpszClassName, 
		NULL, 
		WS_POPUP, 
		mpos.x, 
		mpos.y, 
		popup_w,
		popup_h, 
		NULL, 
		NULL, 
		popup->menu_wndclass.hInstance, 
		NULL
	);

	if (!popup->menu_wnd)
	{
		DBG_ErrorExit("Popup Menu HWND creation (In Worker thread)");
		return 1;
	}

	// Gotta give the WndProc access to our popup struct
	SetWindowLong(popup->menu_wnd, GWLP_USERDATA, (long)popup);

	if (popup->menu_proc)
		popup->menu_proc(popup, CPM_CREATE, NULL);

	ShowWindow(popup->menu_wnd, SW_SHOW);
	UpdateWindow(popup->menu_wnd);

	if (popup->menu_proc)
		popup->menu_proc(popup, CPM_SHOW, NULL);

	MSG msg;

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// Unregister our Window class since we won't ever use it anymore
	UnregisterClass(
		popup->menu_wndclass.lpszClassName, 
		popup->menu_wndclass.hInstance
	);

	return 0;
}

LRESULT CALLBACK Internal_WndProc(HWND hWnd,
									UINT message,
									WPARAM wParam,
									LPARAM lParam)
{
	HPOPUP popup = (HPOPUP)GetWindowLong(hWnd, GWLP_USERDATA);

	switch (message)
	{
		case WM_PAINT:
		{
			HDC hdc;
			PAINTSTRUCT paintstruct;

			hdc = BeginPaint(hWnd, &paintstruct);

			DRAWCONTEXT context;
			context.paintstruct = paintstruct;
			context.fill_color = CLASSIC_DEFAULT_BASECOLOR;
			context.draw_color = 0x000000;

			// NOTE(toni): We can just straight up use the "right" and "bottom"
			// variables because the x and y position are always gonna be zero anyways.
			CDrawUtils::FillRectangle3D(
				&context, 
				0, 
				0, 
				paintstruct.rcPaint.right, 
				paintstruct.rcPaint.bottom, 
				RECT_RAISED
			);

			// Item positions
			int xPos = 3,
				yPos = 3;

			for (int i = 0; 
				i < List_GetCount(popup->menu_items); 
				++i)
			{
				HMENUITEM item = (HMENUITEM)List_Get(popup->menu_items, i);

				switch (item->item_style & CPM_ITEM_TYPEMASK)
				{
					case CPM_ITEM_ICONTEXT:
					{
						// TODO(toni): Implement!
					}
					case CPM_ITEM_TEXT:
					{
						if (item->item_extradata)
						{
							TSTRING label = (TSTRING)item->item_extradata;

							// TODO(toni): Continue here!

							SelectObject(hdc, popup->menu_item_font);
							CDrawUtils::DrawString(&context, label, xPos + 16, yPos);
						}
					} break;
					case CPM_ITEM_SEPARATOR:
					{

					} break;
					default:
						break; // We should never even get here
				}

				yPos += item->item_size.cy;
			}

			EndPaint(hWnd, &paintstruct);
		} break;
		case WM_LBUTTONUP:
		{
			if (popup->menu_proc)
				popup->menu_proc(popup, CPM_SELECTITEM, NULL);
		}
		case WM_ACTIVATE:
		{
			// If our little popup menu gets thrown into the background
			// just destroy it...
			if (!wParam)
				DestroyWindow(hWnd);
		} break;
		case WM_DESTROY:
		{
			// Before we deallocate, we have to notify the Menu Procedure.
			// Because maybe the user wants to access the Menu items
			// before they're gone.
			// (if there even is one)
			if (popup->menu_proc)
				popup->menu_proc(popup, CPM_DESTROY, NULL);

			for (int i = 0;
				i < List_GetCount(popup->menu_items);
				++i)
			{
				HMENUITEM item = (HMENUITEM)List_Get(popup->menu_items, i);
				DestroyMenuItem(item);
			}

			PostQuitMessage(0);
		} break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}