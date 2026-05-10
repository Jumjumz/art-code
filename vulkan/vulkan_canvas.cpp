#include "vulkan_canvas.hpp"
#include "vk_types.hpp"

VulkanCanvas::VulkanCanvas(const vk::raii::Device& device,
                           const vk::raii::Buffer& canvas_uniform_buffer,
                           const vk::raii::DescriptorSetLayout& canvas_descriptor_set_layout,
                           const int& graphics_family, const int& MAX_FRAMES_IN_FLIGHT)
    : device(device),
      canvas_uniform_buffer(canvas_uniform_buffer),
      canvas_descriptor_set_layout(canvas_descriptor_set_layout),
      graphics_family(graphics_family),
      MAX_FRAMES_IN_FLIGHT(MAX_FRAMES_IN_FLIGHT) {
    canvas_create_command_pool();
    canvas_create_command_buffer();
    canvas_create_descriptor_pool();
    canvas_create_descriptor_set();
};

void VulkanCanvas::canvas_create_command_pool() {
    vk::CommandPoolCreateInfo pool_info{};
    pool_info.flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    pool_info.queueFamilyIndex = static_cast<uint32_t>(this->graphics_family);

    this->canvas_command_pool = vk::raii::CommandPool{this->device, pool_info, nullptr};
};

void VulkanCanvas::canvas_create_command_buffer() {
    vk::CommandBufferAllocateInfo alloc_info{};
    alloc_info.level              = vk::CommandBufferLevel::ePrimary;
    alloc_info.commandPool        = this->canvas_command_pool;
    alloc_info.commandBufferCount = this->MAX_FRAMES_IN_FLIGHT;

    this->canvas_command_buffers = vk::raii::CommandBuffers{
        this->device,
        alloc_info,
    };
};

void VulkanCanvas::canvas_create_descriptor_pool() {
    vk::DescriptorPoolSize poolSize(vk::DescriptorType::eUniformBuffer,
                                    this->MAX_FRAMES_IN_FLIGHT);

    vk::DescriptorPoolCreateInfo poolInfo{};
    poolInfo.flags         = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
    poolInfo.maxSets       = this->MAX_FRAMES_IN_FLIGHT;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes    = &poolSize;

    this->canvas_descriptor_pool =
        vk::raii::DescriptorPool{this->device, poolInfo, nullptr};
};

void VulkanCanvas::canvas_create_descriptor_set() {
    std::vector<vk::DescriptorSetLayout> layouts(this->MAX_FRAMES_IN_FLIGHT,
                                                 *this->canvas_descriptor_set_layout);

    vk::DescriptorSetAllocateInfo set_alloc_info{};
    set_alloc_info.descriptorPool     = *this->canvas_descriptor_pool;
    set_alloc_info.descriptorSetCount = static_cast<uint32_t>(layouts.size());
    set_alloc_info.pSetLayouts        = layouts.data();

    this->canvas_descriptor_set.clear();
    this->canvas_descriptor_set = this->device.allocateDescriptorSets(set_alloc_info);

    vk::DescriptorBufferInfo buffer_info{};
    buffer_info.buffer = *this->canvas_uniform_buffer;
    buffer_info.offset = 0;
    buffer_info.range  = sizeof(ArtboardBuffer);

    vk::WriteDescriptorSet write_desc_set{};
    write_desc_set.dstSet          = this->canvas_descriptor_set[0];
    write_desc_set.dstBinding      = 0;
    write_desc_set.dstArrayElement = 0;
    write_desc_set.descriptorCount = 1;
    write_desc_set.descriptorType  = vk::DescriptorType::eUniformBuffer;
    write_desc_set.pBufferInfo     = &buffer_info;

    this->device.updateDescriptorSets(write_desc_set, {});
};
