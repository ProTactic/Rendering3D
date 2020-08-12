#include "BaseWindow.h"

LRESULT CALLBACK  WindowHandleRedirect(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	WindowWraper* const pWindow = reinterpret_cast<WindowWraper*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	// forward message to window class handler
	return pWindow->HandleMessage(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK  WindowProcSetUp(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_NCCREATE: {
		const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
		WindowWraper* pWindow = reinterpret_cast<WindowWraper*>(pCreate->lpCreateParams);
		if (pWindow == nullptr) //Sanity check
		{
			//trow error
			exit(-1);
		}
		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWindow));
		SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WindowHandleRedirect));
		return pWindow->HandleMessage(hwnd, uMsg, wParam, lParam);
	}
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);;
	}
}


BaseWindow::BaseWindow(): m_hwnd(NULL), m_windowClassName(L""), m_windowTitle(L""),
						m_height(0), m_width(0)
{
}

BOOL BaseWindow::Create(WindowWraper *windoWrapter, std::string windowTitle, std::string windowClass, int width, int height)
{

	this->m_windowClassName = std::wstring(windowClass.begin(), windowClass.end());
	this->m_windowTitle = std::wstring(windowTitle.begin(), windowTitle.end());
	this->m_width = width;
	this->m_height = height;

	//Register window
	WNDCLASSEX wc;
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WindowProcSetUp;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hIcon = NULL;
	wc.hIconSm = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = this->m_windowClassName.c_str();
	wc.cbSize = sizeof(WNDCLASSEX);

	RegisterClassEx(&wc);

	this->m_hwnd = CreateWindowEx(0,
		this->m_windowClassName.c_str(),
		this->m_windowTitle.c_str(),
		WS_OVERLAPPEDWINDOW, //WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,
		0,
		0,
		this->m_width,
		this->m_height,
		NULL,
		NULL,
		GetModuleHandle(NULL),
		windoWrapter);

    return (m_hwnd ? TRUE : FALSE);
}

HWND BaseWindow::GetHwnd()
{
	return m_hwnd;
}

WindowWraper::WindowWraper() : m_baseWindow(), m_running(false)
{
}

BOOL WindowWraper::Initialize(std::string windowTitle, std::string windowClass, int width, int height)
{
	BaseWindow baseWindow;
	if (!baseWindow.Create(this, windowTitle, windowClass, width, height)) {
		//thorw error
		return false;
	}

	this->m_baseWindow = baseWindow;
	this->m_running = true;

	ShowWindow(this->m_baseWindow.GetHwnd(), SW_SHOW);
	return true;
}

UINT WindowWraper::getWindowWidth()
{
	return this->m_baseWindow.m_width;
}

UINT WindowWraper::getWindowHeigth()
{
	return this->m_baseWindow.m_height;
}
