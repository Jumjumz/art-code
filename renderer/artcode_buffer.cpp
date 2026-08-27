#include "artcode_buffer.hpp"
#include <cstring>

ArtcodeBuffer::ArtcodeBuffer(const vk::raii::PhysicalDevice&             phys_device,
                             const vk::raii::Device&                     device,
                             const vk::raii::Queue&                      graphics_queue,
                             const vk::raii::CommandPool&                cmd_pool,
                             const vk::raii::Image&                      artboard_image,
                             const std::vector<vk::raii::DescriptorSet>& descriptor_sets)
    : phys_device(phys_device),
      device(device),
      graphics_queue(graphics_queue),
      cmd_pool(cmd_pool),
      artboard_image(artboard_image),
      descriptor_sets(descriptor_sets) {};

// NOTE:might need to remove vertex buffer and index buffers
void ArtcodeBuffer::create_vertex_buffer() {
    for (size_t i = 0; i < this->inst_vertex.size(); i++) {
        const auto vertex = this->inst_vertex[i];

        vk::DeviceSize buffer_size = sizeof(vertex[0]) * vertex.size();

        vk::BufferCreateInfo staging_info{};
        staging_info.size        = buffer_size;
        staging_info.usage       = vk::BufferUsageFlagBits::eTransferSrc;
        staging_info.sharingMode = vk::SharingMode::eExclusive;

        vk::raii::Buffer staging_buffer{this->device, staging_info, nullptr};

        vk::MemoryRequirements mem_req_staging = staging_buffer.getMemoryRequirements();

        vk::MemoryAllocateInfo mem_alloc_info{};
        mem_alloc_info.allocationSize  = mem_req_staging.size;
        mem_alloc_info.memoryTypeIndex = find_memory_type(
            mem_req_staging.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible |
                                                vk::MemoryPropertyFlagBits::eHostCoherent);

        vk::raii::DeviceMemory staging_buffer_mem{this->device, mem_alloc_info, nullptr};

        staging_buffer.bindMemory(staging_buffer_mem, 0);

        void* data_staging = staging_buffer_mem.mapMemory(0, staging_info.size);
        memcpy(data_staging, vertex.data(), static_cast<size_t>(staging_info.size));

        staging_buffer_mem.unmapMemory();

        vk::BufferCreateInfo buffer_info{};
        buffer_info.size  = buffer_size;
        buffer_info.usage = vk::BufferUsageFlagBits::eVertexBuffer |
                            vk::BufferUsageFlagBits::eTransferDst;
        buffer_info.sharingMode = vk::SharingMode::eExclusive;

        this->vertex_buffers.push_back(vk::raii::Buffer{this->device, buffer_info, nullptr});

        // use current buffer
        vk::MemoryRequirements mem_req = this->vertex_buffers[i].getMemoryRequirements();

        mem_alloc_info.allocationSize  = mem_req.size;
        mem_alloc_info.memoryTypeIndex = find_memory_type(
            mem_req.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);

        this->vertex_memories.push_back(
            vk::raii::DeviceMemory{this->device, mem_alloc_info, nullptr});

        this->vertex_buffers[i].bindMemory(*this->vertex_memories[i], 0);

        copy_buffer(staging_buffer, this->vertex_buffers[i], staging_info.size);
    }
};

