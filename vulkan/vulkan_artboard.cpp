#include "vulkan_artboard.hpp"
#include "vk_types.hpp"

VulkanArtboard::VulkanArtboard(
    const vk::raii::Device& device, const vk::raii::Buffer& artboard_uniform_buffer,
    const vk::raii::DescriptorSetLayout& artboard_descriptor_set_layout,
    const int& graphics_family, const int& MAX_FRAMES_IN_FLIGHT)
    : device(device),
      artboard_uniform_buffer(artboard_uniform_buffer),
      artboard_descriptor_set_layout(artboard_descriptor_set_layout),
      graphics_family(graphics_family),
      MAX_FRAMES_IN_FLIGHT(MAX_FRAMES_IN_FLIGHT) {
    artboard_create_command_pool();
    artboard_create_command_buffer();
    artboard_create_descriptor_pool();
    artboard_create_descriptor_set();
};

void VulkanArtboard::artboard_create_command_pool() {
    vk::CommandPoolCreateInfo pool_info{};
    pool_info.flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    pool_info.queueFamilyIndex = static_cast<uint32_t>(this->graphics_family);

    this->artboard_command_pool = vk::raii::CommandPool{this->device, pool_info, nullptr};
};

void VulkanArtboard::artboard_create_command_buffer() {
    vk::CommandBufferAllocateInfo alloc_info{};
    alloc_info.level              = vk::CommandBufferLevel::ePrimary;
    alloc_info.commandPool        = this->artboard_command_pool;
    alloc_info.commandBufferCount = this->MAX_FRAMES_IN_FLIGHT;

    this->artboard_command_buffers = vk::raii::CommandBuffers{
        this->device,
        alloc_info,
    };
};

void VulkanArtboard::artboard_create_descriptor_pool() {
    vk::DescriptorPoolSize pool_size(vk::DescriptorType::eUniformBuffer,
                                     this->MAX_FRAMES_IN_FLIGHT);

    vk::DescriptorPoolCreateInfo pool_info{};
    pool_info.flags         = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
    pool_info.maxSets       = this->MAX_FRAMES_IN_FLIGHT;
    pool_info.poolSizeCount = 1;
    pool_info.pPoolSizes    = &pool_size;

    this->artboard_descriptor_pool =
        vk::raii::DescriptorPool{this->device, pool_info, nullptr};
};

void VulkanArtboard::artboard_create_descriptor_set() {
    std::vector<vk::DescriptorSetLayout> layouts(this->MAX_FRAMES_IN_FLIGHT,
                                                 *this->artboard_descriptor_set_layout);

    vk::DescriptorSetAllocateInfo set_alloc_info{};
    set_alloc_info.descriptorPool     = *this->artboard_descriptor_pool;
    set_alloc_info.descriptorSetCount = static_cast<uint32_t>(layouts.size());
    set_alloc_info.pSetLayouts        = layouts.data();

    this->artboard_descriptor_set.clear();
    this->artboard_descriptor_set = this->device.allocateDescriptorSets(set_alloc_info);

    vk::DescriptorBufferInfo buffer_info{};
    buffer_info.buffer = *this->artboard_uniform_buffer;
    buffer_info.offset = 0;
    buffer_info.range  = sizeof(ArtboardBuffer);

    vk::WriteDescriptorSet write_desc_set{};
    write_desc_set.dstSet          = this->artboard_descriptor_set[0];
    write_desc_set.dstBinding      = 0;
    write_desc_set.dstArrayElement = 0;
    write_desc_set.descriptorCount = 1;
    write_desc_set.descriptorType  = vk::DescriptorType::eUniformBuffer;
    write_desc_set.pBufferInfo     = &buffer_info;

    this->device.updateDescriptorSets(write_desc_set, {});
};
