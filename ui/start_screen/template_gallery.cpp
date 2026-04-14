#include "template_gallery.hpp"
#include "nav_items.hpp"

TemplateGallery::TemplateGallery() {
    ImGui::FileBrowser file(ImGuiFileBrowserFlags_SelectDirectory |
                                ImGuiFileBrowserFlags_CreateNewDir,
                            this->home);

    this->file_dialog = file;
    this->file_dialog.SetTitle("Create Template Project");
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
    static glm::vec3 artboard_size;
    for (const auto &[text, val] : ArtboardTemplates::TEMPLATES) {
        if (ImGui::Button(text.c_str())) {
            artboard_size = set_artboard_template(val);
            this->file_dialog.Open();
            break;
        }
        ImGui::SameLine();
    }

    ImGui::End();
    ImGui::PopStyleVar();

    // display file modal browser
    this->file_dialog.Display();

    if (this->file_dialog.HasSelected()) {
        if (this->build.set_project_directory(this->file_dialog.GetSelected(),
                                              artboard_size)) {
            // pass path to global project path
            ProjectPath::set_project_path(this->file_dialog.GetSelected());
            this->file_dialog.ClearSelected();

            set_artboard_dimensions(artboard_size);
        } else {
            this->file_dialog.ClearSelected();
            this->file_dialog.Close();
        }
    }
};

glm::vec3 TemplateGallery::set_artboard_template(const TemplateSizes &temp) {
    switch (temp) {
    case TemplateSizes::WEB_COMMON: {
        return {1366.0f, 768.0f, 72.0f};
    }
    case TemplateSizes::WEB_LARGE: {
        return {1920.0f, 1080.0f, 72.0f};
    }
    case TemplateSizes::WEB_MEDIUM: {
        return {1440.0f, 900.0f, 72.0f};
    }
    case TemplateSizes::WEB_MINIMUM: {
        return {1024.0f, 768.0f, 72.0f};
    }
    default:
        return {800.0f, 400.0f, 72.0f};
    }
};

void TemplateGallery::get_artboard_solution() {

};

void TemplateGallery::set_artboard_dimensions(const glm::vec3 &dimensions) {
    this->artboard_size = dimensions;
    this->has_dimensions = true;
};

glm::vec3 TemplateGallery::get_artboard_size() const {
    return this->artboard_size;
}

bool TemplateGallery::dimensions_acquired() const {
    return this->has_dimensions;
}
