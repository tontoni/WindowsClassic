
#include "windowsclassic.h"
#include "utils.h"

struct CLASSICPOPUP
{
	SIZE					menu_size;

	LPBasicList				menu_items;

	WNDCLASS				menu_wndclass;
	HWND					menu_wnd;

	HFONT					menu_item_font, 
							menu_item_font_default;

	DWORD					worker_thread_id;
	HANDLE					worker_thread_handle;

	CLASSIC_MENU_PROC		menu_proc;
};

struct CLASSICMENUITEM
{
	SIZE					item_size;

	UINT					item_style;

	LPVOID					item_extradata;
	LPVOID					userdata;
};

struct CPM_ITEM_EXTRADATA_TEXT
{
	TSTRING					item_text;
};

static DWORD WINAPI PopupWorker(LPVOID param);

static LRESULT CALLBACK Internal_WndProc(HWND hWnd,
										UINT message,
										WPARAM wParam,
										LPARAM lParam);

HPOPUP CreatePopupMenuClassic(HINSTANCE hInst, CLASSIC_MENU_PROC proc)
{
	HPOPUP popup = (HPOPUP)malloc(sizeof(CLASSICPOPUP));
	memset(popup, 0, sizeof(CLASSICPOPUP));

	popup->menu_proc = proc;
	popup->menu_items = List_Create(64);

	memset(&popup->menu_wndclass, 0, sizeof(WNDCLASS));
	popup->menu_wndclass.hInstance = hInst;
	
	// We have to create these two little font handles
	// here, or otherwise some font metrics will be screwed up!
	popup->menu_item_font =
		CreateSimpleFontIndependent("MS Sans Serif", 8);

	popup->menu_item_font_default =
		CreateSimpleFontIndependent("MS Sans Serif", 8, FW_BOLD);

	return popup;
}

bool ShowPopupMenuClassic(HPOPUP popup)
{
	popup->worker_thread_handle = CreateThread(
		NULL,
		0,
		PopupWorker,
		popup,
		0,
		&popup->worker_thread_id
	);
	
	return true;
}

HMENUITEM CreateMenuItem(HPOPUP parent, UINT style, LPVOID param)
{
	HMENUITEM item = (HMENUITEM)malloc(sizeof(CLASSICMENUITEM));
	memset(item, 0, sizeof(CLASSICMENUITEM));

	item->item_style = style;

	switch (style & CPM_ITEM_TYPEMASK)
	{
		case CPM_ITEM_ICONTEXT:
		{
			// TODO(toni): Implement!
		} break;
		case CPM_ITEM_TEXT:
		{
			LPCPM_ITEM_EXTRADATA_TEXT text_data = (LPCPM_ITEM_EXTRADATA_TEXT)malloc(sizeof(CPM_ITEM_EXTRADATA_TEXT));
			
			// Yes param is expected to be a string here
			text_data->item_text = (TSTRING)param;
			item->item_extradata = text_data;

			// Temporary DC - for computing the text size...
			HDC temp_dc = CreateDC("DISPLAY", NULL, NULL, NULL);

			if (parent)
			{
				SelectObject(
					temp_dc, 
					(style & CPM_ITEM_DEFAULT) ?
					parent->menu_item_font_default : 
					parent->menu_item_font
				);
			}

			SIZE text_size;

			GetTextExtentPoint32(
				temp_dc, 
				text_data->item_text, 
				StrLenA(text_data->item_text), 
				&text_size
			);

			DeleteDC(temp_dc);

			// Border Width (3 + 3) + Icon Space (16 x 16)
			item->item_size.cx = text_size.cx + 22;
			// Adding a few extra pixels to make it look a bit better
			item->item_size.cy = text_size.cy + 7;
		} break;
		case CPM_ITEM_SEPARATOR:
		{
			item->item_size.cx = 100;
			item->item_size.cy = 9;
		} break;
		default:
			break;
	}

	return item;
}

void AppendMenuItemFlags(HMENUITEM item, UINT flags)
{
	item->item_style |= flags;
}

void RemoveMenuItemFlags(HMENUITEM item, UINT flags)
{
	item->item_style &= ~flags;
}

void SetMenuItemUserdata(HMENUITEM item, LPVOID data)
{
	item->userdata = data;
}

