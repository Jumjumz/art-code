#pragma once

#include <vulkan/vulkan_raii.hpp>

class VulkanCommands {
  public:
    VulkanCommands(const vk::raii::Device &device,
                   const std::vector<vk::Image> &images,
                   const int &graphics_family, const int &MAX_FRAMES_IN_FLIGHT);

    vk::raii::CommandPool command_pool = nullptr;

    std::vector<vk::raii::CommandBuffer> command_buffers;
    std::vector<vk::raii::Semaphore> available_semaphores;
    std::vector<vk::raii::Semaphore> finished_semaphores;
    std::vector<vk::raii::Fence> in_flight_fences;

    vk::raii::DescriptorPool imgui_descriptor_pool = nullptr;

    std::vector<vk::raii::DescriptorSet> descriptor_sets;

  private:
    const vk::raii::Device &device;

    const std::vector<vk::Image> &images;

    const int &graphics_family;

    const int &MAX_FRAMES_IN_FLIGHT;

    void create_command_pool();

    void create_command_buffers();

    void imgui_create_descriptor_pool();

    void artcode_create_descriptor_pool();

    void create_sync_objects();
};
