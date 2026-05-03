#include "TrayIcon.h"
#include "resources.h"

TrayIcon* TrayIcon::s_instance = nullptr;

bool TrayIcon::init(HINSTANCE hInstance)
{
    hInst = hInstance;
    s_instance = this;

    WNDCLASSEXA wc  = {}; // Register a hidden message-only window class for the tray pump
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = "LookAwayTrayClass";
    RegisterClassExA(&wc);

    hwnd = CreateWindowExA(0, "LookAwayTrayClass", "LookAway", 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, hInst, nullptr);
    if (!hwnd)
        return false;

    nid = {}; // Build the NOTIFYICONDATA (ANSI variant)
    nid.cbSize = sizeof(NOTIFYICONDATAA);
    nid.hWnd = hwnd;
    nid.uID = TRAY_ID;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1));
    strncpy(nid.szTip, "LookAway - 20-20-20 Timer", sizeof(nid.szTip) - 1);
    nid.szTip[sizeof(nid.szTip) - 1] = '\0';

    Shell_NotifyIconA(NIM_ADD, &nid);
    return true;
}

void TrayIcon::remove()
{
    Shell_NotifyIconA(NIM_DELETE, &nid);
    if (hwnd)
        DestroyWindow(hwnd);
}
 
void TrayIcon::updateTooltip(const char* text)
{
    strncpy(nid.szTip, text, sizeof(nid.szTip) - 1);
    nid.szTip[sizeof(nid.szTip) - 1] = '\0';
    nid.uFlags = NIF_TIP;
    Shell_NotifyIconA(NIM_MODIFY, &nid);
}

void TrayIcon::showNotification(const char* title, const char* message)
{
    nid.uFlags = NIF_INFO;
    strncpy(nid.szInfoTitle, title, sizeof(nid.szInfoTitle) - 1);
    nid.szInfoTitle[sizeof(nid.szInfoTitle) - 1] = '\0';
    strncpy(nid.szInfo, message, sizeof(nid.szInfo) - 1);
    nid.szInfo[sizeof(nid.szInfo) - 1] = '\0';
    nid.dwInfoFlags = NIIF_INFO; // Standard info icon
    nid.uTimeout = 5000; // 5 seconds (deprecated in Windows 11but kept for compatibility)
    Shell_NotifyIconA(NIM_MODIFY, &nid);
}

void TrayIcon::showContextMenu()
{
    HMENU menu = CreatePopupMenu();
    AppendMenuA(menu, MF_STRING, ID_TRAY_BREAK_NOW, "Take a Break Now");
    AppendMenuA(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuA(menu, MF_STRING, ID_TRAY_ABOUT, "About");
    AppendMenuA(menu, MF_STRING, ID_TRAY_QUIT, "Quit");

    SetForegroundWindow(hwnd); // Required so the menu closes when clicking elsewhere

    POINT pt;
    GetCursorPos(&pt);
    int cmd = TrackPopupMenu(menu, TPM_RETURNCMD | TPM_NONOTIFY | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, nullptr);
    DestroyMenu(menu);

    switch (cmd)
    {
    case ID_TRAY_BREAK_NOW:
        if (onBreakNow) onBreakNow();
        break;
    case ID_TRAY_ABOUT:
        ShellExecuteA(nullptr, "open", "https://github.com/HeX-ecutioner/look-away", nullptr, nullptr, SW_SHOWNORMAL);
        break;
    case ID_TRAY_QUIT:
        if (onQuit) onQuit();
        break;
    }
}

LRESULT CALLBACK TrayIcon::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_TRAYICON)
    {
        UINT event = LOWORD(lParam);
        if (event == WM_RBUTTONUP || event == WM_CONTEXTMENU)
            if (s_instance && !s_instance->m_locked)
                s_instance->showContextMenu();
        return 0;
    }
    
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}
