#include "development.hpp"
#include "imgui.h"

Development::Development() {

};

void Development::render() {
    auto *viewport = ImGui::GetMainViewport();
    auto work_size = viewport->WorkSize;
    auto work_pos = viewport->WorkPos;

    const float width = work_size.x * 0.4f; // 40%
    const float height = work_size.y;
    const float pos_x = work_size.x - width; // absolute position to the right
    const float pos_y = work_pos.y;

    ImGui::SetNextWindowSize(ImVec2{width, height});
    ImGui::SetNextWindowPos(ImVec2{pos_x, pos_y});

    // removes padding
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});

    ImGui::Begin("##development", nullptr,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar |
                     ImGuiWindowFlags_NoScrollWithMouse);

    // render build panel
    ImGui::BeginChild("##panel", ImVec2{0, 40}, false);
    this->build_panel.render();
    ImGui::EndChild();

    // render text editor
    float console_height = 200.0f;
    float editor_height = ImGui::GetContentRegionAvail().y - console_height;
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 0.0f);
    ImGui::BeginChild("##editor", ImVec2{0, editor_height});
    this->text_editor.render();
    ImGui::EndChild();
    ImGui::PopStyleVar();

    // render console
    ImGui::BeginChild("##console", ImVec2{0, console_height});
    this->console.render();
    ImGui::EndChild();

    ImGui::End();
};
