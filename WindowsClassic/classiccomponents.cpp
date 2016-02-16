
#include "cclassicwnd.h"

__tagCClassicComponent::__tagCClassicComponent(HINSTANCE hInst, TSTRING name)
{
	this->wnd_class.cbSize				= sizeof(WNDCLASSEX);
	this->wnd_class.style				= (CS_HREDRAW | CS_VREDRAW);
	this->wnd_class.lpfnWndProc			= Internal_WndProc;
	this->wnd_class.cbClsExtra			= 0;
	this->wnd_class.cbWndExtra			= 0;
	this->wnd_class.hInstance			= hInst;
	this->wnd_class.hIcon				= NULL;
	this->wnd_class.hIconSm				= NULL;
	this->wnd_class.hCursor				= LoadCursor(NULL, IDC_ARROW);
	this->wnd_class.hbrBackground		= (HBRUSH)GetStockObject(WHITE_BRUSH);
	this->wnd_class.lpszMenuName		= NULL;
	this->wnd_class.lpszClassName		= name;

	if ((!RegisterClassEx(&this->wnd_class)) && 
		(GetLastError() != ERROR_CLASS_ALREADY_EXISTS))
	{
		MessageBox(NULL, "Call to RegisterClassEx failed!", "Win32", NULL);
		return;
	}
}

__tagCClassicComponent::~__tagCClassicComponent()
{
}

void __tagCClassicComponent::OnAdd(HWND parent)
{
	// Creating the HWND

	this->hWnd = CreateWindow(
		this->wnd_class.lpszClassName, 
		NULL, 
		WS_CHILD | WS_VISIBLE, 
		this->bounds.left, 
		this->bounds.top, 
		MAKEWIDTH(this->bounds), 
		MAKEHEIGHT(this->bounds), 
		parent, 
		NULL, 
		this->wnd_class.hInstance, 
		NULL
	);

	if (!this->hWnd)
	{
		MessageBox(NULL, "Call to CreateWindow failed!", "Win32", NULL);
		return;
	}

	SetWindowLong(this->hWnd, GWLP_USERDATA, (long)this);
	SendMessage(this->hWnd, WM_CREATE, 0, 0);
}

void __tagCClassicComponent::OnRemove(HWND parent)
{
	DestroyWindow(this->hWnd);
}

// Private function
void __SetBounds(CClassicComponent *inst,
				HWND hWnd,
				RECT &bounds,
				int xPos,
				int yPos,
				int width,
				int height,
				UINT flags)
{
	if (xPos > -1)
		bounds.left = xPos;

	if (yPos > -1)
		bounds.top = yPos;

	if (width > -1)
		bounds.right = MAKECOORDINATE(bounds.left, width);

	if (height > -1)
		bounds.bottom = MAKECOORDINATE(bounds.top, height);

	if (!inst->IsReady())
		return;

	SetWindowPos(
		hWnd,
		NULL,
		xPos, yPos,
		width, height,
		flags
	);
}

void __tagCClassicComponent::SetBounds(int x,
										int y,
										int width,
										int height)
{
	__SetBounds(
		this, 
		this->hWnd, 
		this->bounds, 
		x, y, 
		width, height, 
		SWP_NOZORDER
	);
}

void __tagCClassicComponent::SetPosition(int x, int y)
{
	__SetBounds(
		this,
		this->hWnd,
		this->bounds,
		x, y,
		-1, -1,
		SWP_NOZORDER | SWP_NOSIZE
	);
}

void __tagCClassicComponent::SetSize(int width, int height)
{
	__SetBounds(
		this,
		this->hWnd,
		this->bounds,
		-1, -1,
		width, height,
		SWP_NOZORDER | SWP_NOMOVE
	);
}

void __tagCClassicComponent::SetBackgroundColor(DWORD color)
{
	this->background_color = color;
	this->RepaintComponent();
}

void __tagCClassicComponent::SetForegroundColor(DWORD color)
{
	this->foreground_color = color;
	this->RepaintComponent();
}

