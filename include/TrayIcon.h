#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <functional>

// Tray menu command IDs
#define TRAY_ID 1
#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_BREAK_NOW 1001
#define ID_TRAY_ABOUT 1002
#define ID_TRAY_QUIT 1003

class TrayIcon
{
public:
    // Callbacks set by App before calling init()
    std::function<void()> onBreakNow;
    std::function<void()> onQuit;

    bool init(HINSTANCE hInst);
    void remove();

    // Win32 window procedure for the hidden message pump window
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
    NOTIFYICONDATAA nid{}; // ANSI variant — works with MinGW without UNICODE
    HWND hwnd = nullptr;
    HINSTANCE hInst = nullptr;

    void showContextMenu();

    static TrayIcon* s_instance; // Singleton pointer so WndProc can reach the instance
};
