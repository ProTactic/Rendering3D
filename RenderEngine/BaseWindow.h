#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif

#include <Windows.h>
#include <string>

class WindowWraper;

class BaseWindow
{
public:

    std::wstring m_windowClassName;
    std::wstring m_windowTitle;

    int m_width;
    int m_height;

    BaseWindow();

    BOOL Create(WindowWraper *windowWrapter, std::string windowTitle, std::string windowClass, int width, int height);

    HWND GetHwnd();
    HWND m_hwnd;
protected:
    
};

class WindowWraper {

protected:
    bool m_running;

public:

    BaseWindow m_baseWindow;

    WindowWraper();

    UINT getWindowWidth();
    UINT getWindowHeigth();

    virtual BOOL Initialize(std::string windowTitle, std::string windowClass, int width, int height);
    virtual LRESULT HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
};