#include "vulkan_buffers.hpp"

VulkanBuffers::VulkanBuffers(const vk::raii::PhysicalDevice &physical_device,
                             const vk::raii::Device &device)
    : physical_device(physical_device), device(device) {};

void VulkanBuffers::canvas_create_image_views(const uint32_t &width,
                                              const uint32_t &height) {
    vk::ImageCreateInfo image_info{};
    image_info.imageType = vk::ImageType::e2D;
    image_info.format = vk::Format::eR8G8B8A8Srgb;
    image_info.extent = vk::Extent3D{width, height, 1};
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;
    image_info.samples = vk::SampleCountFlagBits::e1;
    image_info.tiling = vk::ImageTiling::eOptimal;
    image_info.usage = vk::ImageUsageFlagBits::eColorAttachment |
                       vk::ImageUsageFlagBits::eSampled;
    image_info.initialLayout = vk::ImageLayout::eUndefined;

    this->images = vk::raii::Image{this->device, image_info, nullptr};
};

void VulkanBuffers::canvas_create_memory() {
    auto mem_req = this->images.getMemoryRequirements();

    vk::MemoryAllocateInfo alloc_info{};
    alloc_info.allocationSize = mem_req.size;
    alloc_info.memoryTypeIndex = find_memory_type(
        mem_req.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);

    this->canvas_memory =
        vk::raii::DeviceMemory{this->device, alloc_info, nullptr};
    this->images.bindMemory(this->canvas_memory, 0);
};

uint32_t VulkanBuffers::find_memory_type(uint32_t type_filter,
                                         vk::MemoryPropertyFlags properties) {
    vk::PhysicalDeviceMemoryProperties memProperties =
        this->physical_device.getMemoryProperties();

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((type_filter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) ==
                properties)
            return i;
    }

    throw std::runtime_error("Failed to find suitable memory type!");
};
