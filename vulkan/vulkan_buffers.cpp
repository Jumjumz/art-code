#include "vulkan_buffers.hpp"
#include "vk_types.hpp"

VulkanBuffers::VulkanBuffers(const vk::raii::PhysicalDevice &physical_device,
                             const vk::raii::Device &device)
    : physical_device(physical_device), device(device) {
    canvas_create_image(500, 500);
    canvas_create_image_views();
    canvas_create_buffer();
    canvas_create_sampler();
};

void VulkanBuffers::canvas_create_image(const uint32_t &width,
                                        const uint32_t &height) {
    this->extent = vk::Extent3D{width, height, 1};

    vk::ImageCreateInfo image_info{};
    image_info.imageType = vk::ImageType::e2D;
    image_info.format = vk::Format::eR8G8B8A8Srgb;
    image_info.extent = this->extent;
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;
    image_info.samples = vk::SampleCountFlagBits::e1;
    image_info.tiling = vk::ImageTiling::eOptimal;
    image_info.usage = vk::ImageUsageFlagBits::eColorAttachment |
                       vk::ImageUsageFlagBits::eSampled;
    image_info.initialLayout = vk::ImageLayout::eUndefined;

    this->images = vk::raii::Image{this->device, image_info, nullptr};

    canvas_create_image_memory();
};

void VulkanBuffers::canvas_create_image_memory() {
    auto mem_req = this->images.getMemoryRequirements();

    vk::MemoryAllocateInfo alloc_info{};
    alloc_info.allocationSize = mem_req.size;
    alloc_info.memoryTypeIndex = find_memory_type(
        mem_req.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);

    this->canvas_image_memory =
        vk::raii::DeviceMemory{this->device, alloc_info, nullptr};
    // destroy old memory
    this->images.bindMemory(this->canvas_image_memory, 0);
};

void VulkanBuffers::canvas_create_image_views() {
    this->image_format = vk::Format::eR8G8B8A8Srgb;

    vk::ImageViewCreateInfo image_view_info{};
    image_view_info.format = this->image_format;
    image_view_info.image = *this->images;
    image_view_info.viewType = vk::ImageViewType::e2D;
    image_view_info.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    image_view_info.subresourceRange.baseMipLevel = 0;
    image_view_info.subresourceRange.levelCount = 1;
    image_view_info.subresourceRange.baseArrayLayer = 0;
    image_view_info.subresourceRange.layerCount = 1;

    this->image_views =
        vk::raii::ImageView{this->device, image_view_info, nullptr};
};

void VulkanBuffers::canvas_create_buffer() {
    vk::BufferCreateInfo buffer_info{};
    buffer_info.size = sizeof(ArtboardBuffer);
    buffer_info.usage = vk::BufferUsageFlagBits::eUniformBuffer;
    buffer_info.sharingMode = vk::SharingMode::eExclusive;

    this->canvas_uniform_buffer =
        vk::raii::Buffer{this->device, buffer_info, nullptr};

    // allocate memory to the uniform buffer
    vk::MemoryRequirements mem_requirements =
        this->canvas_uniform_buffer.getMemoryRequirements();

    vk::MemoryAllocateInfo mem_alloc_info{};
    mem_alloc_info.allocationSize = mem_requirements.size;
    mem_alloc_info.memoryTypeIndex =
        find_memory_type(mem_requirements.memoryTypeBits,
                         vk::MemoryPropertyFlagBits::eHostVisible |
                             vk::MemoryPropertyFlagBits::eHostCoherent);

    this->canvas_uniform_buffer_memory =
        vk::raii::DeviceMemory{this->device, mem_alloc_info, nullptr};

    this->canvas_uniform_buffer.bindMemory(this->canvas_uniform_buffer_memory, 0);
    this->canvas_uniform_buffer_mapped =
        this->canvas_uniform_buffer_memory.mapMemory(0, sizeof(ArtboardBuffer));
};

void VulkanBuffers::canvas_create_sampler() {
    vk::SamplerCreateInfo sampler_info{};
    sampler_info.magFilter = vk::Filter::eLinear;
    sampler_info.minFilter = vk::Filter::eLinear;
    sampler_info.addressModeU = vk::SamplerAddressMode::eClampToEdge;
    sampler_info.addressModeV = vk::SamplerAddressMode::eClampToEdge;

    this->canvas_sampler = vk::raii::Sampler{this->device, sampler_info, nullptr};
};

uint32_t VulkanBuffers::find_memory_type(uint32_t type_filter,
                                         vk::MemoryPropertyFlags properties) {
    vk::PhysicalDeviceMemoryProperties mem_properties =
        this->physical_device.getMemoryProperties();

    for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
        if ((type_filter & (1 << i)) &&
            (mem_properties.memoryTypes[i].propertyFlags & properties) ==
                properties)
            return i;
    }

    throw std::runtime_error("Failed to find suitable memory type!");
};
