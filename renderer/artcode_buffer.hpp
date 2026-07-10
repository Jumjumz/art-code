#pragma once

#include "artcode_instance.hpp"
#include <glm/glm.hpp>
#include <vector>
#include <vulkan/vulkan_raii.hpp>

class ArtcodeBuffer {
  public:
    ArtcodeBuffer(const vk::raii::PhysicalDevice& phys_device,
                  const vk::raii::Device& device, const vk::raii::Queue& graphics_queue,
                  const vk::raii::CommandPool&                cmd_pool,
                  const std::vector<vk::raii::DescriptorSet>& descriptor_sets);
    std::vector<vk::raii::Buffer>       vertex_buffers;
    std::vector<vk::raii::DeviceMemory> vertex_memories;

    std::vector<vk::raii::Buffer>       index_buffers;
    std::vector<vk::raii::DeviceMemory> index_memories;

    std::vector<vk::raii::Buffer>       ssbo_buffers;
    std::vector<vk::raii::DeviceMemory> ssbo_memories;

    std::vector<vk::raii::Buffer>       handle_buffers;
    std::vector<vk::raii::DeviceMemory> handle_memories;

    std::vector<std::vector<glm::vec2>> inst_vertex;
    std::vector<std::vector<uint32_t>>  inst_index;

    std::vector<SkewData> skew_data;
    std::vector<Handles>  handle_data;

    void create_vertex_buffer();

    void create_index_buffer();

    void create_ssbo_buffer();

    void create_pen_buffer();

  private:
    const vk::raii::PhysicalDevice& phys_device;
    const vk::raii::Device&         device;
    const vk::raii::Queue&          graphics_queue;
    const vk::raii::CommandPool&    cmd_pool;

    const std::vector<vk::raii::DescriptorSet>& descriptor_sets;

    uint32_t find_memory_type(uint32_t type_filter, vk::MemoryPropertyFlags properties);

    void copy_buffer(vk::raii::Buffer& src_buffer, vk::raii::Buffer& dst_buffer,
                     vk::DeviceSize size);
};
