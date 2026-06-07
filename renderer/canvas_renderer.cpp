#include "canvas_renderer.hpp"
#include "imgui_impl_vulkan.h"
#include "imgui_internal.h"
#include "json.hpp"
#include "nav_items.hpp"
#include "vk_types.hpp"

#include <GLFW/glfw3.h>
#include <fstream>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

// definition
float     CanvasRenderer::zoom           = 1;
glm::vec2 CanvasRenderer::panning        = {0.0f, 0.0f};
glm::vec2 CanvasRenderer::mouse_last_pos = {0.0f, 0.0f};

CanvasRenderer::CanvasRenderer(const vk::raii::PhysicalDevice& physical_device,
                               const vk::raii::Device&         device,
                               const vk::raii::Queue&          graphics_queue,
                               const int&                      graphics_family,
                               const int& MAX_FRAMES_IN_FLIGHT, const uint32_t* app_width)
    : physical_device(physical_device),
      device(device),
      graphics_queue(graphics_queue),
      graphics_family(graphics_family),
      MAX_FRAMES_IN_FLIGHT(MAX_FRAMES_IN_FLIGHT),
      app_width(app_width),
      vk_buffers(physical_device, device) {
    // initialize shared memory for artcode
    Shared::Memory::load_shared_memory();
};

// TODO:improve this, result should go somewhere
// used for new created project
void CanvasRenderer::compile_shader() {
    nlohmann::json js;
    {
        const auto    shader_file = ProjectPath::get_solution_file();
        std::ifstream read(shader_file);
        js = nlohmann::json::parse(read);
    }

    std::string result;
    {
        const auto project_dir = ProjectPath::get_project_path();
        const auto shaders     = js["shaders"].get<std::vector<std::filesystem::path>>();
        for (const auto& shader : shaders) {
            const auto shader_dir = shader.parent_path();
            const auto shader_in  = shader_dir / shader.filename();
            const auto shader_out = shader_dir / (shader.filename().string() + ".spv");

            // cd to shader dir first
            std::string cmd = "cd " + project_dir.string() + " && ";
            // compile
            cmd += "glslangValidator -V ";
            cmd += shader_in.string() + " -o "; // shader in cmd
            cmd += shader_out;                  // shader out cmd
            cmd += " 2>&1";

            FILE* pipe = popen(cmd.c_str(), "r");
            if (!pipe) {
                result = "Failed to run the command. Error occured somewhere";
                return;
            }

            // temporary buffer to read chunks of result
            char buffer[128];

            // append buffer to result
            while (fgets(buffer, sizeof(buffer), pipe)) {
                result += buffer;
            }

            int exit_code = pclose(pipe);
            if (exit_code != 0) {
                result += "\nShader compilation failed: " + shader.filename().string();
            }
        }
    }
    std::cerr << result << std::endl;
};

void CanvasRenderer::set_canvas_pipeline() {
    // vulkan graphics pipeline for canvas
    this->graphics_pipeline =
        std::make_unique<VulkanGraphics>(this->device, this->vk_buffers.image_format);
    // artcode pipeline
    this->artcode_pipeline =
        std::make_unique<ArtcodeGraphics>(this->device, this->vk_buffers.image_format);
};

void CanvasRenderer::set_canvas_commands() {
    this->canvas_commands =
        std::make_unique<VulkanCanvas>(this->device, this->vk_buffers.canvas_uniform_buffer,
                                       this->graphics_pipeline->descriptor_set_layout,
                                       this->graphics_family, this->MAX_FRAMES_IN_FLIGHT);
    // artcode commands
    this->artcode_commands = std::make_unique<ArtcodeCommands>(
        this->device, this->vk_buffers.canvas_uniform_buffer,
        this->artcode_pipeline->artcode_set_layout, this->graphics_family,
        this->MAX_FRAMES_IN_FLIGHT);
    // vert and index buffer
    this->artcode_buffer = std::make_unique<ArtcodeBuffer>(
        this->physical_device, this->device, this->graphics_queue,
        this->artcode_commands->artcode_command_pool);
};

