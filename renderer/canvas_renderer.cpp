#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "artcode_instance.hpp"
#include "vk_types.hpp"

#include "canvas_renderer.hpp"
#include "imgui_impl_vulkan.h"
#include "nav_items.hpp"
#include "transition_image.hpp"
#include <GLFW/glfw3.h>
#include <glm/ext/matrix_clip_space.hpp>

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

void CanvasRenderer::set_canvas_pipeline() {
    // vulkan graphics pipeline for canvas
    this->graphics_pipeline =
        std::make_unique<VulkanGraphics>(this->device, this->vk_buffers.image_format);
    // artcode pipeline
    this->artcode_pipeline =
        std::make_unique<ArtcodeGraphics>(this->device, this->vk_buffers.image_format);
};

void CanvasRenderer::set_canvas_commands() {
    this->artboard_commands = std::make_unique<VulkanArtboard>(
        this->device, this->vk_buffers.artboard_uniform_buffer,
        this->graphics_pipeline->descriptor_set_layout, this->graphics_family,
        this->MAX_FRAMES_IN_FLIGHT);
    // artcode commands
    this->artcode_commands = std::make_unique<ArtcodeCommands>(
        this->device, this->vk_buffers.artboard_uniform_buffer,
        this->artcode_pipeline->artcode_set_layout, this->graphics_family,
        this->MAX_FRAMES_IN_FLIGHT);
    // vert, index buffer, ssbo and export image
    this->artcode_buffer = std::make_unique<ArtcodeBuffer>(
        this->physical_device, this->device, this->graphics_queue,
        this->artcode_commands->artcode_command_pool, this->vk_buffers.images,
        this->artcode_commands->artcode_descriptor_sets);
};

void CanvasRenderer::reload_pipeline() {
    this->device.waitIdle();

    // recompile shaders
    this->artcode_pipeline->shader_stages.clear();
    this->artcode_pipeline->create_shaders();

    // reload pipeline
    this->artcode_pipeline->pipeline_triangle.clear();
    this->artcode_pipeline->create_pipeline();
};

void CanvasRenderer::update_artcode_buffers() {
    // wait gpu to finish using old buffers
    this->device.waitIdle();

    // clear the arrays for multiple buffers
    /*this->artcode_buffer->inst_vertex.clear();
    this->artcode_buffer->vertex_buffers.clear();
    this->artcode_buffer->vertex_memories.clear();
    this->artcode_buffer->inst_index.clear();
    this->artcode_buffer->index_buffers.clear();
    this->artcode_buffer->index_memories.clear();*/
    this->artcode_buffer->ssbo_buffers.clear();
    this->artcode_buffer->ssbo_memories.clear();
    this->artcode_buffer->skew_data.clear();
    // clear push constants
    this->push_constants.clear();

    // set the buffers resources from shared memory
    {
        const auto inst_size = Shared::Memory::get_intance_size();
        this->inst_size      = inst_size;
    }

    for (size_t i = 0; i < this->inst_size; i++) {
        const auto& instance = Shared::Memory::get_instance(i);

        /*std::vector<Vec4> vertex(instance.vertex.element.begin(),
                                 instance.vertex.element.begin() + instance.vertex.size);
        std::vector<u32>  indices(instance.index.element.begin(),
                                  instance.index.element.begin() + instance.index.size);

        this->artcode_buffer->inst_vertex.push_back(vertex);
        this->artcode_buffer->inst_index.push_back(indices);*/
        this->artcode_buffer->skew_data.push_back(instance.skew_data);

        this->push_constants.push_back(instance.constants);
    }
    // reset all instances
    Shared::Memory::reset_instance();

    // create buffers for each instance or shape
    /*this->artcode_buffer->create_vertex_buffer();
    this->artcode_buffer->create_index_buffer();*/
    this->artcode_buffer->create_ssbo_buffer();
};

// NOTE: this is used only for checking if buffer data exist to
// push the artcode command buffers in render loop
bool CanvasRenderer::buffer_exist() const {
    if (!this->artcode_buffer->skew_data.empty())
        return true;

    return false;
};

