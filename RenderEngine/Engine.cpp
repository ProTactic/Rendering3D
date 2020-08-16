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

	if (!m_graphics.Initialize(this->m_baseWindow.GetHwnd(), width, height)) {
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
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}

void Engine::EngineLoop()
{
	while (m_running) {

		MSG msg = { };
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			m_graphics.Render();
		}
	}


}
