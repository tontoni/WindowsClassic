
/*
 * Message Box API
 */

#include "windowsclassic.h"
#include "utils.h"
#include "resource.h"

typedef struct __tagMSGBOXINFO
{
	HWND			parent;
	HINSTANCE		hInstance;

	WSTRING			message, 
					title;

	UINT			creation_flags;

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
						TSTRING message, 
						TSTRING title, 
						UINT flags)
{
	WSTRING wMessage = NULL, 
			wTitle = NULL;

	if (message)
		// Convert standard message string to a wide char string
		wMessage = AllocWStr(message);

	if (title)
		wTitle = AllocWStr(title);

	int ret_val = MessageBoxClassicW(parent, wMessage, wTitle, flags);
	
	if (wMessage)
		free(wMessage);

	if (wTitle)
		free(wTitle);

	return ret_val;
}

int MessageBoxClassicW(HWND parent,
						WSTRING message,
						WSTRING title,
						UINT flags)
{
	DWORD thread_id;
	HANDLE thread_handle;

	LPMSGBOXINFO info		= (LPMSGBOXINFO)malloc(sizeof(MSGBOXINFO));

	// Zero memory
	memset(info, 0, sizeof(MSGBOXINFO));

	info->parent			= parent;
	info->hInstance			= GetModuleHandle(NULL); // The current instance handle
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

void AddButtons(LPMSGBOXINFO info, 
				LPCClassicWnd window, 
				const STRING *button_texts, 
				int button_cnt, 
				int button_w, 
				int button_h, 
				int button_offset)
{
	RECT client_bounds = window->GetClientBounds();
	int buttons_width = ((button_w + button_offset) * button_cnt);

	for (int i = 0; i < button_cnt; ++i)
	{
		CClassicButton *button = new CClassicButton(
			info->hInstance,
			GenerateNewClassName(),
			button_texts[i]
		);

		button->SetSize(button_w, button_h);

		button->SetPosition(
			((client_bounds.right / 2) - (buttons_width / 2)) + ((button_w + button_offset) * i),
			client_bounds.bottom - 34
		);

		button->event_listener = MessageBoxEventListener;

		window->AddComponent(button);
	}
}

bool HandleMsgBoxTypes(LPMSGBOXINFO info, 
						LPCClassicWnd window)
{
	// We currently only have message boxes that have 
	// 3 buttons at max
	// +1 entry to tell the "StrPtrArrLen" function when to break
	STRING button_texts[4];
	
	for (int i = 0; i < ARRAYSIZE(button_texts); ++i)
		button_texts[i] = NULL;

	switch (info->creation_flags & MB_TYPEMASK)
	{
		// Adding the different message box buttons
		case MB_OK:
		{
			button_texts[0] = TEXT("OK");
		} break;
		case MB_OKCANCEL:
		{
			button_texts[0] = TEXT("OK");
			button_texts[1] = TEXT("Cancel");
		} break;
		case MB_ABORTRETRYIGNORE:
		{
			button_texts[0] = TEXT("Abort");
			button_texts[1] = TEXT("Retry");
			button_texts[2] = TEXT("Ignore");
		} break;
		case MB_YESNOCANCEL:
		{
			button_texts[0] = TEXT("Yes");
			button_texts[1] = TEXT("No");
			button_texts[2] = TEXT("Cancel");
		} break;
		case MB_YESNO:
		{
			button_texts[0] = TEXT("Yes");
			button_texts[1] = TEXT("No");
		} break;
		case MB_RETRYCANCEL:
		{
			button_texts[0] = TEXT("Retry");
			button_texts[1] = TEXT("Cancel");
		} break;
		case MB_CANCELTRYCONTINUE:
		{
			button_texts[0] = TEXT("Cancel");
			button_texts[1] = TEXT("Try Again");
			button_texts[2] = TEXT("Continue");
		}
		default:
		{
			break;
		}
	}

	T_UINT32 button_cnt = StrPtrArrLen(button_texts);

	if (!button_cnt)
		// If the function wasn't able to add any buttons then
		// just fail the call
		return false;

	AddButtons(
		info,
		window,
		button_texts,
		button_cnt,
		75,
		23,
		3
	);
	
	return true;
}

DWORD WINAPI MessageBoxWorker(LPVOID param)
{
	LPMSGBOXINFO info = (LPMSGBOXINFO)param;
	
	CClassicWnd *window = new CClassicWnd(
		info->hInstance, 
		GenerateNewClassName(TEXT("MSGBOX_Classic_Window")), 
		GenerateNewClassName(TEXT("MSGBOX_Classic_Client"))
	);

	window->SetTitle((STRING)info->title);

	window->SetMinimizable(false);
	window->SetResizable(false);

	CClassicLabel *label_message = new CClassicLabel(
		info->hInstance, 
		GenerateNewClassName(), 
		(STRING)info->message
	);
	
	// A temporary draw context which covers all available monitors
	// we just use it to compute the size of the message label
	HDC temp_dc = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL);

	// Selecting our font, so that the metrics are correct
	SelectObject(temp_dc, window->GetFont());

	int label_w = 0,
		label_h = 0;
	
	STRING token = StrDupl((STRING)info->message);
	token = StrToken(token, TEXT("\n"));
	
	if (token)
	{
		while (token)
		{
			SIZE ln_size;
			GetTextExtentPoint32(temp_dc, token, StrLen(token), &ln_size);

			if (label_w < ln_size.cx)
				label_w = ln_size.cx;

			label_h += ln_size.cy;

			token = StrToken(NULL, TEXT("\n"));
		}
	}
	else
	{
		// If the message is only one line, just use the entire message
		SIZE msg_size;

		GetTextExtentPoint32(
			temp_dc, 
			(STRING)info->message, 
			StrLen((STRING)info->message), 
			&msg_size
		);

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