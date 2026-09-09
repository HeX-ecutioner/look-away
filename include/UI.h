#pragma once

#include <GLFW/glfw3.h>

namespace UI
{
    bool init(GLFWwindow *window);
    void renderOverlay(float alpha, int remaining, const char *msg, float skipProgress = 0.0f, bool showHint = true);
    void shutdown();
}
