#pragma once
#include "BaseWindow.h"
#include "Graphics/Graphics.h"
#include "EngineTimer.h"

class Engine : public WindowWraper
{

protected:

	Graphics m_graphics;
	EngineTimer engineTimer;

public:

	const float FPS = 30;
	const float SECOND_PER_FRAME = 1 / FPS;

	Engine();
	virtual BOOL Initialize(std::string windowTitle, std::string windowClass, int width, int height);

	virtual LRESULT HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual void EngineLoop();
	void CalculateFrameStats();
};

