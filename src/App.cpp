#define GLFW_EXPOSE_NATIVE_WIN32

#include "App.h"
#include "UI.h"
#include "resources.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <cmath>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <mmsystem.h>

static float clamp01(float v) { return v < 0.f ? 0.f : v > 1.f ? 1.f : v; }

static float easeInOut(float t)
{
    t = clamp01(t);
    return t < 0.5f ? 4.f * t * t * t : 1.f - powf(-2.f * t + 2.f, 3.f) / 2.f;
}

static App* g_AppInstance = nullptr;

LRESULT CALLBACK App::LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION && g_AppInstance && g_AppInstance->timer.isOnBreak())
    {
        KBDLLHOOKSTRUCT* pkbhs = (KBDLLHOOKSTRUCT*)lParam;
        
        bool altDown = (pkbhs->flags & LLKHF_ALTDOWN);
        bool ctrlDown = (GetKeyState(VK_CONTROL) & 0x8000);
        bool shiftDown = (GetKeyState(VK_SHIFT) & 0x8000);
        
        bool tab = (pkbhs->vkCode == VK_TAB);
        bool escape = (pkbhs->vkCode == VK_ESCAPE);
        bool lWin = (pkbhs->vkCode == VK_LWIN);
        bool rWin = (pkbhs->vkCode == VK_RWIN);
        bool f4 = (pkbhs->vkCode == VK_F4);
        bool space = (pkbhs->vkCode == VK_SPACE);

        // Block Alt+Tab, Alt+Esc, Alt+F4, Alt+Space, Windows Keys, and Ctrl+Shift+Esc
        if ((altDown && (tab || escape || f4 || space)) || lWin || rWin || (ctrlDown && shiftDown && escape))
            return 1; 
    }
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

bool App::init(HINSTANCE hInst)
{
    g_AppInstance = this;

    tray.onBreakNow = [this]() { timer.forceBreak(); };
    tray.onQuit = [this]() { 
        if (!timer.isOnBreak()) 
            wantsQuit = true; 
    };
    if (!tray.init(hInst))
        return false;

    hhkLowLevelKybd = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, hInst, 0); // Install keyboard hook

    if (!glfwInit()) // Initialize GLFW (hidden at startup)
        return false;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
    glfwWindowHint(GLFW_FOCUS_ON_SHOW, GLFW_TRUE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

    int monitorCount;
    GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);

    for (int i = 0; i < monitorCount; ++i)
    {
        const GLFWvidmode* mode = glfwGetVideoMode(monitors[i]);
        int mx, my;
        glfwGetMonitorPos(monitors[i], &mx, &my);

        GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Look Away!", nullptr, overlayWindows.empty() ? nullptr : overlayWindows[0]);
        if (!window) continue;

        glfwSetWindowPos(window, mx, my);
        overlayWindows.push_back(window);

        HWND hwnd = glfwGetWin32Window(window);
        
        // Hide from taskbar
        DWORD exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
        SetWindowLong(hwnd, GWL_EXSTYLE, exStyle | WS_EX_TOOLWINDOW);

        // Set Icon
        HICON hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1));
        SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
        SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

        SetWindowPos(hwnd, HWND_TOPMOST, mx, my, mode->width, mode->height, SWP_NOACTIVATE);
    }

    if (overlayWindows.empty())
        return false;

    glfwMakeContextCurrent(overlayWindows[0]);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        return false;

    if (!UI::init(overlayWindows[0])) // Initialize UI (Fonts, ImGui) for the primary window
        return false;

    PlaySoundW(L"SystemAsterisk", NULL, SND_ALIAS | SND_ASYNC); // Play a start-up sound
    char startMsg[256];
    snprintf(startMsg, sizeof(startMsg), "Look Away! is now running in your system tray.");
    tray.showNotification("Look Away! Started", startMsg);

    return true;
}

