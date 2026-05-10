#include "canvas_renderer.hpp"

// TODO:integrate the remaining canvas resources
CanvasRenderer::CanvasRenderer(const vk::raii::PhysicalDevice& physical_device,
                               const vk::raii::Device& device, const int& graphics_family,
                               const int& MAX_FRAMES_IN_FLIGHT)
    : physical_device(physical_device),
      device(device),
      graphics_family(graphics_family),
      MAX_FRAMES_IN_FLIGHT(MAX_FRAMES_IN_FLIGHT),
      vk_buffers(physical_device, device),
      pipeline(device, this->vk_buffers.image_format),
      canvas_commands(device, this->vk_buffers.canvas_uniform_buffer,
                      this->pipeline.descriptor_set_layout, graphics_family,
                      MAX_FRAMES_IN_FLIGHT) {};

// TODO:add function that init pipeliene and commands for conditional rendering of shaders
void CanvasRenderer::record_canvas_command(const uint32_t& current_frame) {
    auto& cmd = this->canvas_commands.canvas_command_buffers[current_frame];

    // render
    cmd.begin({});

    transition_image(this->vk_buffers.images, cmd, vk::ImageLayout::eUndefined,
                     vk::ImageLayout::eColorAttachmentOptimal, {},
                     vk::AccessFlagBits2::eColorAttachmentWrite,
                     vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                     vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                     vk::ImageAspectFlagBits::eColor);

    // prepare to render canvas
    vk::RenderingAttachmentInfo canvas_attachement_info{};
    canvas_attachement_info.imageView   = this->vk_buffers.image_views;
    canvas_attachement_info.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    canvas_attachement_info.loadOp      = vk::AttachmentLoadOp::eClear;
    canvas_attachement_info.storeOp     = vk::AttachmentStoreOp::eStore;
    canvas_attachement_info.clearValue  = this->clear_color;

    vk::RenderingInfo canvas_rendering_info{};
    canvas_rendering_info.renderArea.offset = this->offset;
    canvas_rendering_info.renderArea.extent =
        vk::Extent2D{this->vk_buffers.extent.width, this->vk_buffers.extent.height};
    canvas_rendering_info.layerCount           = 1;
    canvas_rendering_info.colorAttachmentCount = 1;
    canvas_rendering_info.pColorAttachments    = &canvas_attachement_info;

    // render canvas
    cmd.beginRendering(canvas_rendering_info);

    cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, this->pipeline.graphics_pipeline);

    cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, this->pipeline.layout, 0,
                           *this->canvas_commands.canvas_descriptor_set[0], nullptr);

    cmd.setViewport(
        0, vk::Viewport{0.0f, 0.0f, static_cast<float>(this->vk_buffers.extent.width),
                        static_cast<float>(this->vk_buffers.extent.height), 0.0f, 1.0f});

    cmd.setScissor(
        0, vk::Rect2D{vk::Offset2D{0, 0}, vk::Extent2D{this->vk_buffers.extent.width,
                                                       this->vk_buffers.extent.height}});

    cmd.draw(4, 1, 0, 0);

    cmd.endRendering();

    transition_image(
        this->vk_buffers.images, cmd, vk::ImageLayout::eColorAttachmentOptimal,
        vk::ImageLayout::eShaderReadOnlyOptimal, vk::AccessFlagBits2::eColorAttachmentWrite,
        {}, vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::PipelineStageFlagBits2::eBottomOfPipe, vk::ImageAspectFlagBits::eColor);

    cmd.end();
};

void CanvasRenderer::transition_image(const vk::Image&               image,
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