void CanvasRenderer::canvas_events(GLFWwindow* app_window) {
    // set window user pointer at the beginning
    glfwSetWindowUserPointer(app_window, this);

    // calculate mouse movement
    glfwSetCursorPosCallback(
        app_window, [](GLFWwindow* window, double x_pos, double y_pos) -> void {
            auto canvas =
                reinterpret_cast<CanvasRenderer*>(glfwGetWindowUserPointer(window));

            if (!canvas->show_main_ui)
                return;

            auto dx = static_cast<float>(x_pos) - CanvasControls::mouse_last_pos.x;
            auto dy = static_cast<float>(y_pos) - CanvasControls::mouse_last_pos.y;

            // update last position
            CanvasControls::mouse_last_pos.x = static_cast<float>(x_pos);
            CanvasControls::mouse_last_pos.y = static_cast<float>(y_pos);

            // uses swapchain width (window width) for the save controls
            if (CanvasControls::mouse_last_pos.x < *canvas->app_width) {
                if (canvas->spacebar_pressed && canvas->left_click_pressed) {
                    CanvasControls::panning.x += dx * 1.0f;
                    CanvasControls::panning.y += -dy * 1.0f;

                    // add extra space in both ends of width and height
                    static constexpr float EXTRA_SPACE = 50.0f;
                    auto width  = static_cast<float>(canvas->vk_buffers.extent.width);
                    auto height = static_cast<float>(canvas->vk_buffers.extent.height);

                    CanvasControls::panning =
                        glm::clamp(CanvasControls::panning,
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
                    if (action == GLFW_PRESS)
                        TextEditorUtils::is_file_save = true;
                }
            }
        });

    // scroll
    glfwSetScrollCallback(app_window, [](GLFWwindow* window, double x, double y) -> void {
        auto canvas = reinterpret_cast<CanvasRenderer*>(glfwGetWindowUserPointer(window));

        if (!canvas->show_main_ui)
            return;

        if (canvas->ctrl_pressed) {
            CanvasControls::zoom += y * 0.10;
            CanvasControls::zoom  = glm::clamp(CanvasControls::zoom, 0.1f, 10.0f);
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

void CanvasRenderer::artboard_setup(const glm::vec3& artboard_size, bool show_main_ui) {
    // transfer data
    this->show_main_ui = show_main_ui;

    // set ubo
    ArtboardBuffer ab_ubo{
        .proj  = glm::ortho(0.0f, artboard_size.x, artboard_size.y, 0.0f, -1.0f, 1.0f),
        .view  = glm::mat4(1.0f),
        .model = glm::mat4(1.0f),
        .reso  = glm::vec2{artboard_size.x, -artboard_size.y},
        // third value in artboard is ppi
        .ppi = artboard_size.z};

    // y-flip
    ab_ubo.proj[1][1] *= -1;

    memcpy(this->vk_buffers.artboard_uniform_buffer_mapped, &ab_ubo, sizeof(ab_ubo));
};

void CanvasRenderer::save_art() {
    if (SaveFile::has_path) {
        const auto width      = static_cast<int>(vk_buffers.extent.width);
        const auto height     = static_cast<int>(vk_buffers.extent.height);
        const int  image_size = width * height * 4;

        const auto& staging_memory = this->artcode_buffer->create_export_image_buffer(
            this->vk_buffers.extent, image_size);

        void* data = staging_memory.mapMemory(0, image_size);

        auto save_path = SaveFile::get_save_path();
        // avoids duplicate extension i.e image.png.png, also adds if extension is
        // missing, also sets to .png if passed extension is wrong
        save_path.replace_extension(".png");

        stbi_write_png(save_path.c_str(), width, height, 4, data, width * 4);

        // unmap after creating
        staging_memory.unmapMemory();

        // return to orig state
        SaveFile::has_path = false;
    }
};

void CanvasRenderer::record_artboard_command(const uint32_t current_frame) {
    auto& cmd = this->artboard_commands->artboard_command_buffers[current_frame];

    // render
    cmd.begin({});

    transition_image_layout(this->vk_buffers.images, cmd, vk::ImageLayout::eUndefined,
                            vk::ImageLayout::eColorAttachmentOptimal, {},
                            vk::AccessFlagBits2::eColorAttachmentWrite,
                            vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                            vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                            vk::ImageAspectFlagBits::eColor);

    // prepare to render canvas
    vk::RenderingAttachmentInfo artboard_attachment_info{};
    artboard_attachment_info.imageView   = this->vk_buffers.msaa_image_view;
    artboard_attachment_info.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    artboard_attachment_info.loadOp      = vk::AttachmentLoadOp::eClear;
    artboard_attachment_info.storeOp     = vk::AttachmentStoreOp::eDontCare;
    artboard_attachment_info.clearValue  = this->clear_color;

    // resolve
    artboard_attachment_info.resolveMode      = vk::ResolveModeFlagBits::eAverage;
    artboard_attachment_info.resolveImageView = this->vk_buffers.image_views;
    artboard_attachment_info.resolveImageLayout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::RenderingInfo artboard_rendering_info{};
    artboard_rendering_info.renderArea.offset = this->offset;
    artboard_rendering_info.renderArea.extent =
        vk::Extent2D{this->vk_buffers.extent.width, this->vk_buffers.extent.height};
    artboard_rendering_info.layerCount           = 1;
    artboard_rendering_info.colorAttachmentCount = 1;
    artboard_rendering_info.pColorAttachments    = &artboard_attachment_info;

    // render canvas
    cmd.beginRendering(artboard_rendering_info);

    cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, this->graphics_pipeline->pipeline);

    cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                           this->graphics_pipeline->layout, 0,
                           *this->artboard_commands->artboard_descriptor_set[0], nullptr);

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
        transition_image_layout(
            this->vk_buffers.images, cmd, vk::ImageLayout::eColorAttachmentOptimal,
            vk::ImageLayout::eShaderReadOnlyOptimal,
            vk::AccessFlagBits2::eColorAttachmentWrite, {},
            vk::PipelineStageFlagBits2::eColorAttachmentOutput,
            vk::PipelineStageFlagBits2::eBottomOfPipe, vk::ImageAspectFlagBits::eColor);

    cmd.end();
};

// NOTE:need to update the pipeline and rendering
void CanvasRenderer::record_artcode_command(const uint32_t current_frame) {
    auto& cmd = this->artcode_commands->artcode_command_buffers[current_frame];
    // render
    cmd.begin({});

    // use MSAA
    vk::RenderingAttachmentInfo artcode_attachement_info{};
    artcode_attachement_info.imageView   = this->vk_buffers.msaa_image_view;
    artcode_attachement_info.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    artcode_attachement_info.loadOp      = vk::AttachmentLoadOp::eClear;
    artcode_attachement_info.storeOp     = vk::AttachmentStoreOp::eDontCare;
    artcode_attachement_info.clearValue  = this->clear_color;

    // resolve with image view
    artcode_attachement_info.resolveMode      = vk::ResolveModeFlagBits::eAverage;
    artcode_attachement_info.resolveImageView = this->vk_buffers.image_views;
    artcode_attachement_info.resolveImageLayout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::RenderingInfo artcode_rendering_info{};
    artcode_rendering_info.renderArea.offset = this->offset;
    artcode_rendering_info.renderArea.extent =
        vk::Extent2D{this->vk_buffers.extent.width, this->vk_buffers.extent.height};
    artcode_rendering_info.layerCount           = 1;
    artcode_rendering_info.colorAttachmentCount = 1;
    artcode_rendering_info.pColorAttachments    = &artcode_attachement_info;

    // render canvas
    cmd.beginRendering(artcode_rendering_info);

    // set viewport and scissors
    cmd.setViewport(
        0, vk::Viewport{0.0f, 0.0f, static_cast<float>(this->vk_buffers.extent.width),
                        static_cast<float>(this->vk_buffers.extent.height), 0.0f, 1.0f});

    cmd.setScissor(
        0, vk::Rect2D{vk::Offset2D{0, 0}, vk::Extent2D{this->vk_buffers.extent.width,
                                                       this->vk_buffers.extent.height}});

    // const auto& inst_index = this->artcode_buffer->inst_index;

    //  draw in reverse order for shape instances
    //  this makes the first declared shape always be the front shape in artboard
    for (size_t i = this->inst_size; i > 0; i--) {
        const auto idx = i - 1;

        cmd.bindPipeline(vk::PipelineBindPoint::eGraphics,
                         this->artcode_pipeline->pipeline_triangle);

        cmd.bindDescriptorSets(
            vk::PipelineBindPoint::eGraphics, this->artcode_pipeline->layout, 0,
            *this->artcode_commands->artcode_descriptor_sets[idx], nullptr);

        cmd.pushConstants<PushConstants>(*this->artcode_pipeline->layout,
                                         vk::ShaderStageFlagBits::eVertex |
                                             vk::ShaderStageFlagBits::eFragment,
                                         0, this->push_constants[idx]);

        cmd.draw(6, 1, 0, 0);
    }

    cmd.endRendering();

    transition_image_layout(
        this->vk_buffers.images, cmd, vk::ImageLayout::eColorAttachmentOptimal,
        vk::ImageLayout::eShaderReadOnlyOptimal, vk::AccessFlagBits2::eColorAttachmentWrite,
        {}, vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::PipelineStageFlagBits2::eBottomOfPipe, vk::ImageAspectFlagBits::eColor);

    cmd.end();
};

void CanvasRenderer::update_artboard() {
    this->device.waitIdle();

    // NOTE:this updates the images from vk buffers to match the artboard dimensions
    // also updates the texture for canvas to render the artboard
    const auto& artboard = Artboard::get_artboard_size();
    const auto  width    = static_cast<uint32_t>(artboard.x);
    const auto  height   = static_cast<uint32_t>(artboard.y);

    if (this->vk_buffers.extent.width != width && this->vk_buffers.extent.height != height) {
        // create image views and msaa image view
        this->vk_buffers.artboard_create_image(width, height);
        this->vk_buffers.artboard_create_image_views();
        this->vk_buffers.artboard_create_msaa();
    }
    // remove old texture
    ImGui_ImplVulkan_RemoveTexture(ArtboardUtils::artboard_texture);

    // run again after texture removal
    ArtboardUtils::artboard_texture = ImGui_ImplVulkan_AddTexture(
        *this->vk_buffers.artboard_sampler, *this->vk_buffers.image_views,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
};