void CanvasRenderer::reload_pipeline() {
    // reset graphics_pipeline
    this->device.waitIdle();
    this->artcode_pipeline->pipeline.clear();

    // recreate graphics_pipeline
    this->artcode_pipeline->create_artcode_pipeline();
};

void CanvasRenderer::update_artcode_buffers() {
    const auto inst_size = Shared::Memory::get_intance_size();
    // wait gpu to finish using old buffers
    this->device.waitIdle();

    // clear the vectors for vertex, indices and its buffers
    this->artcode_buffer->int_vertex.clear();
    this->artcode_buffer->vertex_buffers.clear();
    this->artcode_buffer->int_index.clear();
    this->artcode_buffer->index_buffers.clear();
    // clear color
    this->push_constants.clear();

    for (size_t i = 0; i < inst_size; i++) {
        const auto& inst_vertex    = Shared::Memory::get_vertex(i);
        const auto& inst_indices   = Shared::Memory::get_index(i);
        const auto& inst_constants = Shared::Memory::get_constants(i);

        const std::vector<Vec2> vertex(inst_vertex.element,
                                       inst_vertex.element + inst_vertex.size);
        const std::vector<u32>  indices(inst_indices.element,
                                        inst_indices.element + inst_indices.size);

        this->push_constants.push_back(inst_constants);

        this->artcode_buffer->int_vertex.push_back(vertex);
        this->artcode_buffer->int_index.push_back(indices);
    }
    // clean/reset instance
    Shared::Memory::reset_instance();

    // create vertex buffer for each instance or shape
    this->artcode_buffer->create_vertex_buffer();
    this->artcode_buffer->create_index_buffer();
};

// this is used only for checking if both buffer exist to push the artcode command buffers in render loop
bool CanvasRenderer::buffer_exist() const {
    if (!this->artcode_buffer->vertex_buffers.empty() &&
        !this->artcode_buffer->index_buffers.empty())
        return true;

    return false;
};

void CanvasRenderer::workspace_events(GLFWwindow* app_window) {
    // set window user pointer at the beginning
    glfwSetWindowUserPointer(app_window, this);

    // calculate mouse movement
    glfwSetCursorPosCallback(
        app_window, [](GLFWwindow* window, double x_pos, double y_pos) -> void {
            auto canvas =
                reinterpret_cast<CanvasRenderer*>(glfwGetWindowUserPointer(window));

            if (!canvas->show_main_ui)
                return;

            auto dx = static_cast<float>(x_pos) - CanvasRenderer::mouse_last_pos.x;
            auto dy = static_cast<float>(y_pos) - CanvasRenderer::mouse_last_pos.y;

            // update last position
            CanvasRenderer::mouse_last_pos.x = static_cast<float>(x_pos);
            CanvasRenderer::mouse_last_pos.y = static_cast<float>(y_pos);

            // uses swapchain width (window width) for the save controls
            if (CanvasRenderer::mouse_last_pos.x < *canvas->app_width) {
                if (canvas->spacebar_pressed && canvas->left_click_pressed) {
                    CanvasRenderer::panning.x += dx * 1.0f;
                    CanvasRenderer::panning.y += -dy * 1.0f;

                    // add extra space in both ends of width and height
                    static constexpr float EXTRA_SPACE = 50.0f;
                    auto width  = static_cast<float>(canvas->vk_buffers.extent.width);
                    auto height = static_cast<float>(canvas->vk_buffers.extent.height);

                    CanvasRenderer::panning =
                        glm::clamp(CanvasRenderer::panning,
                                   glm::vec2(-width + EXTRA_SPACE, -height + EXTRA_SPACE),
                                   glm::vec2(width + EXTRA_SPACE, height + EXTRA_SPACE));
                }
            }
        });

    // detect if a key is pressed down or release
    glfwSetKeyCallback(
        app_window,
        [](GLFWwindow* window, int key, int scancode, int action, int mods) -> void {
            auto canvas =
                reinterpret_cast<CanvasRenderer*>(glfwGetWindowUserPointer(window));

            if (!canvas->show_main_ui)
                return;

            if (key == GLFW_KEY_LEFT_CONTROL) {
                if (action == GLFW_PRESS)
                    canvas->ctrl_pressed = true;
                if (action == GLFW_RELEASE)
                    canvas->ctrl_pressed = false;
            } else if (key == GLFW_KEY_SPACE) {
                if (action == GLFW_PRESS)
                    canvas->spacebar_pressed = true;
                if (action == GLFW_RELEASE)
                    canvas->spacebar_pressed = false;
            }

            // for text editor
            if (canvas->ctrl_pressed) {
                if (key == GLFW_KEY_S) {
                    if (action == GLFW_PRESS) {
                        TextEditorUtils::file_save = true;
                    }
                }
            }
        });

    // scroll
    glfwSetScrollCallback(app_window, [](GLFWwindow* window, double x, double y) -> void {
        auto canvas = reinterpret_cast<CanvasRenderer*>(glfwGetWindowUserPointer(window));

        if (!canvas->show_main_ui)
            return;

        if (canvas->ctrl_pressed) {
            CanvasRenderer::zoom += y * 0.10;
            CanvasRenderer::zoom  = glm::clamp(CanvasRenderer::zoom, 0.1f, 10.0f);
        }
    });

    // panning
    glfwSetMouseButtonCallback(
        app_window, [](GLFWwindow* window, int button, int action, int mods) -> void {
            auto canvas =
                reinterpret_cast<CanvasRenderer*>(glfwGetWindowUserPointer(window));

            if (!canvas->show_main_ui)
                return;

            if (button == GLFW_MOUSE_BUTTON_LEFT) {
                if (action == GLFW_PRESS)
                    canvas->left_click_pressed = true;
                if (action == GLFW_RELEASE)
                    canvas->left_click_pressed = false;
            }
        });
};

