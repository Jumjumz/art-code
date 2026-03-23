#include "vulkan_commands.hpp"

VulkanCommands::VulkanCommands(const vk::raii::Device &device,
                               const std::vector<vk::Image> &images,
                               const int &graphics_family,
                               const int &MAX_FRAMES_IN_FLIGHT)
    : device(device), images(images), graphics_family(graphics_family),
      MAX_FRAMES_IN_FLIGHT(MAX_FRAMES_IN_FLIGHT) {
    create_command_pool();
    create_command_buffers();
    imgui_create_descriptor_pool();
    create_sync_objects();
};

void VulkanCommands::create_command_pool() {
    vk::CommandPoolCreateInfo pool_info{};
    pool_info.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    pool_info.queueFamilyIndex = static_cast<uint32_t>(this->graphics_family);

    this->command_pool = vk::raii::CommandPool{this->device, pool_info, nullptr};
};

void VulkanCommands::create_command_buffers() {
    this->command_buffers.clear();

    vk::CommandBufferAllocateInfo alloc_info{};
    alloc_info.level = vk::CommandBufferLevel::ePrimary;
    alloc_info.commandPool = this->command_pool;
    alloc_info.commandBufferCount = VulkanCommands::MAX_FRAMES_IN_FLIGHT;

    this->command_buffers = vk::raii::CommandBuffers{
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
