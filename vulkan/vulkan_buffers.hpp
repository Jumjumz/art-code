#pragma once

#include <vulkan/vulkan_raii.hpp>

class VulkanBuffers {
  public:
    VulkanBuffers(const vk::raii::PhysicalDevice &physical_device,
                  const vk::raii::Device &device);

    vk::raii::Image images = nullptr;

    vk::raii::DeviceMemory canvas_memory = nullptr;

    vk::raii::ImageView image_views = nullptr;

    vk::Format image_format;

    void canvas_create_image_views(const uint32_t &width, const uint32_t &height);

  private:
    const vk::raii::PhysicalDevice &physical_device;
    const vk::raii::Device &device;

    void canvas_create_memory();

    uint32_t find_memory_type(uint32_t type_filter,
                              vk::MemoryPropertyFlags properties);
};
