#include "art_code.hpp"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "imgui_internal.h"
#include "vk_types.hpp"

#include <cstring>
#include <glm/gtc/matrix_transform.hpp>

ArtCode::ArtCode() {};

void ArtCode::run() {
    imgui_init();
    loop();
    cleanup();
};

void ArtCode::loop() {
    while (!glfwWindowShouldClose(this->window.app_window)) {
        glfwWaitEvents();

        // update canvas and texture first
        update_canvas();

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // render all ui components
        this->ui_manager.render();

        ImGui::Render();

        draw_frame();
    }
};

void ArtCode::canvas_setup() {
    // model, view and camera perspective
    UniformBufferObject ubo{
        .model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)),
        .view = glm::lookAt(glm::vec3{0.0f, 0.0f, 0.0f},
                            glm::vec3(0.0f, 0.0f, 0.0f),
                            glm::vec3(0.0f, 1.0f, 0.0f)),
        .proj = glm::perspective(glm::radians(45.0f), this->window.aspect_ratio,
                                 0.1f, 10.0f)};

    ubo.proj[1][1] *= -1;

    memcpy(this->vk_buffers.canvas_uniform_buffer_mapped, &ubo, sizeof(ubo));
};

void ArtCode::imgui_init() {
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForVulkan(this->window.app_window, true);

    // convert raii to c vulkan
    VkInstance instance = *this->ctx.instance;
    VkPhysicalDevice physical_device = *this->ctx.physical_device;
    VkDevice device = *this->ctx.device;
    VkQueue graphics_queue = *this->ctx.graphics_queue;
    VkDescriptorPool imgui_descriptor_pool =
        *this->commands.imgui_descriptor_pool;

    ImGui_ImplVulkan_InitInfo init_info{};
    init_info.UseDynamicRendering = true;
    init_info.PipelineInfoMain.PipelineRenderingCreateInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    init_info.PipelineInfoMain.PipelineRenderingCreateInfo.colorAttachmentCount =
        1;
    init_info.PipelineInfoMain.PipelineRenderingCreateInfo.pColorAttachmentFormats =
        &this->format;
    init_info.Instance = instance;
    init_info.PhysicalDevice = physical_device;
    init_info.Device = device;
    init_info.Queue = graphics_queue;
    init_info.DescriptorPool = imgui_descriptor_pool;
    init_info.MinImageCount = 2;
    init_info.ImageCount = this->ctx.config.image_count;

    ImGui_ImplVulkan_Init(&init_info);
    // descriptor set
    CanvasUtils::canvas_texture = ImGui_ImplVulkan_AddTexture(
        *this->vk_buffers.canvas_sampler, *this->vk_buffers.image_views,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    glfwSetWindowUserPointer(this->window.app_window, this);
    glfwSetFramebufferSizeCallback(
        this->window.app_window,
        [](GLFWwindow *window, int width, int height) -> void {
            auto app =
                reinterpret_cast<ArtCode *>(glfwGetWindowUserPointer(window));
            app->frame_buffer_resize = true;
        });
};

void ArtCode::draw_frame() {
    auto fence_result = this->ctx.device.waitForFences(
        *this->commands.in_flight_fences[this->current_frame], vk::True,
        UINT64_MAX);

    if (fence_result != vk::Result::eSuccess)
        throw std::runtime_error("Failed to wait for fence!");

    auto [result, image_index] = this->swapchain.swapchain.acquireNextImage(
        UINT64_MAX, *this->commands.available_semaphores[this->current_frame],
        nullptr);

    if (result == vk::Result::eErrorOutOfDateKHR) {
        recreate_swapchain();
        return;
    } else if (result != vk::Result::eSuccess &&
               result != vk::Result::eSuboptimalKHR) {
        assert(result == vk::Result::eTimeout || result == vk::Result::eNotReady);
        throw std::runtime_error("Failed to acquire swapchain image!");
    }

    this->ctx.device.resetFences(
        *this->commands.in_flight_fences[this->current_frame]);

    this->commands.imgui_command_buffers[this->current_frame].reset();

    canvas_setup();

    record_command_buffer(image_index);

    vk::PipelineStageFlags destination_stage_mask(
        vk::PipelineStageFlagBits::eColorAttachmentOutput);

    vk::SubmitInfo submit_info{};
    submit_info.waitSemaphoreCount = 1,
    submit_info.pWaitSemaphores =
        &*this->commands.available_semaphores[this->current_frame];
    submit_info.pWaitDstStageMask = &destination_stage_mask;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers =
        &*this->commands.imgui_command_buffers[this->current_frame];
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores =
        &*this->commands.finished_semaphores[this->current_frame];

    this->ctx.graphics_queue.submit(
        submit_info, *this->commands.in_flight_fences[this->current_frame]);

    vk::PresentInfoKHR present_info{};
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores =
        &*this->commands.finished_semaphores[this->current_frame];
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &*this->swapchain.swapchain;
    present_info.pImageIndices = &image_index;

    result = this->ctx.present_queue.presentKHR(present_info);

    if ((result == vk::Result::eSuboptimalKHR) ||
        (result == vk::Result::eErrorOutOfDateKHR) || this->frame_buffer_resize) {
        this->frame_buffer_resize = false;
        recreate_swapchain();
    } else {
        assert(result == vk::Result::eSuccess);
    }

    this->current_frame =
        (this->current_frame + 1) % ArtCode::MAX_FRAMES_IN_FLIGHT;
};

void ArtCode::record_command_buffer(uint32_t image_index) {
    auto &cmd = this->commands.imgui_command_buffers[this->current_frame];

    VkCommandBuffer cmd_buffer = *cmd;

    // render canvas
    cmd.begin({});

    transition_image_layout(this->vk_buffers.images, vk::ImageLayout::eUndefined,
                            vk::ImageLayout::eColorAttachmentOptimal, {},
                            vk::AccessFlagBits2::eColorAttachmentWrite,
                            vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                            vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                            vk::ImageAspectFlagBits::eColor);

    // set up color attachment
    vk::ClearValue clear_color = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);

    vk::Offset2D offset = {0, 0};

    vk::RenderingAttachmentInfo canvas_attachement_info{};
    canvas_attachement_info.imageView = this->vk_buffers.image_views;
    canvas_attachement_info.imageLayout =
        vk::ImageLayout::eColorAttachmentOptimal;
    canvas_attachement_info.loadOp = vk::AttachmentLoadOp::eClear;
    canvas_attachement_info.storeOp = vk::AttachmentStoreOp::eStore;
    canvas_attachement_info.clearValue = clear_color;

    vk::RenderingInfo canvas_rendering_info{};
    canvas_rendering_info.renderArea.offset = offset;
    canvas_rendering_info.renderArea.extent = vk::Extent2D{
        this->vk_buffers.extent.width, this->vk_buffers.extent.height};
    canvas_rendering_info.layerCount = 1;
    canvas_rendering_info.colorAttachmentCount = 1;
    canvas_rendering_info.pColorAttachments = &canvas_attachement_info;

    cmd.beginRendering(canvas_rendering_info);

    cmd.bindPipeline(vk::PipelineBindPoint::eGraphics,
                     this->pipeline.graphics_pipeline);

    cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                           this->pipeline.layout, 0,
                           *this->commands.canvas_descriptor_set[0], nullptr);

    cmd.setViewport(
        0, vk::Viewport{
               0.0f, 0.0f, static_cast<float>(this->vk_buffers.extent.width),
               static_cast<float>(this->vk_buffers.extent.height), 0.0f, 1.0f});

    cmd.setScissor(0, vk::Rect2D{vk::Offset2D{0, 0},
                                 vk::Extent2D{this->vk_buffers.extent.width,
                                              this->vk_buffers.extent.height}});

    cmd.draw(3, 1, 0, 0);

    cmd.endRendering();

    transition_image_layout(this->vk_buffers.images,
                            vk::ImageLayout::eColorAttachmentOptimal,
                            vk::ImageLayout::eShaderReadOnlyOptimal,
                            vk::AccessFlagBits2::eColorAttachmentWrite, {},
                            vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                            vk::PipelineStageFlagBits2::eBottomOfPipe,
                            vk::ImageAspectFlagBits::eColor);

    // render imgui ui components to swapchain
    transition_image_layout(this->swapchain.resources.images[image_index],
                            vk::ImageLayout::eUndefined,
                            vk::ImageLayout::eColorAttachmentOptimal, {},
                            vk::AccessFlagBits2::eColorAttachmentWrite,
                            vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                            vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                            vk::ImageAspectFlagBits::eColor);

    vk::RenderingAttachmentInfo imgui_attachement_info{};
    imgui_attachement_info.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    imgui_attachement_info.loadOp = vk::AttachmentLoadOp::eClear;
    imgui_attachement_info.storeOp = vk::AttachmentStoreOp::eStore;
    imgui_attachement_info.clearValue = clear_color;
    imgui_attachement_info.imageView =
        this->swapchain.resources.image_views[image_index];

    vk::RenderingInfo imgui_rendering_info{};
    imgui_rendering_info.renderArea.offset = offset;
    imgui_rendering_info.renderArea.extent = this->swapchain.resources.extent;
    imgui_rendering_info.layerCount = 1;
    imgui_rendering_info.colorAttachmentCount = 1;
    imgui_rendering_info.pColorAttachments = &imgui_attachement_info;

    cmd.beginRendering(imgui_rendering_info);

    cmd.setViewport(
        0,
        vk::Viewport{0.0f, 0.0f,
                     static_cast<float>(this->swapchain.resources.extent.width),
                     static_cast<float>(this->swapchain.resources.extent.height),
                     0.0f, 1.0f});

    cmd.setScissor(
        0, vk::Rect2D{vk::Offset2D{0, 0}, this->swapchain.resources.extent});

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd_buffer,
                                    VK_NULL_HANDLE);

    cmd.endRendering();

    transition_image_layout(this->swapchain.resources.images[image_index],
                            vk::ImageLayout::eColorAttachmentOptimal,
                            vk::ImageLayout::ePresentSrcKHR,
                            vk::AccessFlagBits2::eColorAttachmentWrite, {},
                            vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                            vk::PipelineStageFlagBits2::eBottomOfPipe,
                            vk::ImageAspectFlagBits::eColor);

    cmd.end();
};

