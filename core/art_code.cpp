#include "art_code.hpp"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

ArtCode::ArtCode() {};

void ArtCode::run() {
    imgui_init();
    loop();
    cleanup();
};

void ArtCode::loop() {
    while (!glfwWindowShouldClose(this->window.app_window)) {
        glfwPollEvents();

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        this->navigation.main_menu_render();

        this->text_editor.text_editor_render();

        ImGui::Render();

        draw_frame();
    }

    this->ctx.device.waitIdle();
};

void ArtCode::imgui_init() {
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForVulkan(this->window.app_window, true);

    // convert raii to c vulkan
    VkInstance instance = *this->ctx.instance;
    VkPhysicalDevice physical_device = *this->ctx.physical_device;
    VkDevice device = *this->ctx.device;
    VkQueue graphics_queue = *this->ctx.graphics_queue;
    VkDescriptorPool descriptor_pool = *this->commands.descriptor_pool;

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
    init_info.DescriptorPool = descriptor_pool;
    init_info.MinImageCount = 2;
    init_info.ImageCount = this->ctx.config.image_count;

    ImGui_ImplVulkan_Init(&init_info);

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

    this->commands.command_buffers[this->current_frame].reset();

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
        &*this->commands.command_buffers[this->current_frame];
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
    auto &cmd = this->commands.command_buffers[this->current_frame];

    VkCommandBuffer cmd_buffer = *cmd;

    cmd.begin({});

    transition_image_layout(this->swapchain.resources.images[image_index],
                            vk::ImageLayout::eUndefined,
                            vk::ImageLayout::eColorAttachmentOptimal, {},
                            vk::AccessFlagBits2::eColorAttachmentWrite,
                            vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                            vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                            vk::ImageAspectFlagBits::eColor);

    // set up color attachment
    vk::ClearValue clear_color = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);

    vk::RenderingAttachmentInfo attachement_info{};
    attachement_info.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    attachement_info.loadOp = vk::AttachmentLoadOp::eClear;
    attachement_info.storeOp = vk::AttachmentStoreOp::eStore;
    attachement_info.clearValue = clear_color;
    attachement_info.imageView =
        this->swapchain.resources.image_views[image_index];

    vk::Offset2D offset = {0, 0};

    vk::RenderingInfo rendering_info{};
    rendering_info.renderArea.offset = offset;
    rendering_info.renderArea.extent = this->swapchain.resources.extent;
    rendering_info.layerCount = 1;
    rendering_info.colorAttachmentCount = 1;
    rendering_info.pColorAttachments = &attachement_info;

    cmd.beginRendering(rendering_info);

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

    this->commands.command_buffers[this->current_frame].pipelineBarrier2(
        dependency_info);
};

void ArtCode::recreate_swapchain() {
    this->ctx.device.waitIdle();

    clean_swapchain();

    this->ctx.create_extent();

    this->swapchain.create_swapchain(this->ctx.config.chosen_extent);

    this->swapchain.create_image_views();
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
