
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
	
	if (!RegisterClassEx(&this->wnd_class))
	{
		DBG_ErrorExit("Register WNDCLASSEX (Window component)");
	}
}

__tagCClassicComponent::~__tagCClassicComponent()
{
}

void __tagCClassicComponent::PostComponentMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
	SendMessage(this->hWnd, msg, wParam, lParam);
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
	UnregisterClass(this->wnd_class.lpszClassName, this->wnd_class.hInstance);
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

	if ((xPos <= -1) &&
		(yPos <= -1))
		flags |= SWP_NOMOVE;

	if ((width <= -1) &&
		(height <= -1))
		flags |= SWP_NOSIZE;

	SetWindowPos(
		hWnd,
		NULL,
		(xPos > -1) ? xPos : bounds.left, 
		(yPos > -1) ? yPos : bounds.top,
		(width > -1) ? width : MAKEWIDTH(bounds), 
		(height > -1) ? height : MAKEHEIGHT(bounds),
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
		SWP_NOZORDER | SWP_NOACTIVATE
	);
}

void __tagCClassicComponent::SetPosition(int x, int y)
{
	__SetBounds(
		this,
		this->hWnd,
		this->bounds,
		x, 
		y,
		MAKEWIDTH(this->bounds), 
		MAKEHEIGHT(this->bounds), 
		SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE
	);
}

void __tagCClassicComponent::SetXPosition(int x)
{
	this->SetBounds(x, -1, MAKEWIDTH(this->bounds), -1);
}

void __tagCClassicComponent::SetYPosition(int y)
{
	this->SetBounds(-1, y, -1, MAKEHEIGHT(this->bounds));
}

void __tagCClassicComponent::SetXPositionRelativeTo(RECT rect)
{
	this->SetXPosition(MAKERELATIVEPOSX(this->bounds, rect));
}

void __tagCClassicComponent::SetYPositionRelativeTo(RECT rect)
{
	this->SetYPosition(MAKERELATIVEPOSY(this->bounds, rect));
}

void __tagCClassicComponent::SetPositionRelativeTo(RECT rect)
{
	int xPos = MAKERELATIVEPOSX(this->bounds, rect),
		yPos = MAKERELATIVEPOSY(this->bounds, rect);

	__SetBounds(
		this,
		this->hWnd,
		this->bounds,
		xPos, 
		yPos,
		MAKEWIDTH(this->bounds), 
		MAKEHEIGHT(this->bounds),
		SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE
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
		SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE
	);
}

