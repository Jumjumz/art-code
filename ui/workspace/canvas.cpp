#include "canvas.hpp"
#include "vk_types.hpp"

Canvas::Canvas() {};

// TODO:update this.. instead of passing the whole canvas as a texture, make it as a
// viewport isntead, pass the artboard as the texture
void Canvas::render() {
    this->viewport  = ImGui::GetMainViewport();
    this->work_size = this->viewport->WorkSize;
    this->work_pos  = this->viewport->WorkPos;

    // takes 60% of the window size, window size can be dynamic but canvas will always takes 60% of it
    ImGui::SetNextWindowSize(ImVec2{this->work_size.x * 0.6f, this->work_size.y});
    ImGui::SetNextWindowPos(ImVec2{this->work_pos.x, this->work_pos.y});

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
    ImGui::Begin("##canvas-begin", nullptr,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoTitleBar);
    const ImVec2 size = ImGui::GetContentRegionAvail();

    // TODO: should be the artboard size
    if (CanvasUtils::canvas_texture != VK_NULL_HANDLE) {
        ImGui::Image((ImTextureID)CanvasUtils::canvas_texture, {1920, 1080});
    }

    ImGui::End();
    ImGui::PopStyleVar();
};
