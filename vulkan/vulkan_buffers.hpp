#pragma once

#include <vulkan/vulkan_raii.hpp>

class VulkanBuffers {
  public:
    VulkanBuffers(const vk::raii::PhysicalDevice& physical_device,
                  const vk::raii::Device&         device);

    vk::raii::Image        images                         = nullptr;
    vk::raii::DeviceMemory artboard_image_memory          = nullptr;
    vk::raii::ImageView    image_views                    = nullptr;
    vk::raii::Buffer       artboard_uniform_buffer        = nullptr;
    vk::raii::DeviceMemory artboard_uniform_buffer_memory = nullptr;
    vk::raii::Sampler      artboard_sampler               = nullptr;

    void* artboard_uniform_buffer_mapped = nullptr;

    vk::Format image_format;

    vk::Extent3D extent;

    void artboard_create_image(const uint32_t& width, const uint32_t& height);

    void artboard_create_image_views();

  private:
    const vk::raii::PhysicalDevice& physical_device;
    const vk::raii::Device&         device;

    void artboard_create_image_memory();

    void artboard_create_buffer();

    void artboard_create_sampler();

    uint32_t find_memory_type(const uint32_t&                type_filter,
                              const vk::MemoryPropertyFlags& properties);
};
