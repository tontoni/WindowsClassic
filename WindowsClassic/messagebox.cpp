
/*
 * Message Box API
 */

#include "cclassicwnd.h"
#include "utils.h"
#include "resource.h"

typedef struct __tagMSGBOXINFO
{
	HWND			parent;
	HINSTANCE		hInstance;
	TSTRING			message, 
					title;

	DWORD			creation_flags;

	DWORD			thread_id;
	HANDLE			thread_handle;

	int				result;
} MSGBOXINFO, *LPMSGBOXINFO;

static void MessageBoxEventListener(CClassicComponent *source, 
									UINT message, 
									WPARAM wParam, 
									LPARAM lParam);

static DWORD WINAPI MessageBoxWorker(LPVOID param);

int MessageBoxClassicA(HWND parent,
						HINSTANCE hInst,
						TSTRING message,
						TSTRING title,
						DWORD flags)
{
	DWORD thread_id;
	HANDLE thread_handle;

	LPMSGBOXINFO info		= (LPMSGBOXINFO)malloc(sizeof(MSGBOXINFO));

	// Zero memory
	memset(info, 0, sizeof(MSGBOXINFO));

	info->parent			= parent;
	info->hInstance			= hInst;
	info->message			= message;
	info->title				= title;
	info->creation_flags	= flags;
	
	thread_handle			= CreateThread(NULL, 0, MessageBoxWorker, info, 0, &thread_id);

	info->thread_id			= thread_id;
	info->thread_handle		= thread_handle;

	// We gotta wait until the thread terminates
	WaitForSingleObject(thread_handle, INFINITE);
	CloseHandle(thread_handle);

	int result = info->result;

	free(info);

	return result;
}

HICON CheckAndLoadIcon(LPMSGBOXINFO info)
{
	switch (info->creation_flags & MB_ICONMASK)
	{
		// Error Icon
		case MB_ICONHAND:
		{
			return LoadIcon(info->hInstance, MAKEINTRESOURCE(IDI_ERROR_CLASSIC));
		}
		// Question Icon
		case MB_ICONQUESTION:
		{
			return LoadIcon(info->hInstance, MAKEINTRESOURCE(IDI_QUESTION_CLASSIC));
		}
		// Warning Icon
		case MB_ICONEXCLAMATION:
		{
			return LoadIcon(info->hInstance, MAKEINTRESOURCE(IDI_WARNING_CLASSIC));
		}
		// Info Icon
		case MB_ICONASTERISK:
		{
			return LoadIcon(info->hInstance, MAKEINTRESOURCE(IDI_INFO_CLASSIC));
		}
		default:
		{
			break;
		}
	}

	return (HICON)NULL; // NULL = No icon
}

bool HandleMsgBoxTypes(LPMSGBOXINFO info, 
						LPCClassicWnd window)
{
	RECT client_bounds = window->GetClientBounds();

	switch (info->creation_flags & MB_TYPEMASK)
	{
		// Adding the different message box buttons
		case MB_OK:
		{
			CClassicButton *button_ok = new CClassicButton(
				info->hInstance,
				GenerateNewClassName(),
				"OK"
			);

			button_ok->SetSize(75, 23);

			button_ok->SetXPositionRelativeTo(client_bounds);
			button_ok->SetYPosition(client_bounds.bottom - 34);

			button_ok->event_listener = MessageBoxEventListener;

			window->AddComponent(button_ok);

			return true;
		}
		case MB_OKCANCEL:
		{
			int center_x = (client_bounds.right / 2);

			CClassicButton *button_cancel = new CClassicButton(
				info->hInstance,
				GenerateNewClassName(),
				"Cancel"
			);

			button_cancel->SetSize(75, 23);

			button_cancel->SetXPosition(center_x + 3);
			button_cancel->SetYPosition(client_bounds.bottom - 34);

			button_cancel->event_listener = MessageBoxEventListener;

			window->AddComponent(button_cancel);

			CClassicButton *button_ok = new CClassicButton(
				info->hInstance,
				GenerateNewClassName(),
				"OK"
			);

			button_ok->SetSize(75, 23);

			button_ok->SetXPosition(center_x - 78);
			button_ok->SetYPosition(client_bounds.bottom - 34);

			button_ok->event_listener = MessageBoxEventListener;

			window->AddComponent(button_ok);
			
			return true;
		}
		case MB_ABORTRETRYIGNORE:
		{
			// TODO(toni): Handle this type!
			return true;
		}
		case MB_YESNOCANCEL:
		{
			// TODO(toni): Handle this type!
			return true;
		}
		case MB_YESNO:
		{
			int center_x = (client_bounds.right / 2);

			CClassicButton *button_no = new CClassicButton(
				info->hInstance,
				GenerateNewClassName(),
				"No"
			);

			button_no->SetSize(75, 23);

			button_no->SetXPosition(center_x + 3);
			button_no->SetYPosition(client_bounds.bottom - 34);

			button_no->event_listener = MessageBoxEventListener;

			window->AddComponent(button_no);

			CClassicButton *button_yes = new CClassicButton(
				info->hInstance,
				GenerateNewClassName(),
				"Yes"
			);

			button_yes->SetSize(75, 23);

			button_yes->SetXPosition(center_x - 78);
			button_yes->SetYPosition(client_bounds.bottom - 34);

			button_yes->event_listener = MessageBoxEventListener;

			window->AddComponent(button_yes);

			return true;
		}
		case MB_RETRYCANCEL:
		{
			int center_x = (client_bounds.right / 2);

			CClassicButton *button_cancel = new CClassicButton(
				info->hInstance,
				GenerateNewClassName(),
				"Cancel"
			);

			button_cancel->SetSize(75, 23);

			button_cancel->SetXPosition(center_x + 3);
			button_cancel->SetYPosition(client_bounds.bottom - 34);

			button_cancel->event_listener = MessageBoxEventListener;

			window->AddComponent(button_cancel);

			CClassicButton *button_retry = new CClassicButton(
				info->hInstance,
				GenerateNewClassName(),
				"Retry"
			);

			button_retry->SetSize(75, 23);

			button_retry->SetXPosition(center_x - 78);
			button_retry->SetYPosition(client_bounds.bottom - 34);

			button_retry->event_listener = MessageBoxEventListener;

			window->AddComponent(button_retry);

			return true;
		}
		case MB_CANCELTRYCONTINUE:
		{
			// TODO(toni): Handle this type!
			return true;
		}
		default:
		{
			break;
		}
	}

	return false;
}

