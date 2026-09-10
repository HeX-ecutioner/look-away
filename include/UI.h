#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace UI
{
    bool init(GLFWwindow *window);
    void beginFrame();
    void renderOverlay(GLFWwindow *window, float alpha, int remaining, const char *msg, float skipProgress = 0.0f, bool showHint = true);
    void shutdown();
}
