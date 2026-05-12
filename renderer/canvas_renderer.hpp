#pragma once

#include "imgui_impl_glfw.h"
#include "vulkan_buffers.hpp"
#include "vulkan_canvas.hpp"
#include "vulkan_graphics.hpp"

#include <glm/glm.hpp>
#include <memory>

class CanvasRenderer {
  public:
    CanvasRenderer(const vk::raii::PhysicalDevice& physical_device,
                   const vk::raii::Device& device, const int& graphics_family,
                   const int& MAX_FRAMES_IN_FLIGHT, const uint32_t* app_width);

    VulkanBuffers vk_buffers;

    std::unique_ptr<VulkanGraphics> pipeline;
    std::unique_ptr<VulkanCanvas>   canvas_commands;

    bool vulkan_init = true;

    // compiles artcode shader the very first moment a new project is created
    void compile_shader();

    void set_canvas_pipeline();

    void set_canvas_commands();

    void reload_pipeline();

    void workspace_events(GLFWwindow* app_window);

    void canvas_setup(const glm::vec3& artboard_size, bool show_main_ui);

    void record_canvas_command(const uint32_t& current_frame);

    void update_canvas(const vk::raii::Device& device);

  private:
    const vk::raii::PhysicalDevice& physical_device;
    const vk::raii::Device&         device;

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

    const vk::ClearColorValue clear_color = {0.0f, 0.0f, 0.0f, 1.0f};
    const vk::Offset2D        offset      = {0, 0};

    void transition_image(const vk::Image& image, const vk::CommandBuffer& cmd_buffer,
                          const vk::ImageLayout&         old_layout,
                          const vk::ImageLayout&         new_layout,
                          const vk::AccessFlags2&        src_access_mask,
                          const vk::AccessFlags2&        dst_accessmask,
                          const vk::PipelineStageFlags2& src_stage_mask,
                          const vk::PipelineStageFlags2& dst_stage_mask,
                          const vk::ImageAspectFlags&    image_aspect_flags);
};
