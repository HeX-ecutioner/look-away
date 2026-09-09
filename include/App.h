#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "TimerManager.h"
#include "TrayIcon.h"
#include <vector>

enum class OverlayState
{
    Hidden,
    FadingIn,
    Visible,
    FadingOut,
};

class App
{
public:
    bool init(HINSTANCE hInst);
    void run();
    void shutdown();

    static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

private:
    std::vector<GLFWwindow *> overlayWindows;
    TimerManager timer;
    TrayIcon tray;
    bool wantsQuit = false;
    HHOOK hhkLowLevelKybd = nullptr;

    // Overlay animation
    OverlayState overlayState = OverlayState::Hidden;
    float overlayAlpha = 0.0f;
    double fadeStartTime = 0.0;
    bool wasOnBreak = false;
    int breakRemaining = 0;
    const char *currentMessage = nullptr;
    static constexpr float FADE_DURATION = 0.6f;

    // Escape hold-to-skip
    double escapeHoldStartTime = 0.0;
    float escapeHoldProgress = 0.0f;
    static constexpr double ESCAPE_HOLD_DURATION = 2.0;

    // Helpers
    void pumpTrayMessages();
    void beginOverlay();
    void endOverlay();
    void updateOverlay();
    void pickNextMessage();
    void triggerSkipBreak();
};