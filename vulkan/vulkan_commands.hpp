#pragma once

#include <vulkan/vulkan_raii.hpp>

class VulkanCommands {
  public:
    VulkanCommands(const vk::raii::Device &device,
                   const vk::raii::Buffer &canvas_uniform_buffer,
                   const vk::raii::DescriptorSetLayout &canvas_descriptor_set_layout,
                   const std::vector<vk::Image> &images,
                   const int &graphics_family, const int &MAX_FRAMES_IN_FLIGHT);

    // imgui
    vk::raii::CommandPool imgui_command_pool = nullptr;

    std::vector<vk::raii::CommandBuffer> imgui_command_buffers;
    std::vector<vk::raii::Semaphore> available_semaphores;
    std::vector<vk::raii::Semaphore> finished_semaphores;
    std::vector<vk::raii::Fence> in_flight_fences;

    vk::raii::DescriptorPool imgui_descriptor_pool = nullptr;

    // canvas
    std::vector<vk::raii::CommandBuffer> canvas_command_buffers;

    vk::raii::CommandPool canvas_command_pool = nullptr;

    vk::raii::DescriptorPool canvas_descriptor_pool = nullptr;

    std::vector<vk::raii::DescriptorSet> canvas_descriptor_set;

  private:
    const vk::raii::Device &device;

    const vk::raii::Buffer &canvas_uniform_buffer;

    const vk::raii::DescriptorSetLayout &canvas_descriptor_set_layout;

    const std::vector<vk::Image> &images;

    const int &graphics_family;

    const int &MAX_FRAMES_IN_FLIGHT;

    void imgui_create_command_pool();

    void imgui_create_command_buffer();

    void imgui_create_descriptor_pool();

    void create_sync_objects();

    void canvas_create_command_pool();

    void canvas_create_command_buffer();

    void canvas_create_descriptor_pool();

    void canvas_create_descriptor_set();
};