void CanvasRenderer::canvas_setup(const glm::vec3& artboard_size, bool show_main_ui) {
    this->show_main_ui = show_main_ui;

    // identity matrix
    glm::mat4 view = glm::mat4(1.0f);
    {
        // get canvas center
        const float center_x = this->vk_buffers.extent.width / 2.0f;
        const float center_y = this->vk_buffers.extent.height / 2.0f;

        // translate to center
        view = glm::translate(view, glm::vec3(center_x, center_y, 0.0f));

        // scale to center
        view =
            glm::scale(view, glm::vec3(CanvasRenderer::zoom, CanvasRenderer::zoom, 1.0f));

        // tanslate back
        view = glm::translate(view, glm::vec3(-center_x, -center_y, 0.0f));

        // translate to the panning position
        view = glm::translate(
            view, glm::vec3(CanvasRenderer::panning.x, CanvasRenderer::panning.y, 0.0f));
    }

    const auto width  = artboard_size.x;
    const auto height = artboard_size.y;

    ArtboardBuffer a_ubo{
        .proj = glm::ortho(0.0f, (float)this->vk_buffers.extent.width,
                           (float)this->vk_buffers.extent.height, 0.0f, -1.0f, 1.0f),
        .view = view,
        .model =
            glm::translate(glm::mat4(1.0f),
                           glm::vec3((this->vk_buffers.extent.width - width) / 2,
                                     (this->vk_buffers.extent.height - height) / 2, 0.0f)),
        .reso     = {width, height},
        .viewport = {this->vk_buffers.extent.width, this->vk_buffers.extent.height}};

    memcpy(this->vk_buffers.canvas_uniform_buffer_mapped, &a_ubo, sizeof(a_ubo));
};

