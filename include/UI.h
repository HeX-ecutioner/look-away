#pragma once

#include <GLFW/glfw3.h>

namespace UI
{
    bool init(GLFWwindow* window);
    // msg: the string to display. If null, displays nothing.
    void renderOverlay(float alpha, int remaining, const char* msg);
    void shutdown();
}
