#pragma once

#include "artcode_buffer.hpp"
#include "artcode_commands.hpp"
#include "artcode_graphics.hpp"
#include "artcode_instance.hpp"
#include "imgui_impl_glfw.h"
#include "vulkan_buffers.hpp"
#include "vulkan_canvas.hpp"
#include "vulkan_graphics.hpp"

#include <glm/glm.hpp>
#include <memory>

class CanvasRenderer {
  public:
    CanvasRenderer(const vk::raii::PhysicalDevice& physical_device,
                   const vk::raii::Device& device, const vk::raii::Queue& graphics_queue,
                   const int& graphics_family, const int& MAX_FRAMES_IN_FLIGHT,
                   const uint32_t* app_width);

    VulkanBuffers vk_buffers;

    std::unique_ptr<VulkanGraphics>  graphics_pipeline;
    std::unique_ptr<ArtcodeGraphics> artcode_pipeline;
    std::unique_ptr<ArtcodeBuffer>   artcode_buffer;

    std::unique_ptr<VulkanCanvas>    canvas_commands;
    std::unique_ptr<ArtcodeCommands> artcode_commands;

    bool vulkan_init = true;

    // compiles artcode shader the very first moment a new project is created
    void compile_shader();

    void set_canvas_pipeline();

    void set_canvas_commands();

    void reload_pipeline();

    void update_artcode_buffers();

    void save_art();

    bool buffer_exist() const;

    void workspace_events(GLFWwindow* app_window);

    void canvas_setup(const glm::vec3& artboard_size, bool show_main_ui);

    void record_canvas_command(const uint32_t current_frame);

    void record_artcode_command(const uint32_t current_frame);

    void update_canvas();

  private:
    const vk::raii::PhysicalDevice& physical_device;
    const vk::raii::Device&         device;
    const vk::raii::Queue&          graphics_queue;

    const int &graphics_family, &MAX_FRAMES_IN_FLIGHT;

    const uint32_t* app_width;

    // mouse pointers controls
    static float     zoom;
    static glm::vec2 panning;
    static glm::vec2 mouse_last_pos;

    bool show_main_ui = false;
    // key inputs
    bool ctrl_pressed       = false;
    bool spacebar_pressed   = false;
    bool left_click_pressed = false;

    std::vector<PushConstants> push_constants;

    const vk::ClearColorValue clear_color = {0.0f, 0.0f, 0.0f, 1.0f};
    const vk::Offset2D        offset      = {0, 0};
};
