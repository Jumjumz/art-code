#pragma once

#include <vulkan/vulkan_raii.hpp>

class VulkanGraphics {
  public:
    VulkanGraphics(const vk::raii::Device& device, const vk::Format& image_format);

    // shader module
    vk::raii::ShaderModule vert_shader_module = nullptr;
    vk::raii::ShaderModule frag_shader_module = nullptr;

    // pipeline
    vk::raii::Pipeline       graphics_pipeline = nullptr;
    vk::raii::PipelineLayout layout            = nullptr;

    vk::raii::DescriptorSetLayout descriptor_set_layout = nullptr;

    void create_graphics_pipeline();

  private:
    const vk::raii::Device& device;

    const vk::Format& image_format;

    std::vector<char> read_file(const std::string& file_name);

    [[nodiscard]]
    vk::raii::ShaderModule create_shader_module(const std::vector<char>& code) const;

    void create_descriptor_set_layout();
};
