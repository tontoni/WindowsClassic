
#include "resource.h"
#include "cclassicwnd.h"

int WINAPI WinMain(HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPSTR lpCmdLine,
					int nCmdShow)
{
	HICON icon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINDOWS)),
		  icon_small	= (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_WINDOWS), IMAGE_ICON, 16, 16, 0);

	CClassicWnd *window = new CClassicWnd(hInstance, icon, icon_small);

	window->SetTitle("Windows Classic Window OMG!");
	window->SetPosition(200, 200);
	window->SetSize(400, 300);

	// window->SetClosable(false);		- Default = true
	// window->SetResizable(false);		- Default = true
	// window->SetMinimizable(false);	- Default = true

	return window->CreateAndShow();
}