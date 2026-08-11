#pragma once

#include <vulkan/vulkan_raii.hpp>

class VulkanArtboard {
  public:
    VulkanArtboard(const vk::raii::Device&              device,
                   const vk::raii::Buffer&              artboard_uniform_buffer,
                   const vk::raii::DescriptorSetLayout& artboard_descriptor_set_layout,
                   const int& graphics_family, const int& MAX_FRAMES_IN_FLIGHT);
    vk::raii::CommandPool artboard_command_pool = nullptr;

    std::vector<vk::raii::CommandBuffer> artboard_command_buffers;

    vk::raii::DescriptorPool artboard_descriptor_pool = nullptr;

    std::vector<vk::raii::DescriptorSet> artboard_descriptor_set;

  private:
    const vk::raii::Device&              device;
    const vk::raii::Buffer&              artboard_uniform_buffer;
    const vk::raii::DescriptorSetLayout& artboard_descriptor_set_layout;

    const int &graphics_family, MAX_FRAMES_IN_FLIGHT;

    void artboard_create_command_pool();

    void artboard_create_command_buffer();

    void artboard_create_descriptor_pool();

    void artboard_create_descriptor_set();
};
