#define GLFW_EXPOSE_NATIVE_WIN32

#include "App.h"
#include "UI.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <cmath>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctime>

static float clamp01(float v) { return v < 0.f ? 0.f : v > 1.f ? 1.f : v; }

static float easeInOut(float t)
{
    t = clamp01(t);
    return t < 0.5f ? 4.f * t * t * t : 1.f - powf(-2.f * t + 2.f, 3.f) / 2.f;
}

bool App::init(HINSTANCE hInst)
{
    // Initialize Tray
    tray.onBreakNow = [this]() { timer.forceBreak(); };
    tray.onQuit     = [this]() { wantsQuit = true;   };
    if (!tray.init(hInst))
        return false;

    // Initialize GLFW (hidden at startup)
    if (!glfwInit())
        return false;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
    glfwWindowHint(GLFW_FOCUS_ON_SHOW, GLFW_TRUE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    overlayWindow = glfwCreateWindow(mode->width, mode->height, "LookAway Overlay", nullptr, nullptr);
    if (!overlayWindow)
        return false;

    HWND hwnd = glfwGetWin32Window(overlayWindow);
    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, mode->width, mode->height, SWP_NOACTIVATE);

    glfwMakeContextCurrent(overlayWindow);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        return false;

    // Initialize UI (Fonts, ImGui)
    if (!UI::init(overlayWindow))
        return false;

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
    "Hydrate while you're at it \xf0\x9f\x92\xa7",
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

    pickNextMessage();

    glfwSetInputMode(overlayWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN); // Hide mouse cursor

    glfwShowWindow(overlayWindow);
    HWND hwnd = glfwGetWin32Window(overlayWindow);
    SetForegroundWindow(hwnd);
    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}

void App::endOverlay()
{
    glfwHideWindow(overlayWindow);
    glfwSetInputMode(overlayWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL); // Show mouse cursor

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
        breakRemaining = timer.getRemaining();

    UI::renderOverlay(overlayAlpha, breakRemaining, currentMessage);
}

void App::run()
{
    while (true)
    {
        pumpTrayMessages();
        if (wantsQuit)
            break;

        timer.update();

        bool nowOnBreak = timer.isOnBreak();

        if (nowOnBreak && !wasOnBreak)
            beginOverlay();

        else if (!nowOnBreak && wasOnBreak)
        {
            if (overlayState == OverlayState::Visible || overlayState == OverlayState::FadingIn)
            {
                overlayState  = OverlayState::FadingOut;
                fadeStartTime = glfwGetTime();
            }
        }
        wasOnBreak = nowOnBreak;

        if (overlayState != OverlayState::Hidden)
        {
            glfwMakeContextCurrent(overlayWindow);
            glfwPollEvents();

            glClearColor(0.f, 0.f, 0.f, 0.f);
            glClear(GL_COLOR_BUFFER_BIT);

            updateOverlay();

            glfwSwapBuffers(overlayWindow);

            if (glfwWindowShouldClose(overlayWindow))
                break;
        }
        else
        {
            Sleep(50);
            glfwPollEvents();
        }
    }
}

void App::shutdown()
{
    tray.remove();
    UI::shutdown();

    if (overlayWindow)
        glfwDestroyWindow(overlayWindow);

    glfwTerminate();
}