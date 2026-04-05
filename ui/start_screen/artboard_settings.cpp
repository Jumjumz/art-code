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

    // init values
    static float ab_width = 1920.0f;
    static float ab_height = 1080.0f;

    // removes padding
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{20.0f, 20.0f});

    ImGui::Begin("##artboard-settings", nullptr,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar |
                     ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::Text("Artboard Settings");
    ImGui::DragFloat("Width", &ab_width, 1.0f, 10.0f, 5000.0f, "%.0f");
    ImGui::DragFloat("Height", &ab_height, 1.0f, 10.0f, 5000.0f, "%.0f");

    if (ImGui::Button("Create")) {
        set_artboard_custom(glm::vec3{ab_width, ab_height, 72.0f});
    }

    ImGui::End();
    ImGui::PopStyleVar();
};

void ArtboardSettings::set_artboard_custom(const glm::vec3 &dimensions) {
    ArtboardSettings::artboard_size = dimensions;
    ArtboardSettings::has_dimensions = true;
};

glm::vec3 ArtboardSettings::get_artboard_size() const {
    return ArtboardSettings::artboard_size;
}

bool ArtboardSettings::dimensions_acquired() const {
    return ArtboardSettings::has_dimensions;
}
