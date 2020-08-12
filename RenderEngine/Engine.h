#pragma once
#include "BaseWindow.h"

class Engine : public WindowWraper
{

public:

	Engine();

	virtual LRESULT HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual void EngineLoop();
};

