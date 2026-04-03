#include "artboard_settings.hpp"
#include "imgui.h"

ArtboardSettings::ArtboardSettings() {

};

void ArtboardSettings::render() {
    const auto *viewport = ImGui::GetMainViewport();
    const auto work_size = viewport->WorkSize;
    const auto work_pos = viewport->WorkPos;

    const float width = work_size.x * 0.3f;
    const float height = work_size.y;
    const float pos_x = work_size.x - width;
    const float pos_y = work_pos.y;

    ImGui::SetNextWindowSize(ImVec2{width, height});
    ImGui::SetNextWindowPos(ImVec2{pos_x, pos_y});

    // removes padding
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{20, 20});

    ImGui::Begin("##artboard-settings", nullptr,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar |
                     ImGuiWindowFlags_NoScrollWithMouse);

    ImGui::Text("artboard settings");
    ImGui::End();
    ImGui::PopStyleVar();
};