void __tagCClassicComponent::SetFont(HFONT font)
{
	this->font = font;
	this->RepaintComponent();
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
			SetFocus(hWnd);
		} break;
		case WM_ACTIVATE:
		{
			// If we get activated, not deactivated
			if (wParam > 0)
				SetFocus(hWnd);
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

			if (this->font)
				SelectObject(hdc, this->font);

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


////////////////////////////////////
////	CCLASSICTEXTCOMPONENT	////
////////////////////////////////////
void __tagCClassicTextComponent::SetText(TSTRING text)
{
	this->text = text;
	this->RepaintComponent();
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

void __tagCClassicPanel::PaintComponent(LPDRAWCONTEXT context)
{
	int width = MAKEWIDTH(context->paintstruct.rcPaint),
		height = MAKEHEIGHT(context->paintstruct.rcPaint);

	CDrawUtils::FillRectangle3D(context, 0, 0, width, height, (this->raised) ? RECT_RAISED : 0);
}

LRESULT CALLBACK __tagCClassicPanel::HandleMessage(HWND hWnd,
													UINT message,
													WPARAM wParam,
													LPARAM lParam)
{
	return DefWindowProc(hWnd, message, wParam, lParam);
}


////////////////////////////
////	CCLASSICICON	////
////////////////////////////
void __tagCClassicIcon::SetIcon(HICON new_icon)
{
	this->icon = new_icon;
	this->RepaintComponent();
}

void __tagCClassicIcon::OnCreate(void)
{
}

void __tagCClassicIcon::PaintComponent(LPDRAWCONTEXT context)
{
	int width = MAKEWIDTH(context->paintstruct.rcPaint),
		height = MAKEHEIGHT(context->paintstruct.rcPaint);

	CDrawUtils::FillSolidRectangle(context, 0, 0, width, height);

	if (this->icon)
	{
		DrawIconEx(
			context->paintstruct.hdc, 
			0, 
			0, 
			this->icon, 
			width, 
			height, 
			0, 
			NULL, 
			DI_NORMAL
		);
	}
}

LRESULT CALLBACK __tagCClassicIcon::HandleMessage(HWND hWnd,
													UINT message,
													WPARAM wParam,
													LPARAM lParam)
{
	return DefWindowProc(hWnd, message, wParam, lParam);
}


////////////////////////////
////	CCLASSICBUTTON	////
////////////////////////////
void __tagCClassicButton::OnCreate(void)
{
}

void __tagCClassicButton::PaintComponent(LPDRAWCONTEXT context)
{
	int width = MAKEWIDTH(context->paintstruct.rcPaint),
		height = MAKEHEIGHT(context->paintstruct.rcPaint);

	bool focused = this->IsFocused();
	UINT flags = (!this->pressed) ? RECT_RAISED : 0;

	if (focused)
		flags |= RECT_OUTLINED;

	CDrawUtils::FillRectangle3D(context, 0, 0, width, height, flags);
	
	if (this->text)
	{
		int offset = (this->pressed ? 1 : 0);

		CDrawUtils::DrawString(
			context, 
			this->text, 
			5 + offset,  
			5 + offset, 
			width - 10, 
			height - 12, 
			DT_CENTER | DT_VCENTER | DT_SINGLELINE
		);
	}

	if (focused)
	{
		RECT bounds = {
			context->paintstruct.rcPaint.left + 4, 
			context->paintstruct.rcPaint.top + 4, 
			context->paintstruct.rcPaint.right - 4, 
			context->paintstruct.rcPaint.bottom - 4
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
			int mx = (int)LOWORD(lParam),
				my = (int)HIWORD(lParam);

			RECT wnd_bounds;
			GetWindowRect(hWnd, &wnd_bounds);

			this->pressed = false;
			this->RepaintComponent();

			if (IsPointInArea(mx, my, 0, 0, MAKEWIDTH(wnd_bounds), MAKEHEIGHT(wnd_bounds)))
			{
				if (this->event_listener) // User clicked on button
					this->event_listener(this, message, wParam, lParam);
			}

			ReleaseCapture();
		} break;
		case WM_KEYDOWN:
		{
			short key_code = (short)wParam;

			if (key_code == VK_RETURN)
			{
				if (this->event_listener)
					this->event_listener(this, message, wParam, lParam);
			}
		} break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}


////////////////////////////
////	CCLASSICLABEL	////
////////////////////////////
void __tagCClassicLabel::SetTextFormatFlags(UINT new_flags)
{
	this->format_flags = new_flags;
	this->RepaintComponent();
}

void __tagCClassicLabel::OnCreate(void)
{
}

void __tagCClassicLabel::PaintComponent(LPDRAWCONTEXT context)
{
	int width = MAKEWIDTH(context->paintstruct.rcPaint),
		height = MAKEHEIGHT(context->paintstruct.rcPaint);

	CDrawUtils::FillSolidRectangle(context, 0, 0, width, height);

	if (this->text)
	{
		UINT format_flags = 0;

		CDrawUtils::DrawString(
			context, 
			this->text, 
			0, 
			0, 
			width, 
			height, 
			format_flags
		);
	}
}

LRESULT CALLBACK __tagCClassicLabel::HandleMessage(HWND hWnd,
													UINT message,
													WPARAM wParam,
													LPARAM lParam)
{
	return DefWindowProc(hWnd, message, wParam, lParam);
}