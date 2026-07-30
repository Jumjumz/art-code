#pragma once

#include <vulkan/vulkan_raii.hpp>

inline void transition_image_layout(const vk::Image&               image,
                                    const vk::CommandBuffer&       cmd_buffer,
                                    const vk::ImageLayout&         old_layout,
                                    const vk::ImageLayout&         new_layout,
                                    const vk::AccessFlags2&        src_access_mask,
                                    const vk::AccessFlags2&        dst_access_mask,
                                    const vk::PipelineStageFlags2& src_stage_mask,
                                    const vk::PipelineStageFlags2& dst_stage_mask,
                                    const vk::ImageAspectFlags&    image_aspect_flags) {
    vk::ImageMemoryBarrier2 barrier{};
    barrier.srcStageMask        = src_stage_mask;
    barrier.srcAccessMask       = src_access_mask;
    barrier.dstStageMask        = dst_stage_mask;
    barrier.dstAccessMask       = dst_access_mask;
    barrier.oldLayout           = old_layout;
    barrier.newLayout           = new_layout;
    barrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
    barrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
    barrier.image               = image;
    barrier.subresourceRange    = {image_aspect_flags, 0, 1, 0, 1};

    vk::DependencyInfo dependency_info{};
    dependency_info.imageMemoryBarrierCount = 1;
    dependency_info.pImageMemoryBarriers    = &barrier;

    cmd_buffer.pipelineBarrier2(dependency_info);
};
