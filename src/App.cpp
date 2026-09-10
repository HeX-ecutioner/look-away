#define GLFW_EXPOSE_NATIVE_WIN32

#include "App.h"
#include "UI.h"
#include "resources.h"
#include "AssetManager.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <cmath>
#include <cstdio>
#include <random>
#include <mmsystem.h>
#include <atomic>

static float easeInOut(float t)
{
    if (t <= 0.f) return 0.f;
    if (t >= 1.f) return 1.f;
    if (t < 0.5f) return 4.f * t * t * t;
    float f = -2.f * t + 2.f;
    return 1.f - (f * f * f) * 0.5f;
}

static App *g_AppInstance = nullptr;
static std::atomic<ULONGLONG> g_lastHeartbeatTime{0};

LRESULT CALLBACK App::LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION && g_AppInstance && g_AppInstance->timer.isOnBreak())
    {
        // Safety guard: If the main render/event loop has become unresponsive (no heartbeat for > 1000ms),
        // pass all keystrokes through so the user is never trapped by a frozen process.
        ULONGLONG lastHeartbeat = g_lastHeartbeatTime.load(std::memory_order_relaxed);
        if (lastHeartbeat > 0 && (GetTickCount64() - lastHeartbeat > 1000))
            return CallNextHookEx(nullptr, nCode, wParam, lParam);
        KBDLLHOOKSTRUCT *pkbhs = (KBDLLHOOKSTRUCT *)lParam;

        bool altDown = (pkbhs->flags & LLKHF_ALTDOWN),
            ctrlDown = (GetAsyncKeyState(VK_CONTROL) & 0x8000),
            tab = (pkbhs->vkCode == VK_TAB),
            f4 = (pkbhs->vkCode == VK_F4),
            escape = (pkbhs->vkCode == VK_ESCAPE),
            lWin = (pkbhs->vkCode == VK_LWIN),
            rWin = (pkbhs->vkCode == VK_RWIN);

        // Break enforcement filter during active break:
        // - Block Alt+Tab task switching
        // - Block Alt+F4 window closing
        // - Block Windows keys (Start menu)
        // - Block Ctrl+Esc (Start menu)
        // Standalone Escape is NOT blocked: quick taps pass through, 2-second hold skips.
        // Heartbeat fail-safe: if the process freezes, all keys pass through automatically.
        if ((altDown && (tab || f4)) || lWin || rWin || (ctrlDown && escape))
            return 1;
    }
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

bool App::init(HINSTANCE hInst)
{
    g_AppInstance = this;

    tray.onBreakNow = [this]() { timer.forceBreak(); };
    tray.onQuit = [this]()
    {
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
    GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);

    for (int i = 0; i < monitorCount; i++)
    {
        const GLFWvidmode *mode = glfwGetVideoMode(monitors[i]);
        int mx, my;
        glfwGetMonitorPos(monitors[i], &mx, &my);

        GLFWwindow *window = glfwCreateWindow(mode->width, mode->height, "Look Away!", nullptr, overlayWindows.empty() ? nullptr : overlayWindows[0]);
        if (!window)
            continue;

        glfwSetWindowPos(window, mx, my);
        overlayWindows.push_back(window);

        HWND hwnd = glfwGetWin32Window(window);

        DWORD exStyle = GetWindowLong(hwnd, GWL_EXSTYLE); // Hide from taskbar
        SetWindowLong(hwnd, GWL_EXSTYLE, exStyle | WS_EX_TOOLWINDOW);
        
        HICON hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1)); // Set Icon
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

    if (!PlaySoundW(L"SystemAsterisk", NULL, SND_ALIAS | SND_ASYNC)) // Play a start-up sound
        AssetManager::logWarning("Failed to play start-up sound (SystemAsterisk).");
    tray.showNotification("Look Away! Started", "Look Away! is now running in your system tray.");
    tray.setIcon(IDI_ICON_GREEN); // Initialize with green dot

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

static const char *FIRST_BREAK_MSG = "Look at something 20 feet away",
    *BREAK_MESSAGES[] = {
        "Give your eyes a rest. Stare into the distance.",
        "Blink slowly. Breathe. You've got this.",
        "Find a window and look as far as you can.",
        "20 seconds of freedom for your retinas",
        "Look up. Look far. Look away.",
        "Rest your focus, the screen will wait.",
        "Step away. Even 20 seconds helps.",
        "Stare at something that isn't a screen.",
    };
static constexpr int NUM_MESSAGES = (int)(sizeof(BREAK_MESSAGES) / sizeof(BREAK_MESSAGES[0]));

void App::pickNextMessage()
{
    if (timer.getBreakCount() <= 1)
    {
        currentMessage = FIRST_BREAK_MSG;
        return;
    }

    static std::mt19937 rng{std::random_device{}()};
    static int lastIdx = -1;
    int next;
    do
    {
        next = (int)(rng() % (unsigned)NUM_MESSAGES);
    } while (next == lastIdx && NUM_MESSAGES > 1);

    lastIdx = next;
    currentMessage = BREAK_MESSAGES[next];
}

