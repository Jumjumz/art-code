#include "vulkan_commands.hpp"
#include <vulkan/vulkan_raii.hpp>

VulkanCommands::VulkanCommands(const vk::raii::Device &device,
                               const std::vector<vk::Image> &images,
                               const vk::raii::CommandPool &command_pool,
                               const int &MAX_FRAMES_IN_FLIGHT)
    : device(device), images(images), command_pool(command_pool),
      MAX_FRAMES_IN_FLIGHT(MAX_FRAMES_IN_FLIGHT) {
    create_descriptor_pool();
    create_command_buffers();
    create_sync_objects();
};

void VulkanCommands::create_descriptor_pool() {
    vk::DescriptorPoolSize pool_size(vk::DescriptorType::eUniformBuffer,
                                     VulkanCommands::MAX_FRAMES_IN_FLIGHT);

    vk::DescriptorPoolCreateInfo pool_info{};
    pool_info.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
    pool_info.maxSets = VulkanCommands::MAX_FRAMES_IN_FLIGHT;
    pool_info.poolSizeCount = 1;
    pool_info.pPoolSizes = &pool_size;

    this->descriptor_pool =
        vk::raii::DescriptorPool{this->device, pool_info, nullptr};
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
