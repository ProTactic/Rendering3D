#include "Engine.h"

Engine::Engine() : m_graphics(), engineTimer()
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
	MSG msg = { };

	engineTimer.Reset();

	while (m_running) {

		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {

			engineTimer.Tick();

			if (engineTimer.DeltaTime() < SECOND_PER_FRAME) {
				Sleep((SECOND_PER_FRAME - engineTimer.DeltaTime()) * 1000 );
			}

			CalculateFrameStats();
			m_graphics.Render();
		}
	}


}

void Engine::CalculateFrameStats()
{
	// Code computes the average frames per second, and also the 
	// average time it takes to render one frame.  These stats 
	// are appended to the window caption bar.

	static int frameCnt = 0;
	static float timeElapsed = 0.0f;

	frameCnt++;

	// Compute averages over one second period.
	if ((engineTimer.TotalTime() - timeElapsed) >= 1.0f)
	{
		float fps = (float)frameCnt; // fps = frameCnt / 1
		float mspf = 1000.0f / fps;

		std::wstring fpsStr = std::to_wstring(fps);
		std::wstring mspfStr = std::to_wstring(mspf);

		std::wstring windowText = m_baseWindow.m_windowTitle +
			L"    fps: " + fpsStr +
			L"   mspf: " + mspfStr;

		SetWindowText(m_baseWindow.GetHwnd(), windowText.c_str());

		// Reset for next average.
		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}
