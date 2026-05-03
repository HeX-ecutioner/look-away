#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h> 

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "TimerManager.h"
#include "TrayIcon.h"

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

private:
    GLFWwindow* overlayWindow = nullptr;
    TimerManager timer;
    TrayIcon tray;
    bool wantsQuit = false;

    // Overlay animation
    OverlayState overlayState = OverlayState::Hidden;
    float overlayAlpha = 0.0f;
    double fadeStartTime = 0.0;
    bool wasOnBreak = false;
    int breakRemaining = 0;
    const char* currentMessage = nullptr;
    static constexpr float FADE_DURATION = 0.6f;

    // Helpers
    void pumpTrayMessages();
    void beginOverlay();
    void endOverlay();
    void updateOverlay();
    void pickNextMessage();
};