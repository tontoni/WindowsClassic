
#ifdef _PROG_CLASSICTEST
/*

		SAMPLE FILE FOR TESTING!

*/

#include "windowsclassic.h"

static void GlobalEventListener(CClassicComponent *source,
								UINT event, 
								WPARAM wParam, 
								LPARAM lParam);

static void PopupProc(HPOPUP popup, UINT message, LPVOID param);

static CClassicButton *my_button, 
					  *my_colored_button, 
					  *scale_window_button;

static LPCClassicTextbox my_textbox;

static HINSTANCE hInst;
static CClassicWnd *window;

static int scale_index = 0;

int WINAPI WinMain(HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPSTR lpCmdLine,
					int nCmdShow)
{
	hInst = hInstance;

	HICON icon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINDOWS)),
		  icon_small	= (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_WINDOWS), IMAGE_ICON, 16, 16, 0);

	window = new CClassicWnd(
		hInstance, 
		TEXT("Windows_Classic_Window_Test"),
		TEXT("Windows_Classic_Window_ClientArea_Test"),
		icon, 
		icon_small
	);

	window->SetTitle(TEXT("Windows Classic Window OMG!"));
	window->SetSize(400, 400);
	window->SetPosition(200, 200);
	window->SetBackgroundColor(0x000000);

	my_button = new CClassicButton(
		hInstance, 
		TEXT("Windows_Classic_Window_Button_1"),
		TEXT("Hello! I\'m a button!")
	);
	
	my_button->SetBounds(10, 140, 160, 40);
	my_button->event_listener = GlobalEventListener;

	my_colored_button = new CClassicButton(
		hInstance, 
		TEXT("Windows_Classic_Window_Button_2"),
		TEXT("And i\'m a colored button!")
	);
	
	my_colored_button->SetBounds(10, 200, 230, 30);

	// Make it colorful!
	my_colored_button->SetBackgroundColor(0x00FFFF);
	my_colored_button->SetForegroundColor(0x0000FF);

	my_colored_button->event_listener = GlobalEventListener;

	scale_window_button = new CClassicButton(
		hInstance, 
		TEXT("Windows_Classic_Window_Button_3"),
		TEXT("Scale Window")
	);
	
	scale_window_button->SetBounds(200, 140, 200, 30);
	scale_window_button->event_listener = GlobalEventListener;

	my_textbox = new CClassicTextbox(
		hInstance, 
		TEXT("Windows_Classic_Textbox"), 
		TEXT("My textbox")
	);

	my_textbox->SetBounds(10, 250, 200, 20);
	
	window->AddComponent(my_button);
	window->AddComponent(my_colored_button);
	window->AddComponent(scale_window_button);
	window->AddComponent(my_textbox);

	// window->SetClosable(false);		- Default = true
	// window->SetResizable(false);		- Default = true
	// window->SetMinimizable(false);	- Default = true

	return window->CreateAndShow();
}

void GlobalEventListener(CClassicComponent *source,
						UINT event,
						WPARAM wParam,
						LPARAM lParam)
{
	if (source == my_button) // Same address
	{
		const STRING message = TEXT(
			"Do line breaks work?\n\
			Sure they do.\n \n\
			Blablablablablablablablablabla This is some long sample text blablablablablablablablablabla\n \n\
			The dialog box should automatically adjust it\'s size to the text inside this label"
		);

		const STRING title = TEXT("Mah message bawx!");

#if 1
		MessageBoxClassic(
			NULL, 
			message, 
			title, 
			MB_OK | MB_ICONINFORMATION
		);
#else
		MessageBox(
			NULL, 
			message, 
			title, 
			MB_OKCANCEL | MB_ICONINFORMATION
		);
#endif
	}
	else if (source == my_colored_button)
	{
		my_colored_button->SetBackgroundColor(0x000000);
		my_colored_button->SetText(TEXT("Whoops! And now \'m black!"));

		HPOPUP popup = CreatePopupMenuClassic(hInst, PopupProc);

		CreateAndAppendMenuItemClassic(popup, CPM_ITEM_TEXT, TEXT("Menu Item 1"));
		CreateAndAppendMenuItemClassic(popup, CPM_ITEM_TEXT | CPM_ITEM_DEFAULT, TEXT("Menu Item 2 (Default Option)"));
		
		CreateAndAppendMenuItemClassic(popup, CPM_ITEM_SEPARATOR, NULL);

		CreateAndAppendMenuItemClassic(popup, CPM_ITEM_TEXT, TEXT("Menu Item 3"));
		CreateAndAppendMenuItemClassic(popup, CPM_ITEM_TEXT, TEXT("Menu Item 4"));
		CreateAndAppendMenuItemClassic(popup, CPM_ITEM_TEXT | CPM_ITEM_STATE_DISABLED, TEXT("Disabled Menu Item"));
		
		CreateAndAppendMenuItemClassic(popup, CPM_ITEM_SEPARATOR, NULL);

		CreateAndAppendMenuItemClassic(popup, CPM_ITEM_TEXT, TEXT("Some big menu item"));
		CreateAndAppendMenuItemClassic(popup, CPM_ITEM_TEXT, TEXT("Some menu item that is even bigger"));
		
		ShowPopupMenuClassic(popup);
	}
	else if (source == scale_window_button)
	{
		++scale_index;

		if (scale_index > 3)
			scale_index = 0;

		window->SetTitlebarFont(
			CreateSimpleFontIndependent(TEXT("MS Sans Serif"), 8 + (scale_index * 2), FW_BOLD)
		);
	}
}

void PopupProc(HPOPUP popup, UINT message, LPVOID param)
{
}

#endif // _PROG_CLASSICTEST