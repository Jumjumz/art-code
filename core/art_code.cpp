#include "art_code.hpp"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "imgui_internal.h"
#include "vk_types.hpp"

#include <cstring>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

ArtCode::ArtCode() {};

void ArtCode::run() {
    canvas_events(); // set the canvas events first
    imgui_init();    // imgui events will be set after canvas
    loop();
    cleanup();
};

void ArtCode::loop() {
    this->canvas_thread = std::thread([this]() -> void {
        while (this->running) {
            std::unique_lock<std::mutex> lock{this->canvas_mutex};
            this->canvas_cv.wait(lock, [this]() -> bool {
                return !this->canvas_ready || !this->running;
            });
            lock.unlock();

            if (!this->running)
                break;

            canvas_setup();

            record_canvas_command();

            // records canvas and runs parallel with the main thread
            {
                std::lock_guard<std::mutex> lock{this->canvas_mutex};
                this->canvas_ready = true;
            }
            this->canvas_cv.notify_one();
        }
    });

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

        reset_buffers();

        // signal canvas thread to start recording
        {
            std::lock_guard<std::mutex> lock{this->canvas_mutex};
            this->canvas_ready = false;
        }
        this->canvas_cv.notify_one();

        // records imgui and runs parallel with canvas
        record_imgui_command();

        // wait for canvas to finish
        std::unique_lock<std::mutex> lock{this->canvas_mutex};
        this->canvas_cv.wait(lock, [this]() -> bool {
            return this->canvas_ready || !this->running;
        });
        lock.unlock();

        const std::vector<vk::CommandBuffer> buffers = {
            *this->commands.canvas_command_buffers[this->current_frame],
            *this->commands.imgui_command_buffers[this->current_frame]};

        submit_buffers(buffers);
    }

    // stop the canvas worker thread
    {
        std::lock_guard<std::mutex> lock{this->canvas_mutex};
        this->running = false;
        this->canvas_ready = false;
    }
    this->canvas_cv.notify_one();
    this->canvas_thread.join();
};

void ArtCode::canvas_setup() {
    const auto artboard_size = this->ui_manager.artboard_size;
    const auto width = artboard_size.x;
    const auto height = artboard_size.y;

    // identity matrix
    glm::mat4 view = glm::mat4(1.0f);

    // get canvas center
    const float center_x = this->vk_buffers.extent.width / 2.0f;
    const float center_y = this->vk_buffers.extent.height / 2.0f;

    // translate to center
    view = glm::translate(view, glm::vec3(center_x, center_y, 0.0f));

    // scale to center
    view =
        glm::scale(view, glm::vec3(CanvasUtils::zoom, CanvasUtils::zoom, 1.0f));

    // tanslate back
    view = glm::translate(view, glm::vec3(-center_x, -center_y, 0.0f));

    // translate to the panning position
    view = glm::translate(
        view, glm::vec3(CanvasUtils::panning.x, CanvasUtils::panning.y, 0.0f));

    ArtboardBuffer a_ubo{
        .proj = glm::ortho(0.0f, (float)this->vk_buffers.extent.width,
                           (float)this->vk_buffers.extent.height, 0.0f, -1.0f,
                           0.0f),
        .view = view,
        .model = glm::translate(
            glm::mat4(1.0f),
            glm::vec3((this->vk_buffers.extent.width - width) / 2,
                      (this->vk_buffers.extent.height - height) / 2, 0.0f)),
        .reso = {width, height}};

    memcpy(this->vk_buffers.canvas_uniform_buffer_mapped, &a_ubo, sizeof(a_ubo));
};

