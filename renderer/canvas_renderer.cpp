#include "vk_types.hpp"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "canvas_renderer.hpp"
#include "imgui_impl_vulkan.h"
#include "imgui_internal.h"
#include "json.hpp"
#include "nav_items.hpp"
#include "transition_image.hpp"

#include <GLFW/glfw3.h>
#include <fstream>
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

// TODO:improve this, result should go somewhere
// used for new created project
void CanvasRenderer::compile_shader() {
    nlohmann::json js;
    {
        const auto&   shader_file = ProjectPath::get_solution_file();
        std::ifstream read(shader_file);
        js = nlohmann::json::parse(read);
    }

    std::string result;
    {
        const auto& project_dir = ProjectPath::get_project_path();
        const auto  shaders     = js["shaders"].get<std::vector<std::filesystem::path>>();
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
    this->artboard_commands = std::make_unique<VulkanArtboard>(
        this->device, this->vk_buffers.artboard_uniform_buffer,
        this->graphics_pipeline->descriptor_set_layout, this->graphics_family,
        this->MAX_FRAMES_IN_FLIGHT);
    // artcode commands
    this->artcode_commands = std::make_unique<ArtcodeCommands>(
        this->device, this->vk_buffers.artboard_uniform_buffer,
        this->artcode_pipeline->artcode_set_layout, this->graphics_family,
        this->MAX_FRAMES_IN_FLIGHT);
    // vert and index buffer
    this->artcode_buffer = std::make_unique<ArtcodeBuffer>(
        this->physical_device, this->device, this->graphics_queue,
        this->artcode_commands->artcode_command_pool, this->vk_buffers.images,
        this->artcode_commands->artcode_descriptor_sets);
};

void CanvasRenderer::reload_pipeline() {
    // reset graphics_pipeline
    this->device.waitIdle();

    bool need_trilist = false, need_linelist = false;

    const auto inst_size = Shared::Memory::get_intance_size();
    // recreate graphics_pipeline
    for (size_t i = 0; i < inst_size; i++) {
        const auto& cons = Shared::Memory::get_constants(i);
        if (!cons.fill) {
            need_linelist = true;
        } else {
            need_trilist = true;
        }

        // early exit if both true
        if (need_trilist && need_linelist)
            break;
    }

    // create shaders
    this->artcode_pipeline->shader_stages.clear();
    this->artcode_pipeline->create_shaders();
    if (need_linelist) {
        this->artcode_pipeline->pipeline_linelist.clear();
        this->artcode_pipeline->create_pipeline(Topology::LineList);
    }

    if (need_trilist) {
        this->artcode_pipeline->pipeline_trianglelist.clear();
        this->artcode_pipeline->create_pipeline(Topology::TriangleList);
    }
};

void CanvasRenderer::update_artcode_buffers() {
    // wait gpu to finish using old buffers
    this->device.waitIdle();

    // clear the arrays for multiple buffers
    this->artcode_buffer->inst_vertex.clear();
    this->artcode_buffer->vertex_buffers.clear();
    this->artcode_buffer->vertex_memories.clear();
    this->artcode_buffer->inst_index.clear();
    this->artcode_buffer->index_buffers.clear();
    this->artcode_buffer->index_memories.clear();
    this->artcode_buffer->ssbo_buffers.clear();
    this->artcode_buffer->ssbo_memories.clear();
    this->artcode_buffer->skew_data.clear();
    // clear push constants
    this->push_constants.clear();

    const auto inst_size = Shared::Memory::get_intance_size();
    for (size_t i = 0; i < inst_size; i++) {
        const auto& instance = Shared::Memory::get_instance(i);

        std::vector<Vec2> vertex(instance.vertex.element.begin(),
                                 instance.vertex.element.begin() + instance.vertex.size);
        std::vector<u32>  indices(instance.index.element.begin(),
                                  instance.index.element.begin() + instance.index.size);

        this->artcode_buffer->inst_vertex.push_back(vertex);
        this->artcode_buffer->inst_index.push_back(indices);
        this->artcode_buffer->skew_data.push_back(instance.skew_data);

        this->push_constants.push_back(instance.constants);
    }

    // create buffers for each instance or shape
    this->artcode_buffer->create_vertex_buffer();
    this->artcode_buffer->create_index_buffer();
    this->artcode_buffer->create_ssbo_buffer();
};

// this is used only for checking if both buffer exist to push the artcode command buffers in render loop
bool CanvasRenderer::buffer_exist() const {
    if (!this->artcode_buffer->vertex_buffers.empty() &&
        !this->artcode_buffer->index_buffers.empty())
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
        .proj     = glm::ortho(0.0f, artboard_size.x, artboard_size.y, 0.0f, -1.0f, 1.0f),
        .view     = glm::mat4(1.0f),
        .model    = glm::mat4(1.0f),
        .reso     = artboard_size,
        .viewport = artboard_size};

    memcpy(this->vk_buffers.artboard_uniform_buffer_mapped, &ab_ubo, sizeof(ab_ubo));
};

