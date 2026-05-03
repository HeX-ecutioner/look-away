#include "UI.h"
#include <imgui.h>

namespace UI
{

    void render(TimerManager &timer)
    {
        ImGui::Begin("LookAway");

        if (!timer.isOnBreak())
        {
            int remaining = timer.getRemaining();

            ImGui::Text("Work Session");
            ImGui::Separator();

            ImGui::SetWindowFontScale(2.0f);
            ImGui::Text("%02d:%02d", remaining / 60, remaining % 60);
            ImGui::SetWindowFontScale(1.0f);

            if (ImGui::Button("Take Break Now"))
            {
                timer.forceBreak();
            }
        }
        else
        {
            int remaining = timer.getRemaining();

            ImGui::Text("Break Time!");
            ImGui::Separator();

            ImGui::SetWindowFontScale(2.5f);
            ImGui::Text("%d", remaining);
            ImGui::SetWindowFontScale(1.0f);

            ImGui::Text("Look 20 feet away");

            if (ImGui::Button("Skip Break"))
            {
                timer.skipBreak();
            }
        }

        ImGui::End();
    }
}