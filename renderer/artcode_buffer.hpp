#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <vulkan/vulkan_raii.hpp>

class ArtcodeBuffer {
  public:
    ArtcodeBuffer(const vk::raii::PhysicalDevice& phys_device,
                  const vk::raii::Device& device, const vk::raii::Queue& graphics_queue,
                  const vk::raii::CommandPool& cmd_pool);
    vk::raii::Buffer       vertex_buffer = nullptr;
    vk::raii::DeviceMemory vertex_memory = nullptr;

    vk::raii::Buffer       index_buffer = nullptr;
    vk::raii::DeviceMemory index_memory = nullptr;

    void create_vertex_buffer(const std::vector<glm::vec2>& vertex);
    void create_index_buffer(const std::vector<uint32_t>& indices);

  private:
    const vk::raii::PhysicalDevice& phys_device;
    const vk::raii::Device&         device;
    const vk::raii::Queue&          graphics_queue;
    const vk::raii::CommandPool&    cmd_pool;

    uint32_t find_memory_type(uint32_t type_filter, vk::MemoryPropertyFlags properties);

    void create_buffer(vk::DeviceSize size, vk::BufferUsageFlags usage,
                       vk::MemoryPropertyFlags properties, vk::raii::Buffer& buffer,
                       vk::raii::DeviceMemory& buffer_memory);

    void copy_buffer(vk::raii::Buffer& src_buffer, vk::raii::Buffer& dst_buffer,
                     vk::DeviceSize size);
};
