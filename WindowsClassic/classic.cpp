
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
					  *my_colored_button;

static HINSTANCE hInst;
static CClassicWnd *window;

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
		"Windows_Classic_Window_Test", 
		"Windows_Classic_Window_ClientArea_Test", 
		icon, 
		icon_small
	);

	window->SetTitle("Windows Classic Window OMG!");
	window->SetSize(400, 300);
	window->SetPosition(200, 200);

	my_button = new CClassicButton(hInstance, "Windows_Classic_Window_Button_1", "Hello! I\'m a button!");
	my_button->SetBounds(10, 140, 160, 40);
	my_button->event_listener = GlobalEventListener;

	my_colored_button = new CClassicButton(hInstance, "Windows_Classic_Window_Button_2", "And i\'m a colored button!");
	my_colored_button->SetBounds(10, 200, 230, 30);

	// Make it colorful!
	my_colored_button->SetBackgroundColor(0x00FFFF);
	my_colored_button->SetForegroundColor(0x0000FF);

	my_colored_button->event_listener = GlobalEventListener;

	window->AddComponent(my_button);
	window->AddComponent(my_colored_button);

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
		const TSTRING message = (
			"Do line breaks work?\n\
			Sure they do.\n \n\
			Blablablablablablablablablabla This is some long sample text blablablablablablablablablabla\n \n\
			The dialog box should automatically adjust it\'s size to the text inside this label"
		);

		const TSTRING title = "Mah message bawx!";

#if 1
		MessageBoxClassic(
			NULL, 
			hInst,
			message, 
			title, 
			MB_ABORTRETRYIGNORE | MB_ICONINFORMATION
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
		my_colored_button->SetText("Whoops! And now \'m black!");

		HPOPUP popup = CreatePopupMenuClassic(hInst, PopupProc);

		CreateAndAppendMenuItemClassic(popup, CPM_ITEM_TEXT, "Menu Item 1");
		CreateAndAppendMenuItemClassic(popup, CPM_ITEM_TEXT | CPM_ITEM_DEFAULT, "Menu Item 2 (Default Option)");
		
		CreateAndAppendMenuItemClassic(popup, CPM_ITEM_SEPARATOR, NULL);

		CreateAndAppendMenuItemClassic(popup, CPM_ITEM_TEXT, "Menu Item 3");
		CreateAndAppendMenuItemClassic(popup, CPM_ITEM_TEXT, "Menu Item 4");
		CreateAndAppendMenuItemClassic(popup, CPM_ITEM_TEXT | CPM_ITEM_STATE_DISABLED, "Disabled Menu Item");
		
		CreateAndAppendMenuItemClassic(popup, CPM_ITEM_SEPARATOR, NULL);

		CreateAndAppendMenuItemClassic(popup, CPM_ITEM_TEXT, "Some big menu item");
		CreateAndAppendMenuItemClassic(popup, CPM_ITEM_TEXT, "Some menu item that is even bigger");
		
		ShowPopupMenuClassic(popup);
	}
}

void PopupProc(HPOPUP popup, UINT message, LPVOID param)
{
	if (message == CPM_ITEMSELECTED)
	{
		LPCPM_ITEMINFO info = (LPCPM_ITEMINFO)param;

		TCHAR buff[32];
		sprintf(buff, "Selected Menu Item with index %d", info->menu_index);

		MessageBox(NULL, buff, "Selected item", 0);
	}
}

#endif // _PROG_CLASSICTEST