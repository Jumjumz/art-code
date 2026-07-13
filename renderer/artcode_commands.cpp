#include "artcode_commands.hpp"
#include "vk_types.hpp"
#include <cstdint>

ArtcodeCommands::ArtcodeCommands(const vk::raii::Device& device,
                                 const vk::raii::Buffer& uniform_buffer,
                                 const vk::raii::DescriptorSetLayout& descriptor_set_layout,
                                 const int& graphics_family,
                                 const int& MAX_FRAMES_IN_FLIGHT)
    : device(device),
      uniform_buffer(uniform_buffer),
      descriptor_set_layout(descriptor_set_layout),
      graphics_family(graphics_family),
      MAX_FRAMES_IN_FLIGHT(MAX_FRAMES_IN_FLIGHT) {
    artcode_create_command_pool();
    artcode_create_command_buffer();
    artcode_create_descriptor_pool();
    artcode_create_descriptor_set();
};

void ArtcodeCommands::artcode_create_command_pool() {
    vk::CommandPoolCreateInfo pool_info{};
    pool_info.flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    pool_info.queueFamilyIndex = static_cast<uint32_t>(this->graphics_family);

    this->artcode_command_pool = vk::raii::CommandPool{this->device, pool_info, nullptr};
};

void ArtcodeCommands::artcode_create_command_buffer() {
    vk::CommandBufferAllocateInfo alloc_info{};
    alloc_info.level              = vk::CommandBufferLevel::ePrimary;
    alloc_info.commandPool        = this->artcode_command_pool;
    alloc_info.commandBufferCount = this->MAX_FRAMES_IN_FLIGHT;

    this->artcode_command_buffers = vk::raii::CommandBuffers{
        this->device,
        alloc_info,
    };
};

// TODO:move this somewhere! or have a better way for other buffers to access this
void ArtcodeCommands::artcode_create_descriptor_pool() {
    std::array<vk::DescriptorPoolSize, 2> pool_size{
        vk::DescriptorPoolSize{vk::DescriptorType::eUniformBuffer,
                               static_cast<uint32_t>(this->MAX_FRAMES_IN_FLIGHT)},
        vk::DescriptorPoolSize{vk::DescriptorType::eStorageBuffer, this->max_instances * 2}};

    vk::DescriptorPoolCreateInfo pool_info{};
    pool_info.flags         = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
    pool_info.maxSets       = this->max_instances;
    pool_info.poolSizeCount = pool_size.size();
    pool_info.pPoolSizes    = pool_size.data();

    this->artcode_descriptor_pool =
        vk::raii::DescriptorPool{this->device, pool_info, nullptr};
};

void ArtcodeCommands::artcode_create_descriptor_set() {
    std::vector<vk::DescriptorSetLayout> layouts(this->max_instances,
                                                 *this->descriptor_set_layout);

    vk::DescriptorSetAllocateInfo set_alloc_info{};
    set_alloc_info.descriptorPool     = *this->artcode_descriptor_pool;
    set_alloc_info.descriptorSetCount = layouts.size();
    set_alloc_info.pSetLayouts        = layouts.data();

    this->artcode_descriptor_sets = this->device.allocateDescriptorSets(set_alloc_info);

    // TODO:move the write of ubo to different place
    for (uint32_t i = 0; i < this->max_instances; i++) {
        vk::DescriptorBufferInfo buffer_info{};
        buffer_info.buffer = *this->uniform_buffer;
        buffer_info.offset = 0;
        buffer_info.range  = sizeof(ArtboardBuffer);

        vk::WriteDescriptorSet write_desc_set{};
        write_desc_set.dstSet          = this->artcode_descriptor_sets[i];
        write_desc_set.dstBinding      = 0;
        write_desc_set.dstArrayElement = 0;
        write_desc_set.descriptorCount = 1;
        write_desc_set.descriptorType  = vk::DescriptorType::eUniformBuffer;
        write_desc_set.pBufferInfo     = &buffer_info;

        this->device.updateDescriptorSets(write_desc_set, {});
    }
};
