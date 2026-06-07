#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan_raii.hpp>

class ArtcodeGraphics {
  public:
    ArtcodeGraphics(const vk::raii::Device& device, vk::Format& image_format);

    vk::raii::DescriptorSetLayout artcode_set_layout = nullptr;
    // shader module
    vk::raii::ShaderModule   vert_shader_module = nullptr;
    vk::raii::ShaderModule   frag_shader_module = nullptr;
    vk::raii::ShaderModule   geom_shader_module = nullptr;
    vk::raii::Pipeline       pipeline           = nullptr;
    vk::raii::PipelineLayout layout             = nullptr;

    void create_artcode_pipeline();

  private:
    const vk::raii::Device& device;

    const vk::Format& image_format;

    std::vector<char> read_file(const std::string& file_name) const;

    [[nodiscard]]
    vk::raii::ShaderModule create_shader_module(const std::vector<char>& code) const;

    void create_descriptor_set_layout();
};
