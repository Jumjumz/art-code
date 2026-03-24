#pragma once

#include <vulkan/vulkan_raii.hpp>

class VulkanBuffers {
  public:
    VulkanBuffers(const vk::raii::PhysicalDevice &physical_device,
                  const vk::raii::Device &device);

    vk::raii::Image images = nullptr;

    vk::raii::DeviceMemory canvas_image_memory = nullptr;

    vk::raii::ImageView image_views = nullptr;

    vk::raii::Buffer canvas_uniform_buffer = nullptr;

    vk::raii::DeviceMemory canvas_uniform_buffer_memory = nullptr;

    void *canvas_uniform_buffer_mapped = nullptr;

    vk::Format image_format;

    vk::Extent3D extent;

    void canvas_create_image(const uint32_t &width, const uint32_t &height);

    void canvas_create_image_views();

  private:
    const vk::raii::PhysicalDevice &physical_device;
    const vk::raii::Device &device;

    void canvas_create_image_memory();

    void canvas_create_buffer();

    uint32_t find_memory_type(uint32_t type_filter,
                              vk::MemoryPropertyFlags properties);
};
