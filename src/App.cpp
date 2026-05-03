#include "App.h"
#include <glad/glad.h> // MUST come before GLFW
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include "UI.h"
#include <iostream>

// Fullscreen toggle
void setFullscreen(GLFWwindow *window, bool fullscreen)
{
    static int wx, wy, ww, wh;

    if (fullscreen)
    {
        GLFWmonitor *monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode *mode = glfwGetVideoMode(monitor);

        glfwGetWindowPos(window, &wx, &wy);
        glfwGetWindowSize(window, &ww, &wh);

        glfwSetWindowMonitor(window, monitor, 0, 0,
                             mode->width, mode->height,
                             mode->refreshRate);
    }
    else
    {
        glfwSetWindowMonitor(window, nullptr,
                             wx, wy, ww, wh, 0);
    }
}

bool App::init()
{
    if (!glfwInit())
        return false;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    window = glfwCreateWindow(800, 400, "LookAway", nullptr, nullptr);
    if (!window)
        return false;

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        return false;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    return true;
}

void App::run()
{
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        timer.update();

        bool shouldFullscreen = timer.isOnBreak();
        if (shouldFullscreen != isFullscreen)
        {
            setFullscreen(window, shouldFullscreen);
            isFullscreen = shouldFullscreen;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (timer.isOnBreak())
        {
            // FULLSCREEN OVERLAY
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);

            ImGui::Begin("Overlay", nullptr,
                         ImGuiWindowFlags_NoDecoration |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoBackground |
                             ImGuiWindowFlags_NoInputs);

            ImDrawList *draw = ImGui::GetBackgroundDrawList();

            draw->AddRectFilled(
                ImVec2(0, 0),
                ImGui::GetIO().DisplaySize,
                IM_COL32(0, 0, 0, 180));

            int remaining = timer.getRemaining();

            ImVec2 center = ImGui::GetIO().DisplaySize;
            center.x *= 0.5f;
            center.y *= 0.5f;

            ImGui::SetCursorPos(ImVec2(center.x - 100, center.y - 50));

            ImGui::SetWindowFontScale(3.5f);
            ImGui::Text("%d", remaining);
            ImGui::SetWindowFontScale(1.0f);

            ImGui::SetCursorPos(ImVec2(center.x - 140, center.y + 20));
            ImGui::Text("Look 20 feet away");

            ImGui::End();
        }
        else
        {
            UI::render(timer);
        }

        ImGui::Render();
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }
}

void App::shutdown()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}