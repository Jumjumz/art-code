#include "artcode_buffer.hpp"

ArtcodeBuffer::ArtcodeBuffer(const vk::raii::PhysicalDevice& phys_device,
                             const vk::raii::Device&         device,
                             const vk::raii::Queue&          graphics_queue,
                             const vk::raii::CommandPool&    cmd_pool)
    : phys_device(phys_device),
      device(device),
      graphics_queue(graphics_queue),
      cmd_pool(cmd_pool) {};

void ArtcodeBuffer::create_vertex_buffer(const std::vector<glm::vec2>& vertex) {
    const vk::DeviceSize buffer_size = sizeof(vertex[0]) * vertex.size();

    vk::BufferCreateInfo staging_info{};
    staging_info.size        = buffer_size;
    staging_info.usage       = vk::BufferUsageFlagBits::eTransferSrc;
    staging_info.sharingMode = vk::SharingMode::eExclusive;

    vk::raii::Buffer staging_buffer{this->device, staging_info, nullptr};

    vk::MemoryRequirements mem_req_staging = staging_buffer.getMemoryRequirements();

    vk::MemoryAllocateInfo mem_alloc_staging_info{};
    mem_alloc_staging_info.allocationSize  = mem_req_staging.size;
    mem_alloc_staging_info.memoryTypeIndex = findMemoryType(
        mem_req_staging.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible |
                                            vk::MemoryPropertyFlagBits::eHostCoherent);

    vk::raii::DeviceMemory staging_buffer_mem{this->device, mem_alloc_staging_info,
                                              nullptr};

    staging_buffer.bindMemory(staging_buffer_mem, 0);

    void* data_staging = staging_buffer_mem.mapMemory(0, staging_info.size);
    memcpy(data_staging, vertex.data(), static_cast<size_t>(staging_info.size));

    staging_buffer_mem.unmapMemory();

    vk::BufferCreateInfo buffer_info{};
    buffer_info.size = buffer_size;
    buffer_info.usage =
        vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst;
    buffer_info.sharingMode = vk::SharingMode::eExclusive;

    this->vertex_buffer = vk::raii::Buffer{this->device, buffer_info, nullptr};

    vk::MemoryRequirements mem_req = this->vertex_buffer.getMemoryRequirements();
    vk::MemoryAllocateInfo mem_alloc_info{};
    mem_alloc_info.allocationSize = mem_req.size;
    mem_alloc_info.memoryTypeIndex =
        findMemoryType(mem_req.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);

    this->vertex_memory = vk::raii::DeviceMemory{this->device, mem_alloc_info, nullptr};

    this->vertex_buffer.bindMemory(*this->vertex_memory, 0);

    copyBuffer(staging_buffer, this->vertex_buffer, staging_info.size);
};

void ArtcodeBuffer::create_index_buffer(const std::vector<uint32_t>& indices) {
    const vk::DeviceSize buffer_size = sizeof(indices[0]) * indices.size();

    vk::raii::Buffer       staging_buffer({});
    vk::raii::DeviceMemory staging_buffer_mem({});

    createBuffer(buffer_size, vk::BufferUsageFlagBits::eTransferSrc,
                 vk::MemoryPropertyFlagBits::eHostVisible |
                     vk::MemoryPropertyFlagBits::eHostCoherent,
                 staging_buffer, staging_buffer_mem);

    void* data = staging_buffer_mem.mapMemory(0, 0);
    memcpy(data, indices.data(), static_cast<size_t>(0));

    staging_buffer_mem.unmapMemory();

    createBuffer(
        buffer_size,
        vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
        vk::MemoryPropertyFlagBits::eDeviceLocal, this->index_buffer, this->index_memory);

    copyBuffer(staging_buffer, this->index_buffer, buffer_size);
};

uint32_t ArtcodeBuffer::findMemoryType(uint32_t                type_filter,
                                       vk::MemoryPropertyFlags properties) {
    vk::PhysicalDeviceMemoryProperties mem_properties =
        this->phys_device.getMemoryProperties();

    for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
        if ((type_filter & (1 << i)) &&
            (mem_properties.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
    }

    throw std::runtime_error("Failed to find suitable memory type!");
};

void ArtcodeBuffer::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage,
                                 vk::MemoryPropertyFlags properties,
                                 vk::raii::Buffer&       buffer,
                                 vk::raii::DeviceMemory& buffer_memory) {
    vk::BufferCreateInfo buffer_info{};
    buffer_info.size        = size;
    buffer_info.usage       = usage;
    buffer_info.sharingMode = vk::SharingMode::eExclusive;

    buffer = vk::raii::Buffer{this->device, buffer_info, nullptr};

    vk::MemoryRequirements mem_req = buffer.getMemoryRequirements();

    vk::MemoryAllocateInfo mem_alloc_staging_info{};
    mem_alloc_staging_info.allocationSize = mem_req.size;
    mem_alloc_staging_info.memoryTypeIndex =
        findMemoryType(mem_req.memoryTypeBits, properties);

    buffer_memory = vk::raii::DeviceMemory{this->device, mem_alloc_staging_info, nullptr};

    buffer.bindMemory(*buffer_memory, 0);
};

void ArtcodeBuffer::copyBuffer(vk::raii::Buffer& src_buffer, vk::raii::Buffer& dst_buffer,
                               vk::DeviceSize size) {
    vk::CommandBufferAllocateInfo cmd_alloc_info{};
    cmd_alloc_info.commandPool        = this->cmd_pool;
    cmd_alloc_info.level              = vk::CommandBufferLevel::ePrimary;
    cmd_alloc_info.commandBufferCount = 1;

    vk::raii::CommandBuffer cmd_buffer =
        std::move(this->device.allocateCommandBuffers(cmd_alloc_info).front());

    vk::CommandBufferBeginInfo begin_info{};
    begin_info.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    cmd_buffer.begin(begin_info);
    cmd_buffer.copyBuffer(src_buffer, dst_buffer, vk::BufferCopy{0, 0, size});

    cmd_buffer.end();

    vk::SubmitInfo submit_info{};
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers    = &*cmd_buffer;

    this->graphics_queue.submit(submit_info, nullptr);
    this->graphics_queue.waitIdle();
};
