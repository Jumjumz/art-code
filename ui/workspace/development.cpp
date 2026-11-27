#include "development.hpp"

void Development::render() {
    this->viewport  = ImGui::GetMainViewport();
    this->work_size = this->viewport->WorkSize;
    this->work_pos  = this->viewport->WorkPos;

    const float width = this->work_size.x, height = 20.0f;

    ImGui::SetNextWindowSize(ImVec2{width, this->work_pos.y + height});
    ImGui::SetNextWindowPos(ImVec2{this->work_pos.x, this->work_pos.y});

    // removes padding
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});

    ImGui::Begin("##development", nullptr,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar |
                     ImGuiWindowFlags_NoScrollWithMouse);

    // render build panel
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{10, 0});
    ImGui::BeginChild("##panel", ImVec2{0, 0}, false);
    this->build_panel.render();
    ImGui::EndChild();
    ImGui::PopStyleVar();

    ImGui::End();
    ImGui::PopStyleVar();
};