// TODO: REFACTOR: this doesnt need to be in this class
void ArtCode::canvas_events() {
    // calculate mouse movement
    glfwSetCursorPosCallback(
        this->window.app_window,
        [](GLFWwindow *window, double x_pos, double y_pos) -> void {
            auto app =
                reinterpret_cast<ArtCode *>(glfwGetWindowUserPointer(window));
            auto dx = static_cast<float>(x_pos) - CanvasUtils::mouse_last_pos.x;
            auto dy = static_cast<float>(y_pos) - CanvasUtils::mouse_last_pos.y;

            // update last position
            CanvasUtils::mouse_last_pos.x = static_cast<float>(x_pos);
            CanvasUtils::mouse_last_pos.y = static_cast<float>(y_pos);

            if (CanvasUtils::mouse_last_pos.x < app->vk_buffers.extent.width) {
                app->mouse_in_canvas = true;
                // check if space bar and mouse left click is pressed
                if (app->spacebar_pressed && app->left_click_pressed) {
                    CanvasUtils::panning.x += dx * 1.0f;
                    CanvasUtils::panning.y += -dy * 1.0f;

                    // add extra space in both ends of width and height
                    constexpr float EXTRA_SPACE = 50.0f;
                    auto width = static_cast<float>(app->vk_buffers.extent.width);
                    auto height =
                        static_cast<float>(app->vk_buffers.extent.height);

                    CanvasUtils::panning = glm::clamp(
                        CanvasUtils::panning,
                        glm::vec2(-width + EXTRA_SPACE, -height + EXTRA_SPACE),
                        glm::vec2(width + EXTRA_SPACE, height + EXTRA_SPACE));
                }
            } else {
                app->mouse_in_canvas = false;
            }
        });

    // detect if a key is pressed down or release
    glfwSetKeyCallback(this->window.app_window,
                       [](GLFWwindow *window, int key, int scancode, int action,
                          int mods) -> void {
                           auto app = reinterpret_cast<ArtCode *>(
                               glfwGetWindowUserPointer(window));

                           if (app->mouse_in_canvas) {
                               if (key == GLFW_KEY_LEFT_CONTROL) {
                                   if (action == GLFW_PRESS)
                                       app->ctrl_pressed = true;
                                   if (action == GLFW_RELEASE)
                                       app->ctrl_pressed = false;
                               } else if (key == GLFW_KEY_SPACE) {
                                   if (action == GLFW_PRESS)
                                       app->spacebar_pressed = true;
                                   if (action == GLFW_RELEASE)
                                       app->spacebar_pressed = false;
                               }
                           }
                       });

    // scroll
    glfwSetScrollCallback(
        this->window.app_window,
        [](GLFWwindow *window, double x, double y) -> void {
            auto app =
                reinterpret_cast<ArtCode *>(glfwGetWindowUserPointer(window));

            if (app->ctrl_pressed) {
                CanvasUtils::zoom += y * 0.10;
                CanvasUtils::zoom = glm::clamp(CanvasUtils::zoom, 0.1f, 10.0f);
            }
        });

    // panning
    glfwSetMouseButtonCallback(
        this->window.app_window,
        [](GLFWwindow *window, int button, int action, int mods) -> void {
            auto app =
                reinterpret_cast<ArtCode *>(glfwGetWindowUserPointer(window));

            if (button == GLFW_MOUSE_BUTTON_LEFT) {
                if (action == GLFW_PRESS)
                    app->left_click_pressed = true;
                if (action == GLFW_RELEASE)
                    app->left_click_pressed = false;
            }
        });
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

    // init descriptor set
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

void ArtCode::reset_buffers() {
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

    // pass image and result
    this->draw_result = result;
    this->image_index = image_index;

    this->ctx.device.resetFences(
        *this->commands.in_flight_fences[this->current_frame]);

    // resets all command buffers
    this->commands.canvas_command_buffers[this->current_frame].reset();
    this->commands.imgui_command_buffers[this->current_frame].reset();
};

void ArtCode::submit_buffers(const std::vector<vk::CommandBuffer> &command_buffers) {
    vk::PipelineStageFlags destination_stage_mask(
        vk::PipelineStageFlagBits::eColorAttachmentOutput);

    // submit both command buffers
    vk::SubmitInfo submit_info{};
    submit_info.waitSemaphoreCount = 1,
    submit_info.pWaitSemaphores =
        &*this->commands.available_semaphores[this->current_frame];
    submit_info.pWaitDstStageMask = &destination_stage_mask;
    submit_info.commandBufferCount = 2;
    submit_info.pCommandBuffers = command_buffers.data();
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

    this->draw_result = this->ctx.present_queue.presentKHR(present_info);

    if ((this->draw_result == vk::Result::eSuboptimalKHR) ||
        (this->draw_result == vk::Result::eErrorOutOfDateKHR) ||
        this->frame_buffer_resize) {
        this->frame_buffer_resize = false;
        recreate_swapchain();
    } else {
        assert(this->draw_result == vk::Result::eSuccess);
    }

    this->current_frame =
        (this->current_frame + 1) % ArtCode::MAX_FRAMES_IN_FLIGHT;
};

void ArtCode::record_canvas_command() {
    auto &cmd = this->commands.canvas_command_buffers[this->current_frame];

    // render
    cmd.begin({});

    transition_image_layout(cmd, this->vk_buffers.images,
                            vk::ImageLayout::eUndefined,
                            vk::ImageLayout::eColorAttachmentOptimal, {},
                            vk::AccessFlagBits2::eColorAttachmentWrite,
                            vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                            vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                            vk::ImageAspectFlagBits::eColor);

    // prepare to render canvas
    vk::RenderingAttachmentInfo canvas_attachement_info{};
    canvas_attachement_info.imageView = this->vk_buffers.image_views;
    canvas_attachement_info.imageLayout =
        vk::ImageLayout::eColorAttachmentOptimal;
    canvas_attachement_info.loadOp = vk::AttachmentLoadOp::eClear;
    canvas_attachement_info.storeOp = vk::AttachmentStoreOp::eStore;
    canvas_attachement_info.clearValue = this->clear_color;

    vk::RenderingInfo canvas_rendering_info{};
    canvas_rendering_info.renderArea.offset = this->offset;
    canvas_rendering_info.renderArea.extent = vk::Extent2D{
        this->vk_buffers.extent.width, this->vk_buffers.extent.height};
    canvas_rendering_info.layerCount = 1;
    canvas_rendering_info.colorAttachmentCount = 1;
    canvas_rendering_info.pColorAttachments = &canvas_attachement_info;

    // render canvas
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

    cmd.draw(4, 1, 0, 0);

    cmd.endRendering();

    transition_image_layout(cmd, this->vk_buffers.images,
                            vk::ImageLayout::eColorAttachmentOptimal,
                            vk::ImageLayout::eShaderReadOnlyOptimal,
                            vk::AccessFlagBits2::eColorAttachmentWrite, {},
                            vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                            vk::PipelineStageFlagBits2::eBottomOfPipe,
                            vk::ImageAspectFlagBits::eColor);

    cmd.end();
};

void ArtCode::record_imgui_command() {
    auto &cmd = this->commands.imgui_command_buffers[this->current_frame];

    VkCommandBuffer cmd_buffer = *cmd;

    // render
    cmd.begin({});

    // render imgui ui components to swapchain
    transition_image_layout(
        cmd, this->swapchain.resources.images[this->image_index],
        vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal,
        {}, vk::AccessFlagBits2::eColorAttachmentWrite,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::ImageAspectFlagBits::eColor);

    // prepare to render imgui
    vk::RenderingAttachmentInfo imgui_attachement_info{};
    imgui_attachement_info.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    imgui_attachement_info.loadOp = vk::AttachmentLoadOp::eClear;
    imgui_attachement_info.storeOp = vk::AttachmentStoreOp::eStore;
    imgui_attachement_info.clearValue = this->clear_color;
    imgui_attachement_info.imageView =
        this->swapchain.resources.image_views[this->image_index];

    vk::RenderingInfo imgui_rendering_info{};
    imgui_rendering_info.renderArea.offset = this->offset;
    imgui_rendering_info.renderArea.extent = this->swapchain.resources.extent;
    imgui_rendering_info.layerCount = 1;
    imgui_rendering_info.colorAttachmentCount = 1;
    imgui_rendering_info.pColorAttachments = &imgui_attachement_info;

    // render imgui
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

    transition_image_layout(cmd,
                            this->swapchain.resources.images[this->image_index],
                            vk::ImageLayout::eColorAttachmentOptimal,
                            vk::ImageLayout::ePresentSrcKHR,
                            vk::AccessFlagBits2::eColorAttachmentWrite, {},
                            vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                            vk::PipelineStageFlagBits2::eBottomOfPipe,
                            vk::ImageAspectFlagBits::eColor);

    cmd.end();
};

void ArtCode::transition_image_layout(
    const vk::CommandBuffer &cmd_buffer, const vk::Image &image,
    const vk::ImageLayout &old_layout, const vk::ImageLayout &new_layout,
    const vk::AccessFlags2 &src_access_mask,
    const vk::AccessFlags2 &dst_access_mask,
    const vk::PipelineStageFlags2 &src_stage_mask,
    const vk::PipelineStageFlags2 &dst_stage_mask,
    const vk::ImageAspectFlags &image_aspect_flags) {
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

    cmd_buffer.pipelineBarrier2(dependency_info);
};

void ArtCode::recreate_swapchain() {
    this->ctx.device.waitIdle();

    // imgui ui
    clean_swapchain();

    this->ctx.create_extent();

    this->swapchain.create_swapchain(this->ctx.config.chosen_extent);

    this->swapchain.imgui_create_image_views();
};

void ArtCode::clean_swapchain() {
    this->swapchain.resources.image_views.clear();
    this->swapchain.swapchain = nullptr;
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

            // remove the old texture at canvas resize
            ImGui_ImplVulkan_RemoveTexture(CanvasUtils::canvas_texture);

            // run again after texture removal
            CanvasUtils::canvas_texture = ImGui_ImplVulkan_AddTexture(
                *this->vk_buffers.canvas_sampler, *this->vk_buffers.image_views,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }
    }
};

void ArtCode::cleanup() {
    this->ctx.device.waitIdle();

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    clean_swapchain();

    this->window.destroy_window();
};
