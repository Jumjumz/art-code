#pragma once

#include <vulkan/vulkan_raii.hpp>

class ArtcodeCommands {
  public:
    ArtcodeCommands(const vk::raii::Device& device, const vk::raii::Buffer& uniform_buffer,
                    const vk::raii::DescriptorSetLayout& descriptor_set,
                    const int& graphics_family, const int& MAX_FRAMES_IN_FLIGHT);
    vk::raii::CommandPool artcode_command_pool = nullptr;

    std::vector<vk::raii::CommandBuffer> artcode_command_buffers;

    vk::raii::DescriptorPool artcode_descriptor_pool = nullptr;

    std::vector<vk::raii::DescriptorSet> artcode_descriptor_set;

  private:
    const vk::raii::Device&              device;
    const vk::raii::Buffer&              uniform_buffer;
    const vk::raii::DescriptorSetLayout& descriptor_set;

    const int &graphics_family, MAX_FRAMES_IN_FLIGHT;

    void artcode_create_command_pool();

    void artcode_create_command_buffer();

    void artcode_create_descriptor_pool();

    void artcode_create_descriptor_set();
};