// FIXME:the artboard size in application doesnt match to what the image looks, when
// inspecting the image it matches the artboard dimensions but the look doesnt
void CanvasRenderer::save_art() {
    // TODO:not sure if the if statement should be inside this funciton or in the
    // place where the function is called
    if (SaveFile::has_path) {
        const glm::vec2 artboard = {vk_buffers.extent.width, vk_buffers.extent.height};

        const auto width      = static_cast<int>(artboard.x);
        const auto height     = static_cast<int>(artboard.y);
        const auto image_size = width * height * 4;

        // NOTE:might need to have a separate frame for the artboard itself
        //  create staging memory and its buffers
        const auto& staging_memory = this->artcode_buffer->create_export_image_buffer(
            this->vk_buffers.extent, artboard, image_size);

        void* data = staging_memory.mapMemory(0, image_size);

        const auto& save_path = SaveFile::get_save_path();
        const auto& file_name = save_path / "image.png";

        stbi_write_png(file_name.c_str(), width, height, 4, data, width * 4);

        // unmap after saving
        staging_memory.unmapMemory();

        // return has path to orig state
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

void CanvasRenderer::record_artcode_command(const uint32_t current_frame) {
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

    const auto& inst_index = this->artcode_buffer->inst_index;
    // render canvas
    cmd.beginRendering(artcode_rendering_info);

    // set viewport and scissors
    cmd.setViewport(
        0, vk::Viewport{0.0f, 0.0f, static_cast<float>(this->vk_buffers.extent.width),
                        static_cast<float>(this->vk_buffers.extent.height), 0.0f, 1.0f});

    cmd.setScissor(
        0, vk::Rect2D{vk::Offset2D{0, 0}, vk::Extent2D{this->vk_buffers.extent.width,
                                                       this->vk_buffers.extent.height}});

    // draw in reverse order for shape instances
    // this makes the first shape instance declared will always be the most front shape in artboard
    for (size_t i = inst_index.size(); i > 0; i--) {
        const auto idx = i - 1;

        const auto& cons = this->push_constants[idx];
        if (cons.fill) {
            cmd.bindPipeline(vk::PipelineBindPoint::eGraphics,
                             this->artcode_pipeline->pipeline_trianglelist);
        } else {
            cmd.bindPipeline(vk::PipelineBindPoint::eGraphics,
                             this->artcode_pipeline->pipeline_linelist);
        }

        cmd.bindDescriptorSets(
            vk::PipelineBindPoint::eGraphics, this->artcode_pipeline->layout, 0,
            *this->artcode_commands->artcode_descriptor_sets[idx], nullptr);

        cmd.pushConstants<PushConstants>(*this->artcode_pipeline->layout,
                                         vk::ShaderStageFlagBits::eVertex |
                                             vk::ShaderStageFlagBits::eFragment,
                                         0, cons);

        cmd.setLineWidth(cons.stroke);

        cmd.bindVertexBuffers(0, *this->artcode_buffer->vertex_buffers[idx], {0});

        cmd.bindIndexBuffer(*this->artcode_buffer->index_buffers[idx], 0,
                            vk::IndexType::eUint32);

        cmd.drawIndexed(inst_index[idx].size(), 1, 0, 0, 0);
    }

    cmd.endRendering();

    transition_image_layout(
        this->vk_buffers.images, cmd, vk::ImageLayout::eColorAttachmentOptimal,
        vk::ImageLayout::eShaderReadOnlyOptimal, vk::AccessFlagBits2::eColorAttachmentWrite,
        {}, vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::PipelineStageFlagBits2::eBottomOfPipe, vk::ImageAspectFlagBits::eColor);

    cmd.end();
};

void CanvasRenderer::update_arboard() {
    const auto& canvas = ImGui::FindWindowByName("##canvas-begin");

    // NOTE:this updates the images from vk buffers to match the artboard dimensions
    // also updates the texture for canvas to render the artboard
    if (canvas) {
        const auto& width  = static_cast<uint32_t>(Artboard::get_artboard_size().x);
        const auto& height = static_cast<uint32_t>(Artboard::get_artboard_size().y);

        this->device.waitIdle();

        this->vk_buffers.artboard_create_image(width, height);
        this->vk_buffers.artboard_create_image_views();

        // remove the old texture at canvas resize
        ImGui_ImplVulkan_RemoveTexture(ArtboardUtils::artboard_texture);

        // run again after texture removal
        ArtboardUtils::artboard_texture = ImGui_ImplVulkan_AddTexture(
            *this->vk_buffers.artboard_sampler, *this->vk_buffers.image_views,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
};
