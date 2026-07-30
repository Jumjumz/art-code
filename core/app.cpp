#include "app.hpp"
#include "artcode_instance.hpp"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "nav_items.hpp"
#include "transition_image.hpp"
#include "vk_types.hpp"

Application::Application() {};

void Application::run() {
    // set the workspace events first
    this->canvas.workspace_events(this->window.app_window);
    imgui_init();
    loop();
    cleanup();
};

void Application::loop() {
    this->canvas_thread = std::thread([this]() -> void {
        while (this->running) {
            std::unique_lock<std::mutex> lock{this->canvas_mutex};
            this->canvas_cv.wait(
                lock, [this]() -> bool { return !this->canvas_ready || !this->running; });
            lock.unlock();

            if (!this->running)
                break;

            // ensure canvas functions are only executed if canvas commands is not nullptr
            if (this->ui_manager.show_main_ui && this->canvas.canvas_commands) {
                this->canvas.canvas_setup(this->ui_manager.artboard_size,
                                          this->ui_manager.show_main_ui);
                this->canvas.record_canvas_command(this->current_frame);
                if (this->canvas.buffer_exist())
                    // artcode command
                    this->canvas.record_artcode_command(this->current_frame);
            }

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

        // init canvas vulkan resources once
        if (this->ui_manager.show_main_ui && this->canvas.vulkan_init) {
            // compile the artcode shader first to generate a .spv file
            if (ProjectPath::fresh_project) {
                this->canvas.compile_shader();
                ProjectPath::fresh_project = false;
            }

            // init pipeline and commands
            this->canvas.set_canvas_pipeline();
            this->canvas.set_canvas_commands();
            // set to false to not run this if block after init
            this->canvas.vulkan_init = false;
        }

        // update canvas and texture first
        if (this->ui_manager.show_main_ui && this->canvas.canvas_commands) {
            if (ShadersCompiled::compiled) {
                this->canvas.reload_pipeline();
                // update artcode buffer
                this->canvas.update_artcode_buffers();

                ShadersCompiled::compiled = false;
            }
            this->canvas.update_canvas();
        }

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // render all ui components
        this->ui_manager.render();

        ImGui::Render();

        reset_buffers();
        std::vector<vk::CommandBuffer> buffers;

        // pre allocate to 3 as that is the max number of cmd buffers there is
        if (buffers.size() == 0)
            buffers.reserve(3);

        if (this->ui_manager.show_main_ui && this->canvas.canvas_commands) {
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
            this->canvas_cv.wait(
                lock, [this]() -> bool { return this->canvas_ready || !this->running; });
            lock.unlock();

            buffers.push_back(
                *this->canvas.canvas_commands->canvas_command_buffers[this->current_frame]);
            if (this->canvas.buffer_exist())
                buffers.push_back(*this->canvas.artcode_commands
                                       ->artcode_command_buffers[this->current_frame]);
        } else {
            record_imgui_command();
        }
        buffers.push_back(*this->commands.imgui_command_buffers[this->current_frame]);

        submit_buffers(buffers);
    }

    // stop the canvas worker thread
    {
        std::lock_guard<std::mutex> lock{this->canvas_mutex};
        this->running      = false;
        this->canvas_ready = false;
    }
    this->canvas_cv.notify_one();
    this->canvas_thread.join();
};

void Application::imgui_init() {
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForVulkan(this->window.app_window, true);

    // convert raii to c vulkan
    VkInstance       instance              = *this->ctx.instance;
    VkPhysicalDevice physical_device       = *this->ctx.physical_device;
    VkDevice         device                = *this->ctx.device;
    VkQueue          graphics_queue        = *this->ctx.graphics_queue;
    VkDescriptorPool imgui_descriptor_pool = *this->commands.imgui_descriptor_pool;

    ImGui_ImplVulkan_InitInfo init_info{};
    init_info.UseDynamicRendering = true;
    init_info.PipelineInfoMain.PipelineRenderingCreateInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    init_info.PipelineInfoMain.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
    init_info.PipelineInfoMain.PipelineRenderingCreateInfo.pColorAttachmentFormats =
        &this->format;
    init_info.Instance       = instance;
    init_info.PhysicalDevice = physical_device;
    init_info.Device         = device;
    init_info.Queue          = graphics_queue;
    init_info.DescriptorPool = imgui_descriptor_pool;
    init_info.MinImageCount  = 2;
    init_info.ImageCount     = this->ctx.config.image_count;

    ImGui_ImplVulkan_Init(&init_info);
};

void Application::reset_buffers() {
    auto fence_result = this->ctx.device.waitForFences(
        *this->commands.in_flight_fences[this->current_frame], vk::True, UINT64_MAX);

    if (fence_result != vk::Result::eSuccess)
        throw std::runtime_error("Failed to wait for fence!");

    auto [result, image_index] = this->swapchain.swapchain.acquireNextImage(
        UINT64_MAX, *this->commands.available_semaphores[this->current_frame], nullptr);

    if (result == vk::Result::eErrorOutOfDateKHR) {
        recreate_swapchain();
        return;
    } else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
        assert(result == vk::Result::eTimeout || result == vk::Result::eNotReady);
        throw std::runtime_error("Failed to acquire swapchain image!");
    }

    // pass image and result
    this->draw_result = result;
    this->image_index = image_index;

    this->ctx.device.resetFences(*this->commands.in_flight_fences[this->current_frame]);

    // resets all command buffers
    if (this->ui_manager.show_main_ui && this->canvas.canvas_commands) {
        this->canvas.canvas_commands->canvas_command_buffers[this->current_frame].reset();
        if (this->canvas.buffer_exist())
            this->canvas.artcode_commands->artcode_command_buffers[this->current_frame].reset();
    }
    this->commands.imgui_command_buffers[this->current_frame].reset();
};

void Application::submit_buffers(const std::vector<vk::CommandBuffer>& command_buffers) {
    vk::PipelineStageFlags destination_stage_mask(
        vk::PipelineStageFlagBits::eColorAttachmentOutput);

    // submit both command buffers
    vk::SubmitInfo submit_info{};
    submit_info.waitSemaphoreCount = 1,
    submit_info.pWaitSemaphores =
        &*this->commands.available_semaphores[this->current_frame];
    submit_info.pWaitDstStageMask    = &destination_stage_mask;
    submit_info.commandBufferCount   = static_cast<uint32_t>(command_buffers.size());
    submit_info.pCommandBuffers      = command_buffers.data();
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores =
        &*this->commands.finished_semaphores[this->current_frame];

    this->ctx.graphics_queue.submit(submit_info,
                                    *this->commands.in_flight_fences[this->current_frame]);

    vk::PresentInfoKHR present_info{};
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores =
        &*this->commands.finished_semaphores[this->current_frame];
    present_info.swapchainCount = 1;
    present_info.pSwapchains    = &*this->swapchain.swapchain;
    present_info.pImageIndices  = &image_index;

    this->draw_result = this->ctx.present_queue.presentKHR(present_info);

    if ((this->draw_result == vk::Result::eSuboptimalKHR) ||
        (this->draw_result == vk::Result::eErrorOutOfDateKHR) || this->frame_buffer_resize) {
        this->frame_buffer_resize = false;
        recreate_swapchain();
    } else {
        assert(this->draw_result == vk::Result::eSuccess);
    }

    this->current_frame = (this->current_frame + 1) % Application::MAX_FRAMES_IN_FLIGHT;
};

void Application::record_imgui_command() {
    auto& cmd = this->commands.imgui_command_buffers[this->current_frame];

    VkCommandBuffer cmd_buffer = *cmd;

    // render
    cmd.begin({});

    // render imgui ui components to swapchain
    transition_image_layout(this->swapchain.resources.images[this->image_index], cmd,
                            vk::ImageLayout::eUndefined,
                            vk::ImageLayout::eColorAttachmentOptimal, {},
                            vk::AccessFlagBits2::eColorAttachmentWrite,
                            vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                            vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                            vk::ImageAspectFlagBits::eColor);

    // prepare to render imgui
    vk::RenderingAttachmentInfo imgui_attachement_info{};
    imgui_attachement_info.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    imgui_attachement_info.loadOp      = vk::AttachmentLoadOp::eClear;
    imgui_attachement_info.storeOp     = vk::AttachmentStoreOp::eStore;
    imgui_attachement_info.clearValue  = this->clear_color;
    imgui_attachement_info.imageView =
        this->swapchain.resources.image_views[this->image_index];

    vk::RenderingInfo imgui_rendering_info{};
    imgui_rendering_info.renderArea.offset    = this->offset;
    imgui_rendering_info.renderArea.extent    = this->swapchain.resources.extent;
    imgui_rendering_info.layerCount           = 1;
    imgui_rendering_info.colorAttachmentCount = 1;
    imgui_rendering_info.pColorAttachments    = &imgui_attachement_info;

    // render imgui
    cmd.beginRendering(imgui_rendering_info);

    cmd.setViewport(
        0, vk::Viewport{
               0.0f, 0.0f, static_cast<float>(this->swapchain.resources.extent.width),
               static_cast<float>(this->swapchain.resources.extent.height), 0.0f, 1.0f});

    cmd.setScissor(0, vk::Rect2D{vk::Offset2D{0, 0}, this->swapchain.resources.extent});

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd_buffer, VK_NULL_HANDLE);

    cmd.endRendering();

    transition_image_layout(
        this->swapchain.resources.images[this->image_index], cmd,
        vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR,
        vk::AccessFlagBits2::eColorAttachmentWrite, {},
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::PipelineStageFlagBits2::eBottomOfPipe, vk::ImageAspectFlagBits::eColor);

    cmd.end();
};

void Application::recreate_swapchain() {
    this->ctx.device.waitIdle();

    // imgui ui
    clean_swapchain();

    this->ctx.create_extent();

    this->swapchain.create_swapchain(this->ctx.config.chosen_extent);

    this->swapchain.imgui_create_image_views();
};

void Application::clean_swapchain() {
    this->swapchain.resources.image_views.clear();
    this->swapchain.swapchain = nullptr;
};

void Application::cleanup() {
    this->ctx.device.waitIdle();

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    // cleanup shared memory from artcode vert and index
    Shared::Memory::cleanup();

    clean_swapchain();

    this->window.destroy_window();
};
