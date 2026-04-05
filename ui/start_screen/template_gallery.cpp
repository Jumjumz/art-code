#include "template_gallery.hpp"
#include "imgui.h"

TemplateGallery::TemplateGallery() {

};

void TemplateGallery::render() {
    const auto *viewport = ImGui::GetMainViewport();
    const auto work_size = viewport->WorkSize;
    const auto work_pos = viewport->WorkPos;

    ImGui::SetNextWindowSize(ImVec2{work_size.x * 0.7f, work_size.y});
    ImGui::SetNextWindowPos(ImVec2{work_pos.x, work_pos.y});

    // removes padding
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{80, 80});

    ImGui::Begin("##template-gallery", nullptr,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar |
                     ImGuiWindowFlags_NoScrollWithMouse);

    ImGui::Text("Template Gallery");
    for (const auto &[text, val] : ArtboardTemplates::TEMPLATES) {
        if (ImGui::Button(text.c_str())) {
            set_artboard_template(val);
        }
        ImGui::SameLine();
    }

    ImGui::End();
    ImGui::PopStyleVar();
};

void TemplateGallery::set_artboard_template(const TemplateSizes &temp) {
    switch (temp) {
    case TemplateSizes::WEB_COMMON: {
        TemplateGallery::artboard_size = {1366.0f, 768.0f, 72.0f};
        TemplateGallery::has_dimensions = true;
        break;
    }
    case TemplateSizes::WEB_LARGE: {
        TemplateGallery::artboard_size = {1920.0f, 1080.0f, 72.0f};
        TemplateGallery::has_dimensions = true;
        break;
    }
    case TemplateSizes::WEB_MEDIUM: {
        TemplateGallery::artboard_size = {1440.0f, 900.0f, 72.0f};
        TemplateGallery::has_dimensions = true;
        break;
    }
    case TemplateSizes::WEB_MINIMUM: {
        TemplateGallery::artboard_size = {1024.0f, 768.0f, 72.0f};
        TemplateGallery::has_dimensions = true;
        break;
    }
    default:
        TemplateGallery::artboard_size = {800.0f, 400.0f, 72.0f};
        TemplateGallery::has_dimensions = true;
        break;
    }
};

glm::vec3 TemplateGallery::get_artboard_size() const {
    return TemplateGallery::artboard_size;
}

bool TemplateGallery::dimensions_acquired() const {
    return TemplateGallery::has_dimensions;
}
