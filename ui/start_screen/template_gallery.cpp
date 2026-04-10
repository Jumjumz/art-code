#include "template_gallery.hpp"

TemplateGallery::TemplateGallery() {
    ImGui::FileBrowser file(ImGuiFileBrowserFlags_SelectDirectory |
                                ImGuiFileBrowserFlags_CreateNewDir,
                            this->home);

    this->file_dialog = file;
    this->file_dialog.SetTitle("Create Custom Project");
};

void TemplateGallery::render() {
    this->viewport = ImGui::GetMainViewport();
    this->work_size = viewport->WorkSize;
    this->work_pos = viewport->WorkPos;

    ImGui::SetNextWindowSize(ImVec2{this->work_size.x * 0.7f, this->work_size.y});
    ImGui::SetNextWindowPos(ImVec2{this->work_pos.x, this->work_pos.y});

    // removes padding
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{80, 80});

    ImGui::Begin("##template-gallery", nullptr,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar |
                     ImGuiWindowFlags_NoScrollWithMouse);

    ImGui::Text("Template Gallery");
    for (const auto &[text, val] : ArtboardTemplates::TEMPLATES) {
        if (ImGui::Button(text.c_str())) {
            this->file_dialog.Open();
            this->templates = val;
            break;
        }
        ImGui::SameLine();
    }

    ImGui::End();
    ImGui::PopStyleVar();

    // display file modal browser
    this->file_dialog.Display();

    if (this->file_dialog.HasSelected()) {
        if (this->build.set_project_directory(this->file_dialog.GetSelected())) {
            this->file_dialog.ClearSelected();

            set_artboard_template(this->templates);
        } else {
            this->file_dialog.ClearSelected();
            this->file_dialog.Close();
        }
    }
};

void TemplateGallery::set_artboard_template(const TemplateSizes &temp) {
    switch (temp) {
    case TemplateSizes::WEB_COMMON: {
        this->artboard_size = {1366.0f, 768.0f, 72.0f};
        this->has_dimensions = true;
        break;
    }
    case TemplateSizes::WEB_LARGE: {
        this->artboard_size = {1920.0f, 1080.0f, 72.0f};
        this->has_dimensions = true;
        break;
    }
    case TemplateSizes::WEB_MEDIUM: {
        this->artboard_size = {1440.0f, 900.0f, 72.0f};
        this->has_dimensions = true;
        break;
    }
    case TemplateSizes::WEB_MINIMUM: {
        this->artboard_size = {1024.0f, 768.0f, 72.0f};
        this->has_dimensions = true;
        break;
    }
    default:
        this->artboard_size = {800.0f, 400.0f, 72.0f};
        this->has_dimensions = true;
        break;
    }
};

glm::vec3 TemplateGallery::get_artboard_size() const {
    return this->artboard_size;
}

bool TemplateGallery::dimensions_acquired() const {
    return this->has_dimensions;
}
