#pragma once
#include "BaseWindow.h"
#include "Graphics/Graphics.h"

class Engine : public WindowWraper
{

protected:

	Graphics m_graphics;

public:

	Engine();
	virtual BOOL Initialize(std::string windowTitle, std::string windowClass, int width, int height);

	virtual LRESULT HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual void EngineLoop();
};