LPVOID GetMenuItemUserdata(HMENUITEM item)
{
	return item->userdata;
}

HMENUITEM GetDefaultMenuItem(HPOPUP popup)
{
	for (int i = 0; 
		i < List_GetCount(popup->menu_items); 
		++i)
	{
		HMENUITEM item = (HMENUITEM)List_Get(popup->menu_items, i);

		if (item->item_style & CPM_ITEM_DEFAULT)
			return item;
	}

	return NULL;
}

void AppendMenuItemClassic(HPOPUP popup, HMENUITEM item)
{
	List_Add(popup->menu_items, item);
}

HMENUITEM CreateAndAppendMenuItemClassic(HPOPUP parent, UINT style, LPVOID param)
{
	HMENUITEM item = CreateMenuItem(parent, style, param);
	AppendMenuItemClassic(parent, item);
	return item;
}

void InsertMenuItemClassic(HPOPUP popup, HMENUITEM item, int index)
{
	List_Set(popup->menu_items, index, item);
}

HMENUITEM CreateAndInsertMenuItemClassic(HPOPUP parent, UINT style, LPVOID param, int index)
{
	HMENUITEM item = CreateMenuItem(parent, style, param);
	InsertMenuItemClassic(parent, item, index);
	return item;
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
	if (item->item_extradata)
		free(item->item_extradata);

	if (item->userdata)
		free(item->userdata);
	
	free(item);
}

