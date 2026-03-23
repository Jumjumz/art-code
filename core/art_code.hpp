#pragma once

#include "ui_manager.hpp"
#include "vulkan_commands.hpp"
#include "vulkan_context.hpp"
#include "vulkan_graphics.hpp"
#include "vulkan_swapchain.hpp"
#include "window.hpp"

class ArtCode {
  public:
    ArtCode();

    void run();

  private:
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

    VulkanGraphics pipeline{this->ctx.device,
                            this->swapchain.resources.image_format};

    VulkanCommands commands{this->ctx.device, this->swapchain.resources.images,
                            this->ctx.family_indices.graphics_family,
                            ArtCode::MAX_FRAMES_IN_FLIGHT};

    VkFormat format =
        static_cast<VkFormat>(this->swapchain.resources.image_format);

    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;
    uint32_t current_frame = 0;

    bool frame_buffer_resize = false;

    UIManager ui_manager;

    void loop();

    void imgui_init();

    void draw_frame();

    void recreate_swapchain();

    void clean_swapchain();

    void record_command_buffer(uint32_t image_index);

    void transition_image_layout(vk::Image image, vk::ImageLayout old_layout,
                                 vk::ImageLayout new_layout,
                                 vk::AccessFlags2 src_access_mask,
                                 vk::AccessFlags2 dst_accessmask,
                                 vk::PipelineStageFlags2 src_stage_mask,
                                 vk::PipelineStageFlags2 dst_stage_mask,
                                 vk::ImageAspectFlags image_aspect_flags);

    void cleanup();
};
