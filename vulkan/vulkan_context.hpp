#ifndef VULKAN_CONTEXT_HPP
#define VULKAN_CONTEXT_HPP
#define GLFW_INCLUDE_VULKAN

#pragma once

#include <GLFW/glfw3.h>
#include <vulkan/vulkan_raii.hpp>

class VulkanContext {
  public:
    VulkanContext(GLFWwindow *window);
    // core
    vk::raii::Context context;
    vk::raii::Instance instance = nullptr;
    vk::raii::SurfaceKHR surface = nullptr;

    vk::raii::PhysicalDevice physical_device = nullptr;
    vk::raii::Device device = nullptr;

    vk::raii::Queue graphics_queue = nullptr;
    vk::raii::Queue present_queue = nullptr;

    struct QueueFamilyIndices {
        int graphics_family = -1;
        int present_family = -1;

        bool is_complete() const {
            return graphics_family >= 0 && present_family >= 0;
        };
    } family_indices;

    // surface
    struct SurfaceConfig {
        vk::SurfaceCapabilitiesKHR capabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR> present_modes;

        vk::SurfaceFormatKHR chosen_format;
        vk::PresentModeKHR chosen_present_mode;
        vk::Extent2D chosen_extent;

        uint32_t image_count;
    } config;

  private:
    GLFWwindow *window;

    void create_instance();

    void pick_physical_device();

    void find_queue_families();

    void create_logical_device();

    void create_surface();

    void surface_config();
};

#endif // !VULKAN_CONTEXT_HPP