void CanvasRenderer::record_canvas_command(const uint32_t& current_frame) {
    auto& cmd = this->canvas_commands->canvas_command_buffers[current_frame];

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

    cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, this->graphics_pipeline->pipeline);

    cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                           this->graphics_pipeline->layout, 0,
                           *this->canvas_commands->canvas_descriptor_set[0], nullptr);

    cmd.setViewport(
        0, vk::Viewport{0.0f, 0.0f, static_cast<float>(this->vk_buffers.extent.width),
                        static_cast<float>(this->vk_buffers.extent.height), 0.0f, 1.0f});

    cmd.setScissor(
        0, vk::Rect2D{vk::Offset2D{0, 0}, vk::Extent2D{this->vk_buffers.extent.width,
                                                       this->vk_buffers.extent.height}});

    cmd.draw(4, 1, 0, 0);

    cmd.endRendering();

    // has its own transition if artcode command function is not running
    if (!buffer_exist())
        transition_image(
            this->vk_buffers.images, cmd, vk::ImageLayout::eColorAttachmentOptimal,
            vk::ImageLayout::eShaderReadOnlyOptimal,
            vk::AccessFlagBits2::eColorAttachmentWrite, {},
            vk::PipelineStageFlagBits2::eColorAttachmentOutput,
            vk::PipelineStageFlagBits2::eBottomOfPipe, vk::ImageAspectFlagBits::eColor);

    cmd.end();
};

void CanvasRenderer::record_artcode_command(const uint32_t& current_frame) {
    auto& cmd = this->artcode_commands->artcode_command_buffers[current_frame];
    // render
    cmd.begin({});

    // prepare to render artcode
    vk::RenderingAttachmentInfo artcode_attachement_info{};
    artcode_attachement_info.imageView   = this->vk_buffers.image_views;
    artcode_attachement_info.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    artcode_attachement_info.loadOp      = vk::AttachmentLoadOp::eLoad;
    artcode_attachement_info.storeOp     = vk::AttachmentStoreOp::eStore;
    artcode_attachement_info.clearValue  = this->clear_color;

    vk::RenderingInfo artcode_rendering_info{};
    artcode_rendering_info.renderArea.offset = this->offset;
    artcode_rendering_info.renderArea.extent =
        vk::Extent2D{this->vk_buffers.extent.width, this->vk_buffers.extent.height};
    artcode_rendering_info.layerCount           = 1;
    artcode_rendering_info.colorAttachmentCount = 1;
    artcode_rendering_info.pColorAttachments    = &artcode_attachement_info;

    // render canvas
    cmd.beginRendering(artcode_rendering_info);

    cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, this->artcode_pipeline->pipeline);

    cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, this->artcode_pipeline->layout,
                           0, *this->artcode_commands->artcode_descriptor_set[0], nullptr);

    cmd.setViewport(
        0, vk::Viewport{0.0f, 0.0f, static_cast<float>(this->vk_buffers.extent.width),
                        static_cast<float>(this->vk_buffers.extent.height), 0.0f, 1.0f});

    cmd.setScissor(
        0, vk::Rect2D{vk::Offset2D{0, 0}, vk::Extent2D{this->vk_buffers.extent.width,
                                                       this->vk_buffers.extent.height}});

    const auto& inst_index = this->artcode_buffer->int_index;
    // loop in reverse, this makes the firnst instance declared to occupy the first layer
    for (size_t i = inst_index.size(); i > 0; i--) {
        // include 0, still uses size_t, maybe int is better
        auto idx = i - 1;
        {
            PushConstants cons = this->push_constants[idx];

            cmd.pushConstants<PushConstants>(*this->artcode_pipeline->layout,
                                             vk::ShaderStageFlagBits::eFragment, 0, cons);
        }
        cmd.bindVertexBuffers(0, *this->artcode_buffer->vertex_buffers[idx], {0});
        cmd.bindIndexBuffer(*this->artcode_buffer->index_buffers[idx], 0,
                            vk::IndexType::eUint32);

        cmd.drawIndexed(inst_index[idx].size(), 1, 0, 0, 0);
    }

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

void CanvasRenderer::update_canvas(const vk::raii::Device& device) {
    const auto canvas = ImGui::FindWindowByName("##canvas-begin");

    if (canvas) {
        const auto& width  = static_cast<uint32_t>(canvas->Size.x);
        const auto& height = static_cast<uint32_t>(canvas->Size.y);

        if (width != this->vk_buffers.extent.width ||
            height != this->vk_buffers.extent.height) {
            device.waitIdle();

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
