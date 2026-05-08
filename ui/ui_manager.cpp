#include "ui_manager.hpp"
#include "imgui_impl_vulkan.h"
#include "imgui_internal.h"
#include "start_screen/artboard_settings.hpp"
#include "start_screen/template_gallery.hpp"
#include "vk_types.hpp"
#include "workspace/canvas.hpp"
#include "workspace/development.hpp"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <iostream>

// definition
float     UIManager::zoom           = 1;
glm::vec2 UIManager::panning        = {0.0f, 0.0f};
glm::vec2 UIManager::mouse_last_pos = {0.0f, 0.0f};

UIManager::UIManager(const Window& window, const VulkanContext& ctx,
                     VulkanBuffers& vk_buffers)
    : window(window),
      ctx(ctx),
      vk_buffers(vk_buffers) {
    this->artboard_size = {0.0f, 0.0f, 0.0f};
    // init start screen
    this->start_ui.push_back(std::make_unique<TemplateGallery>());
    this->start_ui.push_back(std::make_unique<ArtboardSettings>());
};

void UIManager::render() {
    // render main navigation panel
    this->main_nav.render();

    if (!this->show_main_ui) {
        for (const auto& start_screen : this->start_ui) {
            // immidiately render the start screen ui's
            start_screen->render();

            // check if artboard dimensions is ready
            if (start_screen->dimensions_acquired()) {
                this->artboard_size = start_screen->get_artboard_size();

                // clear the vector and its uniq ptr
                this->start_ui.clear();
                this->start_ui.shrink_to_fit();

                // it only needs to init if start ui is done rendering and data is ready
                this->workspace_ui.push_back(std::make_unique<Development>());
                this->workspace_ui.push_back(std::make_unique<Canvas>());

                this->show_main_ui = true;
                break;
            }
        }
    } else {
        for (const auto& workspace : this->workspace_ui) {
            workspace->render();
        }
    }
};

