
/*
 * Message Box API
 */

#include "cclassicwnd.h"
#include "resource.h"

typedef struct
{
	HWND			parent;
	HINSTANCE		hInstance;
	TSTRING			message, 
					title;
	HICON			icon;

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

int MessageBoxClassic(HWND parent,
						HINSTANCE hInst,
						TSTRING message,
						TSTRING title,
						UINT flags)
{
	DWORD thread_id;
	HANDLE thread_handle;

	LPMSGBOXINFO info		= (LPMSGBOXINFO)malloc(sizeof(MSGBOXINFO));
	info->parent			= parent;
	info->hInstance			= hInst;
	info->message			= message;
	info->title				= title;
	info->creation_flags	= flags;

	// Check for icons
	if (CONTAINSFLAG(flags, MB_ICONHAND))
	{
		info->icon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ERROR_CLASSIC));
	}
	else if (CONTAINSFLAG(flags, MB_ICONQUESTION))
	{
		info->icon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_QUESTION_CLASSIC));
	}
	else if (CONTAINSFLAG(flags, MB_ICONEXCLAMATION))
	{
		info->icon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_WARNING_CLASSIC));
	}
	else if (CONTAINSFLAG(flags, MB_ICONASTERISK))
	{
		info->icon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_INFO_CLASSIC));
	}
	else
	{
		info->icon = NULL; // NULL tells the dialog to not add any icon component to the window
	}
	
	thread_handle = CreateThread(NULL, 0, MessageBoxWorker, info, 0, &thread_id);

	info->thread_id		= thread_id;
	info->thread_handle = thread_handle;

	// We gotta wait until the thread terminates
	WaitForSingleObject(thread_handle, INFINITE);
	CloseHandle(thread_handle);

	int result = info->result;

	free(info);

	return result;
}

DWORD WINAPI MessageBoxWorker(LPVOID param)
{
	LPMSGBOXINFO info = (LPMSGBOXINFO)param;

	// TODO(toni): Generate unique window class names for each element!
	// (Get rid of the "Some_message_box_..." stuff!)
	CClassicWnd *window = new CClassicWnd(info->hInstance, "Some_message_box", "Some_message_box_clientarea");

	window->SetTitle(info->title);

	window->SetMinimizable(false);
	window->SetResizable(false);

	// Set the size first!
	// Currently the size is only hardcoded, but i wanna make it dynamic eventually!
	window->SetSize(243, 118);

	RECT parent_bounds;

	// If parent is NULL, just use the Desktop window
	GetWindowRect((info->parent) ? info->parent : GetDesktopWindow(), &parent_bounds);
	window->SetPositionRelativeTo(parent_bounds);

	RECT client_bounds = window->GetClientBounds();

	CClassicLabel *label_message = new CClassicLabel(info->hInstance, "Some_message_box_label", info->message);
	label_message->SetSize(170, 40);
	label_message->SetPosition(56, 10);
	label_message->SetTextFormatFlags(DT_CALCRECT);

	window->AddComponent(label_message);

	if (info->icon)
	{
		CClassicIcon *icon_message = new CClassicIcon(info->hInstance, "Some_message_box_icon", info->icon);
		icon_message->SetSize(32, 32);
		icon_message->SetXPosition(14);
		icon_message->SetYPositionRelativeTo(label_message->GetBounds());

		window->AddComponent(icon_message);
	}

	// Here i'll probably make different message box types for different purposes i the future!
	// ( Yes No - Yes No Cancel - OK Cancel - OK - etc... )
	// But for now this message box will only contain the infamous little "OK" button

	if (CONTAINSFLAG(info->creation_flags, MB_ABORTRETRYIGNORE))
	{
	}
	else if (CONTAINSFLAG(info->creation_flags, MB_CANCELTRYCONTINUE))
	{
	}
	else if (CONTAINSFLAG(info->creation_flags, MB_HELP))
	{
		// Dunno if we should ever use this...
	}
	else if (CONTAINSFLAG(info->creation_flags, MB_OK))
	{
	}
	else if (CONTAINSFLAG(info->creation_flags, MB_OKCANCEL))
	{
	}
	else if (CONTAINSFLAG(info->creation_flags, MB_RETRYCANCEL))
	{
	}
	else if (CONTAINSFLAG(info->creation_flags, MB_YESNO))
	{
	}
	else if (CONTAINSFLAG(info->creation_flags, MB_YESNOCANCEL))
	{
	}

	CClassicButton *button_ok = new CClassicButton(info->hInstance, "Some_message_box_button", "OK");
	button_ok->SetSize(75, 23);

	button_ok->SetXPositionRelativeTo(client_bounds);
	button_ok->SetYPosition(client_bounds.bottom - 34);

	button_ok->event_listener = MessageBoxEventListener;

	window->AddComponent(button_ok);

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