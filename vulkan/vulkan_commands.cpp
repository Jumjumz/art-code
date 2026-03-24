#include "vulkan_commands.hpp"
#include "vertex.hpp"

VulkanCommands::VulkanCommands(
    const vk::raii::Device &device, const vk::raii::Buffer &canvas_uniform_buffer,
    const vk::raii::DescriptorSetLayout &canvas_descriptor_set_layout,
    const std::vector<vk::Image> &images, const int &graphics_family,
    const int &MAX_FRAMES_IN_FLIGHT)
    : device(device), canvas_uniform_buffer(canvas_uniform_buffer),
      canvas_descriptor_set_layout(canvas_descriptor_set_layout),
      images(images), graphics_family(graphics_family),
      MAX_FRAMES_IN_FLIGHT(MAX_FRAMES_IN_FLIGHT) {
    imgui_create_command_pool();
    create_command_buffer();
    imgui_create_descriptor_pool();
    create_sync_objects();
    canvas_create_descriptor_pool();
    canvas_create_descriptor_set();
};

void VulkanCommands::imgui_create_command_pool() {
    vk::CommandPoolCreateInfo pool_info{};
    pool_info.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    pool_info.queueFamilyIndex = static_cast<uint32_t>(this->graphics_family);

    this->imgui_command_pool =
        vk::raii::CommandPool{this->device, pool_info, nullptr};
};

void VulkanCommands::create_command_buffer() {
    this->imgui_command_buffers.clear();

    vk::CommandBufferAllocateInfo alloc_info{};
    alloc_info.level = vk::CommandBufferLevel::ePrimary;
    alloc_info.commandPool = this->imgui_command_pool;
    alloc_info.commandBufferCount = VulkanCommands::MAX_FRAMES_IN_FLIGHT;

    this->imgui_command_buffers = vk::raii::CommandBuffers{
        this->device,
        alloc_info,
    };
};

void VulkanCommands::imgui_create_descriptor_pool() {
    std::vector<vk::DescriptorPoolSize> pool_sizes = {
        {vk::DescriptorType::eSampler, 1000},
        {vk::DescriptorType::eCombinedImageSampler, 1000},
        {vk::DescriptorType::eSampledImage, 1000},
        {vk::DescriptorType::eStorageImage, 1000},
        {vk::DescriptorType::eUniformBuffer, 1000},
        {vk::DescriptorType::eStorageBuffer, 1000},
        {vk::DescriptorType::eUniformBufferDynamic, 1000},
        {vk::DescriptorType::eInputAttachment, 1000},
        {vk::DescriptorType::eUniformTexelBuffer, 1000},
        {vk::DescriptorType::eStorageTexelBuffer, 1000}};

    vk::DescriptorPoolCreateInfo pool_info{};
    pool_info.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
    pool_info.maxSets = 1000;
    pool_info.poolSizeCount = pool_sizes.size();
    pool_info.pPoolSizes = pool_sizes.data();

    this->imgui_descriptor_pool =
        vk::raii::DescriptorPool{this->device, pool_info, nullptr};
};

void VulkanCommands::create_sync_objects() {
    assert(this->available_semaphores.empty() &&
           this->finished_semaphores.empty() && this->in_flight_fences.empty());

    for (size_t i = 0; i < this->images.size(); i++) {
        this->finished_semaphores.emplace_back(this->device,
                                               vk::SemaphoreCreateInfo());
    }

    for (size_t i = 0; i < (size_t)VulkanCommands::MAX_FRAMES_IN_FLIGHT; i++) {
        this->available_semaphores.emplace_back(this->device,
                                                vk::SemaphoreCreateInfo());

        vk::FenceCreateInfo fence_info{};
        fence_info.flags = vk::FenceCreateFlagBits::eSignaled;

        this->in_flight_fences.emplace_back(this->device, fence_info);
    }
};

void VulkanCommands::canvas_create_descriptor_pool() {
    vk::DescriptorPoolSize poolSize(vk::DescriptorType::eUniformBuffer,
                                    VulkanCommands::MAX_FRAMES_IN_FLIGHT);

    vk::DescriptorPoolCreateInfo poolInfo{};
    poolInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
    poolInfo.maxSets = VulkanCommands::MAX_FRAMES_IN_FLIGHT;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;

    this->canvas_descriptor_pool =
        vk::raii::DescriptorPool{this->device, poolInfo, nullptr};
};

void VulkanCommands::canvas_create_descriptor_set() {
    std::vector<vk::DescriptorSetLayout> layouts(
        this->MAX_FRAMES_IN_FLIGHT, *this->canvas_descriptor_set_layout);

    vk::DescriptorSetAllocateInfo set_alloc_info{};
    set_alloc_info.descriptorPool = *this->canvas_descriptor_pool;
    set_alloc_info.descriptorSetCount = static_cast<uint32_t>(layouts.size());
    set_alloc_info.pSetLayouts = layouts.data();

    this->canvas_descriptor_set.clear();
    this->canvas_descriptor_set =
        this->device.allocateDescriptorSets(set_alloc_info);

    vk::DescriptorBufferInfo buffer_info{};
    buffer_info.buffer = *this->canvas_uniform_buffer;
    buffer_info.offset = 0;
    buffer_info.range = sizeof(UniformBufferObject);

    vk::WriteDescriptorSet write_desc_set{};
    write_desc_set.dstSet = this->canvas_descriptor_set[0];
    write_desc_set.dstBinding = 0;
    write_desc_set.dstArrayElement = 0;
    write_desc_set.descriptorCount = 1;
    write_desc_set.descriptorType = vk::DescriptorType::eUniformBuffer;
    write_desc_set.pBufferInfo = &buffer_info;

    this->device.updateDescriptorSets(write_desc_set, {});
};