void UIManager::canvas_setup() {
    const auto artboard_size = this->artboard_size;
    const auto width         = artboard_size.x;
    const auto height        = artboard_size.y;

    // identity matrix
    glm::mat4 view = glm::mat4(1.0f);

    // get canvas center
    const float center_x = this->vk_buffers.extent.width / 2.0f;
    const float center_y = this->vk_buffers.extent.height / 2.0f;

    // translate to center
    view = glm::translate(view, glm::vec3(center_x, center_y, 0.0f));

    // scale to center
    view = glm::scale(view, glm::vec3(UIManager::zoom, UIManager::zoom, 1.0f));

    // tanslate back
    view = glm::translate(view, glm::vec3(-center_x, -center_y, 0.0f));

    // translate to the panning position
    view =
        glm::translate(view, glm::vec3(UIManager::panning.x, UIManager::panning.y, 0.0f));

    ArtboardBuffer a_ubo{
        .proj = glm::ortho(0.0f, (float)this->vk_buffers.extent.width,
                           (float)this->vk_buffers.extent.height, 0.0f, -1.0f, 0.0f),
        .view = view,
        .model =
            glm::translate(glm::mat4(1.0f),
                           glm::vec3((this->vk_buffers.extent.width - width) / 2,
                                     (this->vk_buffers.extent.height - height) / 2, 0.0f)),
        .reso = {width, height}};

    memcpy(this->vk_buffers.canvas_uniform_buffer_mapped, &a_ubo, sizeof(a_ubo));
};
// FIXME:this crashes the applicaiton, race condition or vk_buffers is not initialized is my suspicion
void UIManager::workspace_events() {
    // calculate mouse movement
    glfwSetCursorPosCallback(
        this->window.app_window, [](GLFWwindow* window, double x_pos, double y_pos) -> void {
            auto app = reinterpret_cast<UIManager*>(glfwGetWindowUserPointer(window));

            auto dx = static_cast<float>(x_pos) - UIManager::mouse_last_pos.x;
            auto dy = static_cast<float>(y_pos) - UIManager::mouse_last_pos.y;

            // update last position
            UIManager::mouse_last_pos.x = static_cast<float>(x_pos);
            UIManager::mouse_last_pos.y = static_cast<float>(y_pos);

            if (UIManager::mouse_last_pos.x < app->vk_buffers.extent.width) {
                app->mouse_in_canvas = true;
                std::cout << app->mouse_in_canvas << std::endl;
                // check if space bar and mouse left click is pressed
                if (app->spacebar_pressed && app->left_click_pressed) {
                    UIManager::panning.x += dx * 1.0f;
                    UIManager::panning.y += -dy * 1.0f;

                    // add extra space in both ends of width and height
                    constexpr float EXTRA_SPACE = 50.0f;
                    auto width  = static_cast<float>(app->vk_buffers.extent.width);
                    auto height = static_cast<float>(app->vk_buffers.extent.height);

                    UIManager::panning =
                        glm::clamp(UIManager::panning,
                                   glm::vec2(-width + EXTRA_SPACE, -height + EXTRA_SPACE),
                                   glm::vec2(width + EXTRA_SPACE, height + EXTRA_SPACE));
                }
            } else {
                app->mouse_in_canvas = false;
            }
        });

    // detect if a key is pressed down or release
    glfwSetKeyCallback(
        this->window.app_window,
        [](GLFWwindow* window, int key, int scancode, int action, int mods) -> void {
            auto app = reinterpret_cast<UIManager*>(glfwGetWindowUserPointer(window));

            if (app->mouse_in_canvas) {
                if (key == GLFW_KEY_LEFT_CONTROL) {
                    if (action == GLFW_PRESS)
                        app->ctrl_pressed = true;
                    if (action == GLFW_RELEASE)
                        app->ctrl_pressed = false;
                } else if (key == GLFW_KEY_SPACE) {
                    if (action == GLFW_PRESS)
                        app->spacebar_pressed = true;
                    if (action == GLFW_RELEASE)
                        app->spacebar_pressed = false;
                }
            }
            // for text editor
            if (app->ctrl_pressed) {
                if (key == GLFW_KEY_S) {
                    if (action == GLFW_PRESS) {
                        TextEditorUtils::file_save = true;
                    }
                }
            }
        });

    // scroll
    glfwSetScrollCallback(
        this->window.app_window, [](GLFWwindow* window, double x, double y) -> void {
            auto app = reinterpret_cast<UIManager*>(glfwGetWindowUserPointer(window));

            if (app->ctrl_pressed) {
                app->zoom += y * 0.10;
                app->zoom  = glm::clamp(UIManager::zoom, 0.1f, 10.0f);
            }
        });

    // panning
    glfwSetMouseButtonCallback(
        this->window.app_window,
        [](GLFWwindow* window, int button, int action, int mods) -> void {
            auto app = reinterpret_cast<UIManager*>(glfwGetWindowUserPointer(window));

            if (button == GLFW_MOUSE_BUTTON_LEFT) {
                if (action == GLFW_PRESS)
                    app->left_click_pressed = true;
                if (action == GLFW_RELEASE)
                    app->left_click_pressed = false;
            }
        });
};

void UIManager::update_canvas() {
    if (this->show_main_ui) {
        const auto canvas = ImGui::FindWindowByName("##canvas-begin");
        if (canvas) {
            const auto width  = static_cast<uint32_t>(canvas->Size.x);
            const auto height = static_cast<uint32_t>(canvas->Size.y);

            if (width != this->vk_buffers.extent.width ||
                height != this->vk_buffers.extent.height) {
                this->ctx.device.waitIdle();

                this->vk_buffers.canvas_create_image(width, height);
                this->vk_buffers.canvas_create_image_views();

                // remove the old texture at canvas resize
                ImGui_ImplVulkan_RemoveTexture(CanvasUtils::canvas_texture);

                // run again after texture removal
                CanvasUtils::canvas_texture = ImGui_ImplVulkan_AddTexture(
                    *this->vk_buffers.canvas_sampler, *this->vk_buffers.image_views,
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            }
        }
    }
};
