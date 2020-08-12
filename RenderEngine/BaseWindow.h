#pragma once

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

protected:
    HWND m_hwnd;
};

class WindowWraper {

protected:
    bool m_running;

public:

    BaseWindow m_baseWindow;

    WindowWraper();

    virtual BOOL Initialize(std::string windowTitle, std::string windowClass, int width, int height);
    virtual LRESULT HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
};