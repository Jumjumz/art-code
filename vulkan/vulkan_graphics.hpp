#ifndef VULKAN_GRAPHICS_HPP
#define VULKAN_GRAPHICS_HPP

#pragma once

#include <vulkan/vulkan_raii.hpp>

class VulkanGraphics {
  public:
    VulkanGraphics(const vk::raii::PhysicalDevice &physical_device,
                   const vk::raii::Device &device,
                   const vk::Format &image_format, const int &graphics_family);

    // shader module
    vk::raii::ShaderModule vert_shader_module = nullptr;
    vk::raii::ShaderModule frag_shader_module = nullptr;

    // pipeline
    vk::raii::Pipeline graphics_pipeline = nullptr;
    vk::raii::PipelineLayout layout = nullptr;

    vk::raii::DescriptorSetLayout descriptor_set_layout = nullptr;

    // command pool
    vk::raii::CommandPool command_pool = nullptr;

  private:
    const vk::raii::PhysicalDevice &physical_device;

    const vk::raii::Device &device;

    const vk::Format &image_format;

    const int &graphics_family;

    void create_descriptor_set_layout();

    void create_graphics_pipeline();

    void create_command_pool();

    static std::vector<char> read_file(const std::string &file_name);

    [[nodiscard]]
    vk::raii::ShaderModule
    create_shader_module(const std::vector<char> &code) const;
};

#endif // !VULKAN_GRAPHICS_HPP
