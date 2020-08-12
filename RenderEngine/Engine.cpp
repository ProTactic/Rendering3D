#include "Engine.h"

Engine::Engine(): m_graphics()
{
}

BOOL Engine::Initialize(std::string windowTitle, std::string windowClass, int width, int height)
{
	if (!WindowWraper::Initialize(windowTitle, windowClass, width, height))
	{
		return false;
	}

	if (!m_graphics.Initialize()) {
		return false;
	}
}

LRESULT Engine::HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	switch (uMsg)
	{
	case WM_CLOSE: {
		DestroyWindow(hwnd);
		m_running = false;
	}

	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}

void Engine::EngineLoop()
{
	while (this->m_running) {
		MSG msg = { };
		if (GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}
