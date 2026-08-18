#include "canvas.hpp"
#include "vk_types.hpp"

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
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar |
                     ImGuiWindowFlags_NoScrollWithMouse);

    ImDrawList*  draw_list = ImGui::GetWindowDrawList();
    const ImVec2 panel_pos = ImGui::GetCursorScreenPos();
    const ImVec2 size      = ImGui::GetContentRegionAvail();
    // uses global variable
    const auto& artboard = Artboard::get_artboard_size();
    const float width    = artboard.x;
    const float height   = artboard.y;

    // canvas background
    draw_list->AddRectFilled(panel_pos, ImVec2(panel_pos.x + size.x, panel_pos.y + size.y),
                             IM_COL32(0, 0, 0, 255));

    // render artboard as a rect
    // calculate artboard position based on zoom/pan and size of the canvas
    const float ab_x = panel_pos.x + (size.x - width * CanvasControls::zoom) / 2.0f +
                       CanvasControls::panning.x;
    const float ab_y = panel_pos.y + (size.y - height * CanvasControls::zoom) / 2.0f -
                       CanvasControls::panning.y;
    const float ab_w = width * CanvasControls::zoom;
    const float ab_h = height * CanvasControls::zoom;

    draw_list->AddRectFilled(ImVec2(ab_x, ab_y), ImVec2(ab_x + ab_w, ab_y + ab_h),
                             IM_COL32(255, 255, 255, 255));

    if (ArtboardUtils::artboard_texture != VK_NULL_HANDLE) {
        // Texture on top of the rect
        draw_list->AddImage((ImTextureID)ArtboardUtils::artboard_texture,
                            ImVec2(ab_x, ab_y), ImVec2(ab_x + ab_w, ab_y + ab_h));
    }

    ImGui::End();
    ImGui::PopStyleVar();
};
