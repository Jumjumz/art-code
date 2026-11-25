#include "development.hpp"

// TODO:remove this!
void Development::render() {
    this->viewport  = ImGui::GetMainViewport();
    this->work_size = this->viewport->WorkSize;
    this->work_pos  = this->viewport->WorkPos;

    const float width  = this->work_size.x * 0.40f; // 40%
    const float height = this->work_size.y;
    const float pos_x  = this->work_size.x - width; // absolute position to the right
    const float pos_y  = this->work_pos.y;

    ImGui::SetNextWindowSize(ImVec2{width, height});
    ImGui::SetNextWindowPos(ImVec2{pos_x, pos_y});

    // removes padding
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});

    ImGui::Begin("##development", nullptr,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar |
                     ImGuiWindowFlags_NoScrollWithMouse);

    // render build panel
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{10, 0});
    ImGui::BeginChild("##panel", ImVec2{0, 40}, false);
    this->build_panel.render();
    ImGui::EndChild();
    ImGui::PopStyleVar();

    // TODO:console should be a dialog
    // render console
    ImGui::BeginChild("##console", ImVec2{0, 200});
    this->console.render();
    ImGui::EndChild();

    ImGui::End();
    ImGui::PopStyleVar();
};
