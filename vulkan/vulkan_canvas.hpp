#pragma once

#include <vulkan/vulkan_raii.hpp>

class VulkanCanvas {
  public:
    VulkanCanvas(const vk::raii::Device&              device,
                 const vk::raii::Buffer&              canvas_uniform_buffer,
                 const vk::raii::DescriptorSetLayout& canvas_descriptor_set_layout,
                 const int& graphics_family, const int& MAX_FRAMES_IN_FLIGHT);
    vk::raii::CommandPool canvas_command_pool = nullptr;

    std::vector<vk::raii::CommandBuffer> canvas_command_buffers;

    vk::raii::DescriptorPool canvas_descriptor_pool = nullptr;

    std::vector<vk::raii::DescriptorSet> canvas_descriptor_set;

  private:
    const vk::raii::Device&              device;
    const vk::raii::Buffer&              canvas_uniform_buffer;
    const vk::raii::DescriptorSetLayout& canvas_descriptor_set_layout;

    const int &graphics_family, MAX_FRAMES_IN_FLIGHT;

    void canvas_create_command_pool();

    void canvas_create_command_buffer();

    void canvas_create_descriptor_pool();

    void canvas_create_descriptor_set();
};
