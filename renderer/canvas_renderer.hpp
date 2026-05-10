#pragma once

#include "vulkan_buffers.hpp"
#include "vulkan_canvas.hpp"
#include "vulkan_graphics.hpp"

class CanvasRenderer {
  public:
    CanvasRenderer(const vk::raii::PhysicalDevice& physical_device,
                   const vk::raii::Device& device, const int& graphics_family,
                   const int& MAX_FRAMES_IN_FLIGHT);

    VulkanBuffers  vk_buffers;
    VulkanGraphics pipeline;
    VulkanCanvas   canvas_commands;

    void record_canvas_command(const uint32_t& current_frame);

    // void update_canvas();

  private:
    const vk::raii::PhysicalDevice& physical_device;
    const vk::raii::Device&         device;

    const int &graphics_family, MAX_FRAMES_IN_FLIGHT;

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
