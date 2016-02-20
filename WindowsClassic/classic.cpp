
/*

		SAMPLE FILE FOR TESTING!

*/

#include "resource.h"
#include "cclassicwnd.h"

static void GlobalEventListener(CClassicComponent *source,
								UINT event, 
								WPARAM wParam, 
								LPARAM lParam);

static CClassicButton *my_button, 
					  *my_colored_button;

int WINAPI WinMain(HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPSTR lpCmdLine,
					int nCmdShow)
{
#if 0
	HICON icon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINDOWS)),
		  icon_small	= (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_WINDOWS), IMAGE_ICON, 16, 16, 0);

	CClassicWnd *window = new CClassicWnd(hInstance, icon, icon_small);

	window->SetTitle("Windows Classic Window OMG!");
	window->SetPosition(200, 200);
	window->SetSize(400, 300);

	my_button = new CClassicButton(hInstance, "Hello! I\'m a button!");
	my_button->SetBounds(10, 140, 160, 40);
	my_button->event_listener = GlobalEventListener;

	my_colored_button = new CClassicButton(hInstance, "And i\'m a colored button!");
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
#else
	MessageBoxClassic(NULL, hInstance, "(Not used yet)", "Mah message bawx!");
	return 0;
#endif
}

void GlobalEventListener(CClassicComponent *source,
						UINT event,
						WPARAM wParam,
						LPARAM lParam)
{
	if (source == my_button) // Same address
	{
		MessageBox(NULL, "Clicked on button!", "Dux", NULL);
	}
	else if (source == my_colored_button)
	{
		my_colored_button->SetBackgroundColor(0x000000);
		my_colored_button->SetText("Whoops! And now \'m black!");
	}
}