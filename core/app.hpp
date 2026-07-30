#pragma once

#include "canvas_renderer.hpp"
#include "ui_manager.hpp"
#include "vulkan_commands.hpp"
#include "vulkan_context.hpp"
#include "vulkan_swapchain.hpp"
#include "window.hpp"

#include <condition_variable>
#include <mutex>
#include <thread>

class Application {
  public:
    Application();

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

    VulkanCommands commands{this->ctx.device, this->swapchain.resources.images,
                            this->ctx.family_indices.graphics_family,
                            Application::MAX_FRAMES_IN_FLIGHT};

    VkFormat format = static_cast<VkFormat>(this->swapchain.resources.image_format);

    vk::Result draw_result;

    uint32_t image_index, current_frame = 0;

    bool frame_buffer_resize = false;

    const vk::ClearColorValue clear_color = {0.0f, 0.0f, 0.0f, 1.0f};
    const vk::Offset2D        offset      = {0, 0};

    UIManager ui_manager;

    CanvasRenderer canvas{
        this->ctx.physical_device,         this->ctx.device,
        this->ctx.graphics_queue,          this->ctx.family_indices.graphics_family,
        Application::MAX_FRAMES_IN_FLIGHT, &this->swapchain.resources.extent.width};

    // multi  threading
    std::mutex              canvas_mutex;
    std::condition_variable canvas_cv;
    std::atomic<bool>       running      = true;
    std::atomic<bool>       canvas_ready = true;

    std::thread canvas_thread;

    void loop();

    void imgui_init();

    void reset_buffers();

    void submit_buffers(const std::vector<vk::CommandBuffer>& command_buffers);

    void recreate_swapchain();

    void record_imgui_command();

    void clean_swapchain();

    void cleanup();
};