void App::pumpTrayMessages()
{
    MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

static const char* FIRST_BREAK_MSG = "Look at something 20 feet away";
static const char* BREAK_MESSAGES[] = {
    "Give your eyes a rest — stare into the distance",
    "Blink slowly. Breathe. You've got this.",
    "Find a window and look as far as you can",
    "Your eyes work hard. Let them wander.",
    "20 seconds of freedom for your retinas",
    "Look up. Look far. Look easy.",
    "Rest your focus — the screen will wait",
    "Hydrate while you're at it.",
    "Step away. Even 20 seconds helps.",
    "Stare at something that isn't a screen",
};
static constexpr int NUM_MESSAGES = (int)(sizeof(BREAK_MESSAGES) / sizeof(BREAK_MESSAGES[0]));

void App::pickNextMessage()
{
    if (timer.getBreakCount() <= 1)
    {
        currentMessage = FIRST_BREAK_MSG;
        return;
    }

    static int lastIdx = -1;
    int next;
    do { next = std::rand() % NUM_MESSAGES; }
    while (next == lastIdx && NUM_MESSAGES > 1);
    
    lastIdx = next;
    currentMessage = BREAK_MESSAGES[next];
}

void App::beginOverlay()
{
    overlayState = OverlayState::FadingIn;
    overlayAlpha  = 0.0f;
    fadeStartTime = glfwGetTime();

    PlaySoundW(L"SystemNotification", NULL, SND_ALIAS | SND_ASYNC); // Play a notification sound when the break starts

    pickNextMessage();

    for (auto* w : overlayWindows)
    {
        glfwSetInputMode(w, GLFW_CURSOR, GLFW_CURSOR_HIDDEN); // Hide mouse cursor
        glfwShowWindow(w);
        HWND hwnd = glfwGetWin32Window(w);
        SetForegroundWindow(hwnd);
        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }
}

void App::endOverlay()
{
    for (auto* w : overlayWindows)
    {
        glfwHideWindow(w);
        glfwSetInputMode(w, GLFW_CURSOR, GLFW_CURSOR_NORMAL); // Show mouse cursor
    }

    overlayState = OverlayState::Hidden;
    overlayAlpha = 0.f;
}

void App::updateOverlay()
{
    double now = glfwGetTime();

    switch (overlayState)
    {
    case OverlayState::FadingIn:
    {
        float t = (float)((now - fadeStartTime) / FADE_DURATION);
        overlayAlpha = easeInOut(t);
        if (t >= 1.f)
        {
            overlayAlpha = 1.f;
            overlayState = OverlayState::Visible;
        }
        break;
    }
    case OverlayState::FadingOut:
    {
        float t = (float)((now - fadeStartTime) / FADE_DURATION);
        overlayAlpha = easeInOut(1.f - t);
        if (t >= 1.f)
        {
            endOverlay();
            return;
        }
        break;
    }
    default:
        break;
    }

    if (timer.isOnBreak())
    {
        breakRemaining = timer.getRemaining();
        
        for (auto* w : overlayWindows) // Force topmost and focus during break
        {
            HWND hwnd = glfwGetWin32Window(w);
            if (GetForegroundWindow() != hwnd && w == overlayWindows[0])
                SetForegroundWindow(hwnd);
                
            SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        }
    }

    for (size_t i = 0; i < overlayWindows.size(); ++i) // Render to all windows
    {
        glfwMakeContextCurrent(overlayWindows[i]);
        glClearColor(0.f, 0.f, 0.f, 0.f);
        glClear(GL_COLOR_BUFFER_BIT);

        if (i == 0)
            UI::renderOverlay(overlayAlpha, breakRemaining, currentMessage); // Only primary window gets the full UI
        else
        {
            // Others get a solid black shield
            // We can use a simple glClear with alpha or just a quad
            // Since we use GLFW_TRANSPARENT_FRAMEBUFFER, we need to draw something
            // But glClear with alpha 1.0 works too
            glClearColor(0.f, 0.f, 0.f, overlayAlpha);
            glClear(GL_COLOR_BUFFER_BIT);
        }
        glfwSwapBuffers(overlayWindows[i]);
    }
}

void App::run()
{
    while (!wantsQuit)
    {
        pumpTrayMessages();
        timer.update();

        // Update Tray Tooltip only when it changes or every second
        static int lastRemaining = -1;
        int remaining = timer.getRemaining();
        if (remaining != lastRemaining)
        {
            lastRemaining = remaining;
            char buf[128];
            if (timer.isOnBreak())
                snprintf(buf, sizeof(buf), "Look Away! - On Break (%ds left)", remaining);
            else
                snprintf(buf, sizeof(buf), "Look Away! - Next break in %02d:%02d", remaining / 60, remaining % 60);
            tray.updateTooltip(buf);
        }

        bool nowOnBreak = timer.isOnBreak();
        if (nowOnBreak && !wasOnBreak)
        {
            tray.setLocked(true);
            beginOverlay();
        }
        else if (!nowOnBreak && wasOnBreak)
        {
            tray.setLocked(false);
            if (overlayState == OverlayState::Visible || overlayState == OverlayState::FadingIn)
            {
                overlayState  = OverlayState::FadingOut;
                fadeStartTime = glfwGetTime();
            }
        }
        wasOnBreak = nowOnBreak;

        if (overlayState != OverlayState::Hidden)
        {
            glfwPollEvents();
            updateOverlay();

            for (auto* w : overlayWindows)
            {
                if (glfwWindowShouldClose(w))
                {
                    if (timer.isOnBreak())
                        glfwSetWindowShouldClose(w, GLFW_FALSE);
                    else
                    {
                        wantsQuit = true;
                        break;
                    }
                }
            }
        }
        else
        {
            // Use a wait with timeout to keep CPU usage at near-zero when idle.
            // This will wake up immediately for mouse/keyboard/tray events.
            glfwWaitEventsTimeout(0.5);
        }
    }
}


void App::shutdown()
{
    if (hhkLowLevelKybd)
        UnhookWindowsHookEx(hhkLowLevelKybd);

    tray.remove();
    UI::shutdown();

    for (auto* w : overlayWindows)
        glfwDestroyWindow(w);

    glfwTerminate();
}