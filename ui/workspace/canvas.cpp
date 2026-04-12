#include "canvas.hpp"
#include "vk_types.hpp"

Canvas::Canvas() {};

void Canvas::render() {
    this->viewport = ImGui::GetMainViewport();
    this->work_size = this->viewport->WorkSize;
    this->work_pos = this->viewport->WorkPos;

    ImGui::SetNextWindowSize(ImVec2{this->work_size.x * 0.6f, this->work_size.y});
    ImGui::SetNextWindowPos(ImVec2{this->work_pos.x, this->work_pos.y});

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
    ImGui::Begin("##canvas-begin", nullptr,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoTitleBar);
    const ImVec2 size = ImGui::GetContentRegionAvail();

    if (CanvasUtils::canvas_texture != VK_NULL_HANDLE) {
        ImGui::Image((ImTextureID)CanvasUtils::canvas_texture, size);
    }

    ImGui::End();
    ImGui::PopStyleVar();
};
