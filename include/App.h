#pragma once

#define GLFW_INCLUDE_NONE   // 🔥 THIS FIXES EVERYTHING
#include <GLFW/glfw3.h>

#include "TimerManager.h"

class App
{
public:
    bool init();
    void run();
    void shutdown();

private:
    GLFWwindow *window;
    TimerManager timer;
    bool isFullscreen = false;
};