void ArtcodeBuffer::create_index_buffer() {
    for (size_t i = 0; i < this->inst_index.size(); i++) {
        const auto indices = this->inst_index[i];

        const vk::DeviceSize buffer_size = sizeof(indices[0]) * indices.size();

        vk::BufferCreateInfo staging_info{};
        staging_info.size        = buffer_size;
        staging_info.usage       = vk::BufferUsageFlagBits::eTransferSrc;
        staging_info.sharingMode = vk::SharingMode::eExclusive;

        vk::raii::Buffer staging_buffer{this->device, staging_info, nullptr};

        vk::MemoryRequirements mem_req_staging = staging_buffer.getMemoryRequirements();

        vk::MemoryAllocateInfo mem_alloc_info{};
        mem_alloc_info.allocationSize  = mem_req_staging.size;
        mem_alloc_info.memoryTypeIndex = find_memory_type(
            mem_req_staging.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible |
                                                vk::MemoryPropertyFlagBits::eHostCoherent);

        vk::raii::DeviceMemory staging_buffer_mem{this->device, mem_alloc_info, nullptr};

        staging_buffer.bindMemory(staging_buffer_mem, 0);

        void* data = staging_buffer_mem.mapMemory(0, staging_info.size);
        memcpy(data, indices.data(), static_cast<size_t>(staging_info.size));

        staging_buffer_mem.unmapMemory();

        vk::BufferCreateInfo buffer_info{};
        buffer_info.size = buffer_size;
        buffer_info.usage =
            vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst;
        buffer_info.sharingMode = vk::SharingMode::eExclusive;

        this->index_buffers.push_back(vk::raii::Buffer{this->device, buffer_info, nullptr});

        // use current buffer
        vk::MemoryRequirements mem_req = this->index_buffers[i].getMemoryRequirements();
        mem_alloc_info.allocationSize  = mem_req.size;
        mem_alloc_info.memoryTypeIndex = find_memory_type(
            mem_req.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);

        this->index_memories.push_back(
            vk::raii::DeviceMemory{this->device, mem_alloc_info, nullptr});

        this->index_buffers[i].bindMemory(*this->index_memories[i], 0);

        copy_buffer(staging_buffer, this->index_buffers[i], staging_info.size);
    }
};

void ArtcodeBuffer::create_ssbo_buffer() {
    // create buffer per instance
    std::vector<vk::DescriptorBufferInfo> ssbo_infos;
    std::vector<vk::WriteDescriptorSet>   writes;

    // reserve size to avoid seg faults
    ssbo_infos.reserve(this->skew_data.size());
    writes.reserve(this->skew_data.size());

    // creates ssbo buffer per shape instance, meaning every shape has an attached ssbo buffer
    for (size_t i = 0; i < this->skew_data.size(); i++) {
        vk::BufferCreateInfo buffer_info{};
        buffer_info.size        = sizeof(this->skew_data[0]);
        buffer_info.usage       = vk::BufferUsageFlagBits::eStorageBuffer;
        buffer_info.sharingMode = vk::SharingMode::eExclusive;

        this->ssbo_buffers.push_back(vk::raii::Buffer{this->device, buffer_info, nullptr});

        vk::MemoryRequirements mem_req = this->ssbo_buffers[i].getMemoryRequirements();

        vk::MemoryAllocateInfo mem_alloc_info{};
        mem_alloc_info.allocationSize  = mem_req.size;
        mem_alloc_info.memoryTypeIndex = find_memory_type(
            mem_req.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible |
                                        vk::MemoryPropertyFlagBits::eHostCoherent);

        this->ssbo_memories.push_back(
            vk::raii::DeviceMemory{this->device, mem_alloc_info, nullptr});

        this->ssbo_buffers[i].bindMemory(this->ssbo_memories[i], 0);

        // map memory
        void* map_memory = this->ssbo_memories[i].mapMemory(0, buffer_info.size);
        memcpy(map_memory, &this->skew_data[i], buffer_info.size);
        this->ssbo_memories[i].unmapMemory();

        // write to the buffer per instance
        vk::DescriptorBufferInfo ssbo_info{};
        ssbo_info.buffer = *this->ssbo_buffers[i];
        ssbo_info.offset = 0;
        ssbo_info.range  = buffer_info.size;
        ssbo_infos.push_back(ssbo_info);

        vk::WriteDescriptorSet write{};
        write.dstSet          = *this->descriptor_sets[i];
        write.dstBinding      = 1;
        write.dstArrayElement = 0;
        write.descriptorCount = 1;
        write.descriptorType  = vk::DescriptorType::eStorageBuffer;
        write.pBufferInfo     = &ssbo_infos[i];
        writes.push_back(write);
    }
    // update descriptor sets for entire writes
    this->device.updateDescriptorSets(writes, {});
};