DWORD WINAPI MessageBoxWorker(LPVOID param)
{
	LPMSGBOXINFO info = (LPMSGBOXINFO)param;
	
	CClassicWnd *window = new CClassicWnd(
		info->hInstance, 
		GenerateNewClassName("MSGBOX_Classic_Window"), 
		GenerateNewClassName("MSGBOX_Classic_Client")
	);

	window->SetTitle(info->title);

	window->SetMinimizable(false);
	window->SetResizable(false);

	CClassicLabel *label_message = new CClassicLabel(
		info->hInstance, 
		GenerateNewClassName(), 
		info->message
	);
	
	// A temporary draw context which covers all available monitors
	// we just use it to compute the size of the message label
	HDC temp_dc = CreateDC("DISPLAY", NULL, NULL, NULL);

	// Selecting our font, so that the metrics are correct
	SelectObject(temp_dc, window->GetFont());

	int label_w = 0,
		label_h = 0;

	TSTRING token = _strdup(info->message);
	token = strtok(token, "\n");

	if (token)
	{
		while (token)
		{
			SIZE ln_size;
			GetTextExtentPoint32(temp_dc, token, StrLenA(token), &ln_size);

			if (label_w < ln_size.cx)
				label_w = ln_size.cx;

			label_h += ln_size.cy;

			token = strtok(NULL, "\n");
		}
	}
	else
	{
		// If the message is only one line, just use the entire message
		SIZE msg_size;
		GetTextExtentPoint32(temp_dc, info->message, StrLenA(info->message), &msg_size);

		label_w = msg_size.cx;
		label_h = msg_size.cy;
	}

	// Getting rid of the draw context that we created earlier...
	DeleteDC(temp_dc);

	label_message->SetSize(label_w, label_h);
	label_message->SetPosition(59, 11);
	label_message->SetTextFormatFlags(DT_WORDBREAK);

	// Finally setting the size of the window
	window->SetSize(label_w + 84, label_h + 92);

	window->AddComponent(label_message);

	// Check for an icon
	HICON box_icon = CheckAndLoadIcon(info);

	if (box_icon)
	{
		// If an icon has been loaded somehow...

		CClassicIcon *icon_message = new CClassicIcon(
			info->hInstance, 
			GenerateNewClassName(),
			box_icon
		);

		icon_message->SetSize(32, 32);
		icon_message->SetPosition(11, 11);

		window->AddComponent(icon_message);
	}

	// If this operation fails, the user passed in an invalid or unknown flag!
	if (!HandleMsgBoxTypes(info, window))
	{
		SetLastError(ERROR_INVALID_DATA);
		DBG_ErrorExit("Message box creation (Worker thread)");

		return ERROR_INVALID_DATA;
	}
	
	RECT parent_bounds;

	// If parent is NULL, just use the Desktop window
	GetWindowRect((info->parent) ? info->parent : GetDesktopWindow(), &parent_bounds);
	window->SetPositionRelativeTo(parent_bounds);

	// TODO(toni): Maybe we'll implement our own "Windows Classic" themed beep sounds
	MessageBeep(info->creation_flags & MB_ICONMASK);

	window->CreateAndShow();

	// TODO(toni): In the future this function should return the users decision(s)!
	info->result = 0;
	
	return 0;
}

void MessageBoxEventListener(CClassicComponent *source,
								UINT message,
								WPARAM wParam,
								LPARAM lParam)
{
	// Nothing (yet)
}