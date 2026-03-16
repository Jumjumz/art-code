#ifndef VULKAN_SWAPCHAIN_HPP
#define VULKAN_SWAPCHAIN_HPP

#pragma once

#include <vulkan/vulkan_raii.hpp>

class VulkanSwapchain {
  public:
    VulkanSwapchain(const vk::raii::SurfaceKHR &surface,
                    const vk::raii::PhysicalDevice &physical_device,
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

    vk::Format depth_format;

    void create_swapchain();

    void create_image_views();

  private:
    const vk::raii::SurfaceKHR &surface;

    const vk::raii::PhysicalDevice &physical_device;

    const vk::raii::Device &device;

    const vk::SurfaceCapabilitiesKHR capabilities;
    const vk::SurfaceFormatKHR format;
    const vk::PresentModeKHR present_mode;
    const vk::Extent2D extent;

    const int graphics_family;
    const int present_family;

    const uint32_t image_count;

    void find_depth_format();

    vk::Format supported_format(const std::vector<vk::Format> &candidates,
                                const vk::ImageTiling tiling,
                                const vk::FormatFeatureFlags features);
};

#endif // !VULKAN_SWAPCHAIN_HPP
