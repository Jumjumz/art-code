#include "vulkan_swapchain.hpp"

VulkanSwapchain::VulkanSwapchain(const vk::raii::SurfaceKHR &surface,
                                 const vk::raii::PhysicalDevice &physical_device,
                                 const vk::raii::Device &device,
                                 const vk::SurfaceCapabilitiesKHR &capabilities,
                                 const vk::SurfaceFormatKHR &format,
                                 const vk::PresentModeKHR &present_mode,
                                 const vk::Extent2D &extent,
                                 const int &graphics_family,
                                 const int &present_family,
                                 const uint32_t &image_count)
    : surface(surface), physical_device(physical_device), device(device),
      capabilities(capabilities), format(format), present_mode(present_mode),
      extent(extent), graphics_family(graphics_family),
      present_family(present_family), image_count(image_count) {
    create_swapchain();
    create_image_views();
    find_depth_format();
};

void VulkanSwapchain::create_swapchain() {
    vk::SwapchainCreateInfoKHR swapchain_info{};
    swapchain_info.flags = vk::SwapchainCreateFlagsKHR();
    swapchain_info.surface = this->surface;
    swapchain_info.minImageCount = this->image_count;
    swapchain_info.imageFormat = this->format.format;
    swapchain_info.imageColorSpace = this->format.colorSpace;
    swapchain_info.imageExtent = this->extent;
    swapchain_info.imageArrayLayers = 1;
    swapchain_info.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

    uint32_t queue_family_indices[] = {
        static_cast<uint32_t>(this->graphics_family),
        static_cast<uint32_t>(this->present_family)};

    if (this->graphics_family != this->present_family) {
        swapchain_info.imageSharingMode = vk::SharingMode::eConcurrent;
        swapchain_info.queueFamilyIndexCount = 2;
        swapchain_info.pQueueFamilyIndices = queue_family_indices;
    } else {
        swapchain_info.imageSharingMode = vk::SharingMode::eExclusive;
        swapchain_info.queueFamilyIndexCount = 0;
        swapchain_info.pQueueFamilyIndices = nullptr;
    }

    swapchain_info.preTransform = this->capabilities.currentTransform;
    swapchain_info.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    swapchain_info.presentMode = this->present_mode;
    swapchain_info.clipped = vk::True;
    swapchain_info.oldSwapchain = nullptr;

    this->swapchain = vk::raii::SwapchainKHR{this->device, swapchain_info};
    this->resources.images = this->swapchain.getImages();
};

void VulkanSwapchain::create_image_views() {
    this->resources.image_views.clear();

    for (auto const &image : this->resources.images) {
        vk::ImageViewCreateInfo image_info{};
        image_info.image = image;
        image_info.format = this->format.format;
        image_info.viewType = vk::ImageViewType::e2D;
        image_info.components.r = vk::ComponentSwizzle::eIdentity;
        image_info.components.g = vk::ComponentSwizzle::eIdentity;
        image_info.components.b = vk::ComponentSwizzle::eIdentity;
        image_info.components.a = vk::ComponentSwizzle::eIdentity;
        image_info.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0,
                                       1};

        this->resources.image_views.emplace_back(this->device, image_info,
                                                 nullptr);
    }

    this->resources.extent = this->extent;
    this->resources.image_format = this->format.format;
};

void VulkanSwapchain::find_depth_format() {
    this->depth_format =
        supported_format({vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint,
                          vk::Format::eD24UnormS8Uint},
                         vk::ImageTiling::eOptimal,
                         vk::FormatFeatureFlagBits::eDepthStencilAttachment);
};

vk::Format
VulkanSwapchain::supported_format(const std::vector<vk::Format> &candidates,
                                  const vk::ImageTiling tiling,
                                  const vk::FormatFeatureFlags features) {
    for (const auto format : candidates) {
        vk::FormatProperties props =
            this->physical_device.getFormatProperties(format);

        if (tiling == vk::ImageTiling::eLinear &&
            (props.linearTilingFeatures & features) == features)
            return format;

        if (tiling == vk::ImageTiling::eOptimal &&
            (props.optimalTilingFeatures & features) == features)
            return format;
    }

    throw std::runtime_error("Failed to find supported format!");
};
