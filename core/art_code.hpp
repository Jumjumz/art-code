#ifndef ART_CODE_HPP
#define ART_CODE_HPP

#pragma once

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
    uint32_t width = 1440;
    float aspect = 16.0f / 9;

    Window window{this->width, this->aspect};

    VulkanContext ctx{this->window.app_window};

    VulkanSwapchain swapchain{this->ctx.surface,
                              this->ctx.physical_device,
                              this->ctx.device,
                              this->ctx.config.capabilities,
                              this->ctx.config.chosen_format,
                              this->ctx.config.chosen_present_mode,
                              this->ctx.config.chosen_extent,
                              this->ctx.family_indices.graphics_family,
                              this->ctx.family_indices.present_family,
                              this->ctx.config.image_count};

    VulkanGraphics pipeline{this->ctx.physical_device, this->ctx.device,
                            this->swapchain.resources.image_format,
                            this->ctx.family_indices.graphics_family};

    VulkanCommands commands{this->ctx.device, this->swapchain.resources.images,
                            this->pipeline.command_pool,
                            ArtCode::MAX_FRAMES_IN_FLIGHT};

    // convert raii to c vulkan
    VkInstance instance = *this->ctx.instance;
    VkPhysicalDevice physical_device = *this->ctx.physical_device;
    VkDevice device = *this->ctx.device;
    VkQueue graphics_queue = *this->ctx.graphics_queue;
    VkDescriptorPool descriptor_pool = *this->commands.descriptor_pool;
    VkFormat format =
        static_cast<VkFormat>(this->swapchain.resources.image_format);

    VkCommandBuffer cmd_buffer;

    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;
    uint32_t current_frame = 0;

    bool frame_buffer_resize = false;

    void loop();

    void show_ui();

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

#endif // !ART_CODE_HPP
