#include "UI.h"
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <cstdio>

namespace UI
{
    static ImFont* fontHuge = nullptr;
    static ImFont* fontTitle = nullptr;
    static ImFont* fontHint = nullptr;

    bool init(GLFWwindow* window)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = nullptr;

        ImGui::StyleColorsDark();
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowBorderSize = 0.f;
        style.WindowRounding = 0.f;
        style.WindowPadding = ImVec2(0, 0);

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 330");

        auto tryFont = [&](ImFontAtlas* atlas, float size, const char* name) -> ImFont*
        {
            char path[512];
            snprintf(path, sizeof(path), "fonts/%s", name);
            ImFont* f = atlas->AddFontFromFileTTF(path, size);
            if (f) return f;

            snprintf(path, sizeof(path), "external/imgui/misc/fonts/%s", name);
            f = atlas->AddFontFromFileTTF(path, size);
            return f;
        };

        fontHuge = tryFont(io.Fonts, 140.f, "Cousine-Regular.ttf");
        fontTitle = tryFont(io.Fonts, 56.f, "Roboto-Medium.ttf");
        fontHint = tryFont(io.Fonts, 18.f, "Roboto-Medium.ttf");

        if (!fontHuge) fontHuge = io.Fonts->AddFontDefault();
        if (!fontTitle) fontTitle = io.Fonts->AddFontDefault();
        if (!fontHint) fontHint = io.Fonts->AddFontDefault();

        io.Fonts->Build();
        return true;
    }

    void renderOverlay(float alpha, int remaining, const char* msg, float skipProgress, bool showHint)
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGuiIO& io = ImGui::GetIO();
        ImVec2 disp = io.DisplaySize;

        ImGui::SetNextWindowPos({0, 0}); // Fullscreen window
        ImGui::SetNextWindowSize(disp);
        ImGui::SetNextWindowBgAlpha(0.f);

        ImGui::Begin("##overlay", nullptr,
            ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoBringToFrontOnFocus);

        ImDrawList* dl = ImGui::GetWindowDrawList();
        float cx = disp.x * 0.5f, cy = disp.y * 0.5f;

        char timeBuf[16]; // Digits
        snprintf(timeBuf, sizeof(timeBuf), "%d", remaining);
        
        ImGui::PushFont(fontHuge);
        ImVec2 digitSz = ImGui::CalcTextSize(timeBuf);
        ImVec2 digitPos = ImVec2(cx - digitSz.x * 0.5f, cy - digitSz.y * 0.5f - 40.f);
        dl->AddText(fontHuge, 140.f, digitPos, IM_COL32(255, 255, 255, (int)(alpha * 255)), timeBuf);
        ImGui::PopFont();

        if (msg)
        {
            ImGui::PushFont(fontTitle);
            ImVec2 msgSz = ImGui::CalcTextSize(msg);
            ImVec2 msgPos = ImVec2(cx - msgSz.x * 0.5f, cy + 80.f);
            dl->AddText(fontTitle, 56.f, msgPos, IM_COL32(230, 230, 230, (int)(alpha * 220)), msg);
            ImGui::PopFont();
        }

        if (showHint)
        {
            ImGui::PushFont(fontHint);
            const char* hintText = (skipProgress > 0.0f) ? "Skipping break..." : "Hold Esc for 2 seconds to skip";
            ImVec2 hintSz = ImGui::CalcTextSize(hintText);
            float hintY = disp.y - 70.f;
            ImVec2 hintPos = ImVec2(cx - hintSz.x * 0.5f, hintY);
            dl->AddText(fontHint, 18.f, hintPos, IM_COL32(180, 180, 180, (int)(alpha * 140)), hintText);
            ImGui::PopFont();

            if (skipProgress > 0.0f)
            {
                float barWidth = 180.f;
                float barHeight = 4.f;
                float barX = cx - barWidth * 0.5f;
                float barY = hintY + hintSz.y + 8.f;

                // Background track
                dl->AddRectFilled(ImVec2(barX, barY), ImVec2(barX + barWidth, barY + barHeight),
                                  IM_COL32(60, 60, 60, (int)(alpha * 180)), 2.0f);

                // Progress fill
                float fillWidth = barWidth * (skipProgress > 1.0f ? 1.0f : skipProgress);
                dl->AddRectFilled(ImVec2(barX, barY), ImVec2(barX + fillWidth, barY + barHeight),
                                  IM_COL32(220, 220, 220, (int)(alpha * 240)), 2.0f);
            }
        }

        ImGui::End();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void shutdown()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
}
