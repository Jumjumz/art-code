#pragma once

#include <vulkan/vulkan_raii.hpp>

class VulkanCommands {
  public:
    VulkanCommands(const vk::raii::Device &device,
                   const std::vector<vk::Image> &images,
                   const vk::raii::CommandPool &command_pool,
                   const int &MAX_FRAMES_IN_FLIGHT);

    std::vector<vk::raii::CommandBuffer> command_buffers;
    std::vector<vk::raii::Semaphore> available_semaphores;
    std::vector<vk::raii::Semaphore> finished_semaphores;
    std::vector<vk::raii::Fence> in_flight_fences;

    vk::raii::DescriptorPool descriptor_pool = nullptr;

    std::vector<vk::raii::DescriptorSet> descriptor_sets;

  private:
    const vk::raii::Device &device;

    const std::vector<vk::Image> &images;

    const vk::raii::CommandPool &command_pool;

    const int &MAX_FRAMES_IN_FLIGHT;

    void create_descriptor_pool();

    void create_command_buffers();

    void create_sync_objects();
};