void DestroyPopupMenuClassic(HPOPUP popup)
{
	DestroyWindow(popup->menu_wnd);
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
	popup_w += 19; // Suffix Icon Size (16 x 16) + Border Width (3)
	popup_h += 7; // Menu Item Extra pixels from bottom + 3 pixels for border

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

	popup->menu_size.cx = popup_w;
	popup->menu_size.cy = popup_h;

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

	DeleteObject(popup->menu_item_font);
	DeleteObject(popup->menu_item_font_default);

	// Unregister our Window class since we won't ever use it anymore
	UnregisterClass(
		popup->menu_wndclass.lpszClassName, 
		popup->menu_wndclass.hInstance
	);

	CloseHandle(popup->worker_thread_handle);
	free(popup); // Freeing the Popup handle

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
				popup->menu_size.cx, 
				popup->menu_size.cy,
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
						LPCPM_ITEM_EXTRADATA_TEXT text_data = (LPCPM_ITEM_EXTRADATA_TEXT)item->item_extradata;
						
						if (!text_data->item_text)
							break;

						int text_x = xPos + 16,
							text_y = yPos + 3;

						// Selecting the correct font to draw
						SelectObject(
							hdc,
							(item->item_style & CPM_ITEM_DEFAULT) ?
							popup->menu_item_font_default :
							popup->menu_item_font
						);

						if ((item->item_style & CPM_ITEM_STATEMASK) == CPM_ITEM_STATE_SELECTED)
						{
							context.fill_color = 0x800000;
							context.draw_color = 0xFFFFFF;

							CDrawUtils::FillSolidRectangle(
								&context, 
								xPos, 
								yPos, 
								popup->menu_size.cx - 6,
								item->item_size.cy
							);
						}
						else if ((item->item_style & CPM_ITEM_STATEMASK) == CPM_ITEM_STATE_DISABLED)
						{
							context.fill_color = CLASSIC_DEFAULT_BASECOLOR;
							context.draw_color = 0xFFFFFF;

							CDrawUtils::DrawString(
								&context, 
								text_data->item_text, 
								text_x + 1, 
								text_y + 1
							);

							context.draw_color = 0x848284;
						}
						else
						{
							context.fill_color = CLASSIC_DEFAULT_BASECOLOR;
							context.draw_color = 0x000000;
						}

						SetBkMode(hdc, TRANSPARENT);

						CDrawUtils::DrawString(
							&context, 
							text_data->item_text, 
							text_x, 
							text_y
						);
						
						SetBkMode(hdc, OPAQUE);
					} break;
					case CPM_ITEM_SEPARATOR:
					{
						context.fill_color = 0x848284;
						CDrawUtils::FillSolidRectangle(&context, xPos + 1, yPos + 3, popup->menu_size.cx - 8, 1);
						
						context.fill_color = 0xFFFFFF;
						CDrawUtils::FillSolidRectangle(&context, xPos + 1, yPos + 4, popup->menu_size.cx - 8, 1);
					} break;
					default:
						break; // We should never even get here
				}

				yPos += item->item_size.cy;
			}

			EndPaint(hWnd, &paintstruct);
		} break;
		case WM_MOUSEMOVE:
		{
			int mx = (int)((short)LOWORD(lParam)),
				my = (int)((short)HIWORD(lParam));

			// Initial item positions
			int xPos = 3,
				yPos = 3;

			bool flags_changed = false;
			
			RECT rect_deselected = { 0, 0, 0, 0 },
				 rect_selected   = { 0, 0, 0, 0 };

			for (int i = 0; 
				i < List_GetCount(popup->menu_items); 
				++i)
			{
				HMENUITEM item = (HMENUITEM)List_Get(popup->menu_items, i);

				// Skipping Separators and disabled items...
				if (((item->item_style & CPM_ITEM_TYPEMASK) == CPM_ITEM_SEPARATOR) || 
					((item->item_style & CPM_ITEM_STATEMASK) == CPM_ITEM_STATE_DISABLED))
				{
					yPos += item->item_size.cy;
					continue;
				}

				if ((item->item_style & CPM_ITEM_STATEMASK) == CPM_ITEM_STATE_SELECTED)
				{
					// Remember: Border is 3 pixels, and width always means "double the value dude!"
					if (!IsPointInArea(mx, my, xPos, yPos, popup->menu_size.cx - 6, item->item_size.cy))
					{
						// If previous items are still selected, 
						// even though the mouse isn't over it anymore, 
						// just deselect them
						item->item_style &= ~CPM_ITEM_STATE_SELECTED;

						// Marking the area that has to be redrawn
						//
						// NOTE(toni): There only should ever be exactly 
						// 1 menu item selected at the same time.
						// So we only have to care about this one area.
						rect_deselected.left = xPos;
						rect_deselected.top = yPos;
						rect_deselected.right = MAKECOORDINATE(xPos, popup->menu_size.cx - 6);
						rect_deselected.bottom = MAKECOORDINATE(yPos, item->item_size.cy);

						flags_changed = true;
					}
				}
				else
				{
					if (IsPointInArea(mx, my, xPos, yPos, popup->menu_size.cx - 6, item->item_size.cy))
					{
						// Select the current item
						item->item_style |= CPM_ITEM_STATE_SELECTED;
						
						// Marking the area that has to be redrawn
						rect_selected.left = xPos;
						rect_selected.top = yPos;
						rect_selected.right = MAKECOORDINATE(xPos, popup->menu_size.cx - 6);
						rect_selected.bottom = MAKECOORDINATE(yPos, item->item_size.cy);

						flags_changed = true;
					}
				}

				yPos += item->item_size.cy;
			}

			if (flags_changed)
			{
				if ((rect_deselected.left > 0) && 
					(rect_deselected.top > 0) && 
					(rect_deselected.right > 0) && 
					(rect_deselected.bottom > 0))
					RedrawWindow(hWnd, &rect_deselected, NULL, RDW_INVALIDATE);
				
				if ((rect_selected.left > 0) &&
					(rect_selected.top > 0) &&
					(rect_selected.right > 0) &&
					(rect_selected.bottom > 0))
					RedrawWindow(hWnd, &rect_selected, NULL, RDW_INVALIDATE);
			}
		} break;
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		{
			for (int i = 0;
				i < List_GetCount(popup->menu_items);
				++i)
			{
				HMENUITEM item = (HMENUITEM)List_Get(popup->menu_items, i);

				if ((item->item_style & CPM_ITEM_STATEMASK) == CPM_ITEM_STATE_SELECTED)
				{
					// Notify the menu procedure (if there is one)
					// that the user clicked on a menu item!
					if (popup->menu_proc)
					{
						CPM_ITEMINFO info = {
							item,
							i
						};
						
						popup->menu_proc(popup, CPM_ITEMSELECTED, &info);
					}

					DestroyWindow(hWnd);
					break;
				}
			}
		} break;
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

			// Destroying all the Menu items
			// we don't need them anymore after this popup menu has been closed.
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