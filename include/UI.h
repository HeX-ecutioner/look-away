#pragma once

#include <GLFW/glfw3.h>

namespace UI
{
    bool init(GLFWwindow *window);
    void renderOverlay(float alpha, int remaining, const char *msg);
    void shutdown();
}