[[nodiscard]]
vk::raii::DeviceMemory
ArtcodeBuffer::create_export_image_buffer(const vk::Extent3D&  extent,
                                          const vk::DeviceSize image_size) {
    // wait for the gpu
    this->device.waitIdle();

    vk::BufferCreateInfo buffer_info{};
    buffer_info.size        = image_size;
    buffer_info.usage       = vk::BufferUsageFlagBits::eTransferDst;
    buffer_info.sharingMode = vk::SharingMode::eExclusive;

    vk::raii::Buffer staging_buffer{this->device, buffer_info, nullptr};

    vk::MemoryRequirements mem_req = staging_buffer.getMemoryRequirements();

    vk::MemoryAllocateInfo mem_alloc_info{};
    mem_alloc_info.allocationSize  = mem_req.size;
    mem_alloc_info.memoryTypeIndex = find_memory_type(
        mem_req.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible |
                                    vk::MemoryPropertyFlagBits::eHostCoherent);

    vk::raii::DeviceMemory staging_memory{this->device, mem_alloc_info, nullptr};

    staging_buffer.bindMemory(staging_memory, 0);

    // create command buffer
    vk::CommandBufferAllocateInfo cmd_alloc_info{};
    cmd_alloc_info.commandPool        = this->cmd_pool;
    cmd_alloc_info.level              = vk::CommandBufferLevel::ePrimary;
    cmd_alloc_info.commandBufferCount = 1;

    vk::raii::CommandBuffer cmd =
        std::move(this->device.allocateCommandBuffers(cmd_alloc_info).front());

    // begin commands
    vk::CommandBufferBeginInfo cmd_begin_info{};
    cmd_begin_info.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

    cmd.begin(cmd_begin_info);

    // transition artboard image
    vk::ImageMemoryBarrier barrier{};
    barrier.oldLayout                       = vk::ImageLayout::eUndefined;
    barrier.newLayout                       = vk::ImageLayout::eTransferSrcOptimal;
    barrier.srcQueueFamilyIndex             = vk::QueueFamilyIgnored;
    barrier.dstQueueFamilyIndex             = vk::QueueFamilyIgnored;
    barrier.image                           = *this->artboard_image;
    barrier.subresourceRange.aspectMask     = vk::ImageAspectFlagBits::eColor;
    barrier.subresourceRange.baseMipLevel   = 0;
    barrier.subresourceRange.levelCount     = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount     = 1;
    barrier.srcAccessMask                   = vk::AccessFlagBits::eShaderRead;
    barrier.dstAccessMask                   = vk::AccessFlagBits::eTransferRead;

    cmd.pipelineBarrier(vk::PipelineStageFlagBits::eFragmentShader,
                        vk::PipelineStageFlagBits::eTransfer, {}, {}, {}, barrier);

    // copy image to buffer
    vk::BufferImageCopy region{};
    region.bufferOffset                    = 0;
    region.bufferRowLength                 = 0;
    region.bufferImageHeight               = 0;
    region.imageSubresource.aspectMask     = vk::ImageAspectFlagBits::eColor;
    region.imageSubresource.mipLevel       = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount     = 1;
    region.imageOffset                     = vk::Offset3D{0, 0, 0};
    region.imageExtent                     = extent;

    cmd.copyImageToBuffer(*artboard_image, vk::ImageLayout::eTransferSrcOptimal,
                          *staging_buffer, region);

    // transition again back to gpu
    barrier.image         = *this->artboard_image;
    barrier.oldLayout     = vk::ImageLayout::eTransferSrcOptimal;
    barrier.newLayout     = vk::ImageLayout::eShaderReadOnlyOptimal;
    barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
    barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

    cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
                        vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, barrier);

    cmd.end();

    // submit from gpu to cpu
    vk::SubmitInfo submit_info{};
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers    = &*cmd;
    this->graphics_queue.submit(submit_info, nullptr);
    this->graphics_queue.waitIdle();

    return staging_memory;
};

uint32_t ArtcodeBuffer::find_memory_type(uint32_t                type_filter,
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

void ArtcodeBuffer::copy_buffer(vk::raii::Buffer& src_buffer,
                                vk::raii::Buffer& dst_buffer, vk::DeviceSize size) {
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
