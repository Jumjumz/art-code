#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan_raii.hpp>

enum class Topology { TriangleList, LineList };

class ArtcodeGraphics {
  public:
    ArtcodeGraphics(const vk::raii::Device& device, vk::Format& image_format);

    vk::raii::DescriptorSetLayout artcode_set_layout    = nullptr;
    vk::raii::Pipeline            pipeline_trianglelist = nullptr;
    vk::raii::Pipeline            pipeline_linelist     = nullptr;
    vk::raii::PipelineLayout      layout                = nullptr;

    std::vector<vk::PipelineShaderStageCreateInfo> shader_stages;

    void create_shaders(Topology topology);
    void create_pipeline(Topology topology);

  private:
    const vk::raii::Device& device;

    const vk::Format& image_format;

    // shader module
    vk::raii::ShaderModule vert_shader_module = nullptr;
    vk::raii::ShaderModule frag_shader_module = nullptr;

    [[nodiscard]]
    vk::raii::ShaderModule create_shader_module(const std::vector<char>& code) const;

    std::vector<char> read_file(const std::string& file_name) const;

    void create_descriptor_set_layout();
};
