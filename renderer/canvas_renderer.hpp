#pragma once

#include "artcode_buffer.hpp"
#include "artcode_commands.hpp"
#include "artcode_graphics.hpp"
#include "artcode_instance.hpp"
#include "imgui_impl_glfw.h"
#include "vulkan_artboard.hpp"
#include "vulkan_buffers.hpp"
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

    std::unique_ptr<VulkanArtboard>  artboard_commands;
    std::unique_ptr<ArtcodeCommands> artcode_commands;

    bool vulkan_init = true;

    void set_canvas_pipeline();

    void set_canvas_commands();

    void reload_pipeline();

    void update_artcode_buffers();

    void save_art();

    bool buffer_exist() const;

    void canvas_events(GLFWwindow* app_window);

    void artboard_setup(const glm::vec3& artboard_size, bool show_main_ui);

    void record_artboard_command(const uint32_t current_frame);

    void record_artcode_command(const uint32_t current_frame);

    void update_artboard();

  private:
    const vk::raii::PhysicalDevice& physical_device;
    const vk::raii::Device&         device;
    const vk::raii::Queue&          graphics_queue;

    const int &graphics_family, &MAX_FRAMES_IN_FLIGHT;

    const uint32_t* app_width;

    bool show_main_ui = false;
    // key inputs
    bool ctrl_pressed       = false;
    bool spacebar_pressed   = false;
    bool left_click_pressed = false;

    std::vector<PushConstants> push_constants = {};

    size_t inst_size = 0;

    const vk::ClearColorValue clear_color = {1.0f, 1.0f, 1.0f, 1.0f};
    const vk::Offset2D        offset      = {0, 0};
};
