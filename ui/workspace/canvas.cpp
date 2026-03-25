#include "canvas.hpp"
#include "imgui.h"
#include "vk_types.hpp"

Canvas::Canvas() {

};

void Canvas::render() {
    auto *viewport = ImGui::GetMainViewport();
    auto work_size = viewport->WorkSize;
    auto work_pos = viewport->WorkPos;

    ImGui::SetNextWindowSize(ImVec2{work_size.x * 0.6f, work_size.y});
    ImGui::SetNextWindowPos(ImVec2{work_pos.x, work_pos.y});

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
    ImGui::Begin("##canvas-begin", nullptr,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoTitleBar);
    ImVec2 size = ImGui::GetContentRegionAvail();

    if (CanvasUtils::canvas_texture != VK_NULL_HANDLE) {
        ImGui::Image((ImTextureID)CanvasUtils::canvas_texture, size);
    }

    ImGui::End();
    ImGui::PopStyleVar();
};