void App::beginOverlay()
{
    g_lastHeartbeatTime.store(GetTickCount64(), std::memory_order_relaxed);
    overlayState = OverlayState::FadingIn;
    overlayAlpha = 0.0f;
    fadeStartTime = glfwGetTime();
    escapeHoldStartTime = 0.0;
    escapeHoldProgress = 0.0f;

    switch (tray.currentSound)
    {
    case SoundSetting::Default:
        if (!PlaySoundW(L"SystemNotification", NULL, SND_ALIAS | SND_ASYNC))
            AssetManager::logError("Failed to play default break notification sound (SystemNotification).");
        break;
    case SoundSetting::Rain:
        AssetManager::playSound("rain.wav", IDR_WAVE_RAIN);
        break;
    case SoundSetting::Chime:
        AssetManager::playSound("chime.wav", IDR_WAVE_CHIME);
        break;
    case SoundSetting::Mute:
        break;
    }


    pickNextMessage();

    for (auto *w : overlayWindows)
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
    PlaySoundW(nullptr, nullptr, 0); // Stop any lingering audio

    for (auto *w : overlayWindows)
    {
        glfwHideWindow(w);
        glfwSetInputMode(w, GLFW_CURSOR, GLFW_CURSOR_NORMAL); // Show mouse cursor
    }

    overlayState = OverlayState::Hidden;
    overlayAlpha = 0.f;
    escapeHoldStartTime = 0.0;
    escapeHoldProgress = 0.0f;
}

void App::triggerSkipBreak()
{
    escapeHoldStartTime = 0.0;
    escapeHoldProgress = 0.0f;

    PlaySoundW(nullptr, nullptr, 0); // Stop break sound immediately

    for (auto *w : overlayWindows)
    {
        glfwSetInputMode(w, GLFW_CURSOR, GLFW_CURSOR_NORMAL); // Restore cursor immediately
    }

    timer.skipBreak();

    tray.setLocked(false);
    tray.setIcon(IDI_ICON_GREEN);
    wasOnBreak = false;

    if (overlayState == OverlayState::Visible || overlayState == OverlayState::FadingIn)
    {
        overlayState = OverlayState::FadingOut;
        fadeStartTime = glfwGetTime();
    }
}

void App::updateOverlay()
{
    g_lastHeartbeatTime.store(GetTickCount64(), std::memory_order_relaxed);
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
    }

    if (timer.isOnBreak())
    {
        breakRemaining = timer.getRemaining();

        // Check hold-to-skip via Escape key (requires holding for ESCAPE_HOLD_DURATION)
        if (overlayState == OverlayState::Visible || overlayState == OverlayState::FadingIn)
        {
            bool isEscDown = (GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0;
            bool noModifiers = !(GetAsyncKeyState(VK_CONTROL) & 0x8000) && !(GetAsyncKeyState(VK_MENU) & 0x8000);

            if (isEscDown && noModifiers)
            {
                if (escapeHoldStartTime <= 0.0)
                    escapeHoldStartTime = now;

                double heldDuration = now - escapeHoldStartTime;
                escapeHoldProgress = (float)(heldDuration / ESCAPE_HOLD_DURATION);
                if (escapeHoldProgress > 1.0f)
                    escapeHoldProgress = 1.0f;

                if (heldDuration >= ESCAPE_HOLD_DURATION)
                {
                    triggerSkipBreak();
                    return;
                }
            }
            else
            {
                escapeHoldStartTime = 0.0;
                escapeHoldProgress = 0.0f;
            }
        }

        for (auto *w : overlayWindows) // Force topmost and focus during break
        {
            HWND hwnd = glfwGetWin32Window(w);
            if (GetForegroundWindow() != hwnd && w == overlayWindows[0])
                SetForegroundWindow(hwnd);

            SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        }
    }
    else
    {
        escapeHoldStartTime = 0.0;
        escapeHoldProgress = 0.0f;
    }

    UI::beginFrame();

    for (size_t i = 0; i < overlayWindows.size(); ++i)
    {
        glfwMakeContextCurrent(overlayWindows[i]);
        glClearColor(0.f, 0.f, 0.f, overlayAlpha);
        glClear(GL_COLOR_BUFFER_BIT);

        UI::renderOverlay(overlayWindows[i], overlayAlpha, breakRemaining, currentMessage, escapeHoldProgress, timer.isOnBreak());

        glfwSwapBuffers(overlayWindows[i]);
    }
}

void App::run()
{
    while (!wantsQuit)
    {
        pumpTrayMessages();
        timer.update();

        static int lastRemaining = -1;
        int remaining = timer.getRemaining();
        if (remaining != lastRemaining) // Update Tray Tooltip only when it changes or every second
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
            tray.setIcon(IDI_ICON_YELLOW);
            beginOverlay();
        }
        else if (!nowOnBreak && wasOnBreak)
        {
            tray.setLocked(false);
            tray.setIcon(IDI_ICON_GREEN);
            if (overlayState == OverlayState::Visible || overlayState == OverlayState::FadingIn)
            {
                overlayState = OverlayState::FadingOut;
                fadeStartTime = glfwGetTime();
            }
        }
        wasOnBreak = nowOnBreak;

        if (overlayState != OverlayState::Hidden)
        {
            glfwPollEvents();
            updateOverlay();

            for (auto *w : overlayWindows)
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
            glfwWaitEventsTimeout(0.5); // Use a wait with timeout to keep CPU usage at near-zero when idle.
    }
}

void App::shutdown()
{
    PlaySoundW(nullptr, nullptr, 0);

    if (hhkLowLevelKybd)
        UnhookWindowsHookEx(hhkLowLevelKybd);

    tray.remove();
    UI::shutdown();

    for (auto *w : overlayWindows)
        glfwDestroyWindow(w);

    glfwTerminate();
}