#pragma once

#include <vulkan/vulkan_raii.hpp>

class VulkanSwapchain {
  public:
    VulkanSwapchain(const vk::raii::SurfaceKHR &surface,
                    const vk::raii::Device &device,
                    const vk::SurfaceCapabilitiesKHR &capabilities,
                    const vk::SurfaceFormatKHR &format,
                    const vk::PresentModeKHR &present_mode,
                    const vk::Extent2D &extent, const int &graphics_family,
                    const int &present_family, const uint32_t &image_count);

    vk::raii::SwapchainKHR swapchain = nullptr;

    struct SwapchainResources {
        std::vector<vk::Image> images;
        std::vector<vk::raii::ImageView> image_views;

        vk::Format image_format;
        vk::Extent2D extent;
    } resources;

    void create_swapchain(const vk::Extent2D &extent);

    void create_image_views();

  private:
    const vk::raii::SurfaceKHR &surface;

    const vk::raii::Device &device;

    const vk::SurfaceCapabilitiesKHR &capabilities;
    const vk::SurfaceFormatKHR &format;
    const vk::PresentModeKHR &present_mode;
    const vk::Extent2D &extent;

    const int &graphics_family;
    const int &present_family;

    const uint32_t &image_count;
};
