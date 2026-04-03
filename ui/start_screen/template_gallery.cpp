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

    ImGui::Text("template gallery");
    for (const auto &[text, val] : ArtboardTemplates::TEMPLATES) {
        if (ImGui::Button(text.c_str())) {
            web_template_size(val);
        }
        ImGui::SameLine();
    }

    ImGui::End();
    ImGui::PopStyleVar();
};

void TemplateGallery::web_template_size(const TemplateSizes &temp) {
    switch (temp) {
    case TemplateSizes::WEB_COMMON: {
        ArtboardSize::width = 1366.0f;
        ArtboardSize::height = 768.0f;
        ArtboardSize::ppi = 72.0f;
        break;
    }
    case TemplateSizes::WEB_LARGE: {
        ArtboardSize::width = 1920.0f;
        ArtboardSize::height = 1080.0f;
        ArtboardSize::ppi = 72.0f;
        break;
    }
    case TemplateSizes::WEB_MEDIUM: {
        ArtboardSize::width = 1440.0f;
        ArtboardSize::height = 900.0f;
        ArtboardSize::ppi = 72.0f;
        break;
    }
    case TemplateSizes::WEB_MINIMUM: {
        ArtboardSize::width = 1024.0f;
        ArtboardSize::height = 768.0f;
        ArtboardSize::ppi = 72.0f;
        break;
    }
    default:
        ArtboardSize::width = 800.0f;
        ArtboardSize::height = 400.0f;
        ArtboardSize::ppi = 72.0f;
        break;
    }
}