void __tagCClassicComponent::RepaintComponent()
{
	if (!this->IsReady())
		return;

	RedrawWindow(this->hWnd, NULL, NULL, RDW_INVALIDATE);
}

LRESULT CALLBACK __tagCClassicComponent::WndProc(HWND hWnd,
												UINT message,
												WPARAM wParam,
												LPARAM lParam)
{
	switch (message)
	{
		case WM_CREATE:
		{
			this->OnCreate();
		} break;
		case WM_PAINT:
		{
			HDC hdc;
			PAINTSTRUCT paintstruct;

			hdc = BeginPaint(hWnd, &paintstruct);

			DRAWCONTEXT context;
			context.paintstruct = paintstruct;
			context.fill_color = this->background_color;
			context.draw_color = this->foreground_color;

			this->PaintComponent(&context);
			
			EndPaint(hWnd, &paintstruct);
		} break;
		case WM_DESTROY:
		{
			// Destroy routine
		} break;
		default:
			return this->HandleMessage(hWnd, message, wParam, lParam);
	}

	return 0;
}


////////////////////////////
////	CCLASSICPANEL	////
////////////////////////////
void __tagCClassicPanel::SetRaised(bool raised)
{
	this->raised = raised;
	this->RepaintComponent();
}

void __tagCClassicPanel::OnCreate(void)
{
}

void __tagCClassicPanel::PaintComponent(DRAWCONTEXT *context)
{
	int width = MAKEWIDTH(context->paintstruct.rcPaint),
		height = MAKEHEIGHT(context->paintstruct.rcPaint);

	CDrawUtils::FillRectangle3D(context, 0, 0, width, height, this->raised);
}

LRESULT CALLBACK __tagCClassicPanel::HandleMessage(HWND hWnd,
													UINT message,
													WPARAM wParam,
													LPARAM lParam)
{
	return DefWindowProc(hWnd, message, wParam, lParam);
}


////////////////////////////
////	CCLASSICBUTTON	////
////////////////////////////
void __tagCClassicButton::SetText(TSTRING text)
{
	this->button_text = text;
	this->RepaintComponent();
}

void __tagCClassicButton::OnCreate(void)
{
}

void __tagCClassicButton::PaintComponent(DRAWCONTEXT *context)
{
	int width = MAKEWIDTH(context->paintstruct.rcPaint),
		height = MAKEHEIGHT(context->paintstruct.rcPaint);

	CDrawUtils::FillRectangle3D(context, 0, 0, width, height, !this->pressed);
	
	if (this->button_text)
	{
		int offset = (this->pressed ? 1 : 0);

		CDrawUtils::DrawString(
			context, 
			this->button_text, 
			5 + offset,  
			5 + offset, 
			width - 10, 
			height - 10, 
			DT_CENTER | DT_VCENTER | DT_SINGLELINE
		);
	}

	if (GetFocus() == this->hWnd)
	{
		RECT bounds = {
			context->paintstruct.rcPaint.left + 3, 
			context->paintstruct.rcPaint.top + 3, 
			context->paintstruct.rcPaint.right - 3, 
			context->paintstruct.rcPaint.bottom - 3
		};

		// Thanks Micro$oft for that neat little function!
		DrawFocusRect(context->paintstruct.hdc, &bounds);
	}
}

LRESULT CALLBACK  __tagCClassicButton::HandleMessage(HWND hWnd,
													UINT message,
													WPARAM wParam,
													LPARAM lParam)
{
	switch (message)
	{
		case WM_LBUTTONDOWN:
		{
			this->pressed = true;
			this->RepaintComponent();

			SetCapture(hWnd);
		} break;
		case WM_LBUTTONUP:
		{
			if (this->pressed)
			{
				this->pressed = false;
				this->RepaintComponent();

				if (this->event_listener) // User clicked on button
					this->event_listener(this, message, wParam, lParam);

				ReleaseCapture();
			}
		} break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}