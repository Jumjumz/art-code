#pragma once

#include "ui_manager.hpp"
#include "vulkan_buffers.hpp"
#include "vulkan_commands.hpp"
#include "vulkan_context.hpp"
#include "vulkan_graphics.hpp"
#include "vulkan_swapchain.hpp"
#include "window.hpp"

#include <condition_variable>
#include <mutex>
#include <thread>

class ArtCode {
  public:
    ArtCode();

    void run();

  private:
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    Window window;

    VulkanContext ctx{this->window.app_window};

    VulkanSwapchain swapchain{this->ctx.surface,
                              this->ctx.device,
                              this->ctx.config.capabilities,
                              this->ctx.config.chosen_format,
                              this->ctx.config.chosen_present_mode,
                              this->ctx.config.chosen_extent,
                              this->ctx.family_indices.graphics_family,
                              this->ctx.family_indices.present_family,
                              this->ctx.config.image_count};

    VulkanBuffers vk_buffers{this->ctx.physical_device, this->ctx.device};

    VulkanGraphics pipeline{this->ctx.device, this->vk_buffers.image_format};

    VulkanCommands commands{this->ctx.device,
                            this->vk_buffers.canvas_uniform_buffer,
                            this->pipeline.descriptor_set_layout,
                            this->swapchain.resources.images,
                            this->ctx.family_indices.graphics_family,
                            ArtCode::MAX_FRAMES_IN_FLIGHT};

    VkFormat format =
        static_cast<VkFormat>(this->swapchain.resources.image_format);

    vk::Result draw_result;

    uint32_t image_index;

    uint32_t current_frame = 0;

    bool frame_buffer_resize = false;

    const vk::ClearColorValue clear_color = {0.0f, 0.0f, 0.0f, 1.0f};

    const vk::Offset2D offset = {0, 0};

    UIManager ui_manager;

    // multi  threading
    std::mutex canvas_mutex;
    std::condition_variable canvas_cv;
    std::atomic<bool> running = true;

    std::atomic<bool> canvas_ready = true;

    std::thread canvas_thread;

    // key inputs
    bool mouse_in_canvas = false;
    bool ctrl_pressed = false;
    bool spacebar_pressed = false;
    bool left_click_pressed = false;

    void loop();

    void canvas_events();

    void imgui_init();

    void canvas_setup();

    void reset_buffers();

    void submit_buffers(const std::vector<vk::CommandBuffer> &command_buffers);

    void update_canvas();

    void recreate_swapchain();

    void record_canvas_command();

    void record_imgui_command();

    void transition_image_layout(const vk::CommandBuffer &cmd_buffer,
                                 const vk::Image &image,
                                 const vk::ImageLayout &old_layout,
                                 const vk::ImageLayout &new_layout,
                                 const vk::AccessFlags2 &src_access_mask,
                                 const vk::AccessFlags2 &dst_accessmask,
                                 const vk::PipelineStageFlags2 &src_stage_mask,
                                 const vk::PipelineStageFlags2 &dst_stage_mask,
                                 const vk::ImageAspectFlags &image_aspect_flags);

    void clean_swapchain();

    void cleanup();
};