void ArtCode::transition_image_layout(vk::Image image, vk::ImageLayout old_layout,
                                      vk::ImageLayout new_layout,
                                      vk::AccessFlags2 src_access_mask,
                                      vk::AccessFlags2 dst_access_mask,
                                      vk::PipelineStageFlags2 src_stage_mask,
                                      vk::PipelineStageFlags2 dst_stage_mask,
                                      vk::ImageAspectFlags image_aspect_flags) {
    vk::ImageMemoryBarrier2 barrier{};
    barrier.srcStageMask = src_stage_mask;
    barrier.srcAccessMask = src_access_mask;
    barrier.dstStageMask = dst_stage_mask;
    barrier.dstAccessMask = dst_access_mask;
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    barrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
    barrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
    barrier.image = image;
    barrier.subresourceRange = {image_aspect_flags, 0, 1, 0, 1};

    vk::DependencyInfo dependency_info{};
    dependency_info.imageMemoryBarrierCount = 1;
    dependency_info.pImageMemoryBarriers = &barrier;

    this->commands.imgui_command_buffers[this->current_frame].pipelineBarrier2(
        dependency_info);
};

void ArtCode::recreate_swapchain() {
    this->ctx.device.waitIdle();

    // imgui ui
    clean_swapchain();

    this->ctx.create_extent();

    this->swapchain.create_swapchain(this->ctx.config.chosen_extent);

    this->swapchain.imgui_create_image_views();

    // render ui with the new size immidiately <- this doesnt work
    ImGuiIO &io = ImGui::GetIO();
    io.DisplaySize = ImVec2{float(this->swapchain.resources.extent.width),
                            float(this->swapchain.resources.extent.height)};
};

void ArtCode::clean_swapchain() {
    this->swapchain.resources.image_views.clear();
    this->swapchain.swapchain = nullptr;
};

void ArtCode::cleanup() {
    this->ctx.device.waitIdle();

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    clean_swapchain();

    this->window.destroy_window();
};

void ArtCode::update_canvas() {
    auto canvas = ImGui::FindWindowByName("##canvas-begin");
    if (canvas) {
        auto width = static_cast<uint32_t>(canvas->Size.x);
        auto height = static_cast<uint32_t>(canvas->Size.y);

        if (width != this->vk_buffers.extent.width ||
            height != this->vk_buffers.extent.height) {
            this->ctx.device.waitIdle();

            this->vk_buffers.canvas_create_image(width, height);
            this->vk_buffers.canvas_create_image_views();

            // run again after removal descriptor set
            CanvasUtils::canvas_texture = ImGui_ImplVulkan_AddTexture(
                *this->vk_buffers.canvas_sampler, *this->vk_buffers.image_views,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }
    }
};
