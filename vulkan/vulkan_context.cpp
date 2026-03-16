#include "vulkan_context.hpp"
#include "ndebug.h"
#include <map>
#include <set>

VulkanContext::VulkanContext(GLFWwindow *window) : window(window) {
    create_instance();
    pick_physical_device();
    create_surface();
    create_logical_device();
};

void VulkanContext::create_instance() {
    constexpr vk::ApplicationInfo app_info{
        "Art Code", VK_MAKE_VERSION(0, 0, 1), "Jumz Art Engine",
        VK_MAKE_VERSION(0, 0, 1), vk::ApiVersion13};

    uint32_t extension_count = 0;
    auto glfw_extensions = glfwGetRequiredInstanceExtensions(&extension_count);

    std::vector extensions(glfw_extensions, glfw_extensions + extension_count);
    auto extension_properties =
        this->context.enumerateInstanceExtensionProperties();

    auto unsupported_property = std::ranges::find_if(
        extensions, [&extension_properties](const auto &extension) {
            return std::ranges::none_of(
                extension_properties, [extension](const auto &extension_property) {
                    return strcmp(extension_property.extensionName, extension) ==
                           0;
                });
        });

    if (unsupported_property != extensions.end()) {
        throw std::runtime_error("Required extension not supported: ");
    }

    std::vector<char const *> required_layers;

    // enable validation layers
    if (enable_validation_layers)
        required_layers.assign(validation_layers.begin(),
                               validation_layers.end());

    auto layer_properties = this->context.enumerateInstanceLayerProperties();

    if (std::ranges::any_of(required_layers, [&layer_properties](
                                                 const auto &requiredLayer) {
            return std::ranges::none_of(
                layer_properties, [requiredLayer](auto const &layerProperty) {
                    return strcmp(layerProperty.layerName, requiredLayer) == 0;
                });
        })) {
        throw std::runtime_error(
            "One or more required layers are not supported!");
    }

    vk::InstanceCreateInfo instance_info;
    instance_info.pApplicationInfo = &app_info;

    if (enable_validation_layers) {
        instance_info.enabledLayerCount =
            static_cast<uint32_t>(required_layers.size());
        instance_info.ppEnabledLayerNames = required_layers.data();
    }

    instance_info.enabledExtensionCount = extension_count;
    instance_info.ppEnabledExtensionNames = extensions.data();

    // create instance
    this->instance = vk::raii::Instance{this->context, instance_info, nullptr};
};

void VulkanContext::pick_physical_device() {
    auto devices = this->instance.enumeratePhysicalDevices();

    if (devices.empty())
        throw std::runtime_error("Failed to find GPUs with Vulkan Support!");

    std::multimap<int, vk::raii::PhysicalDevice> candidates;

    for (const auto &device : devices) {
        auto properties = device.getProperties();
        auto features = device.getFeatures();
        uint32_t score = 0;

        if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
            score += 1000;
        } else if (properties.deviceType ==
                   vk::PhysicalDeviceType::eIntegratedGpu) {
            score += 100;
        }

        score += properties.limits.maxImageDimension2D;

        if (!features.geometryShader) {
            continue;
        }

        candidates.insert(std::make_pair(score, device));
    };

    if (candidates.rbegin()->first > 0) {
        this->physical_device = candidates.rbegin()->second;
    } else {
        throw std::runtime_error("Failed to find a suitable GPU!");
    }
};

void VulkanContext::create_logical_device() {
    find_queue_families();

    std::vector<vk::DeviceQueueCreateInfo> deviceQueueInfos;
    std::set<int> unique_queue_families = {
        this->family_indices.graphics_family,
        this->family_indices.present_family,
    };

    float queuePriority = 0.5f;
    for (const int queue_family : unique_queue_families) {
        vk::DeviceQueueCreateInfo deviceQueueInfo{};
        deviceQueueInfo.queueFamilyIndex = queue_family;
        deviceQueueInfo.queueCount = 1;
        deviceQueueInfo.pQueuePriorities = &queuePriority;

        deviceQueueInfos.push_back(deviceQueueInfo);
    }

    vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan13Features,
                       vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
        feature_chain{};

    auto &features = feature_chain.get<vk::PhysicalDeviceFeatures2>();
    features.features.geometryShader = vk::True;
    features.features.samplerAnisotropy = vk::True;
    features.features.dualSrcBlend = vk::True;
    features.features.robustBufferAccess = vk::True;

    auto &dynamic_rendering =
        feature_chain.get<vk::PhysicalDeviceVulkan13Features>();
    dynamic_rendering.dynamicRendering = vk::True;
    dynamic_rendering.synchronization2 = vk::True;

    auto &dynamic_state =
        feature_chain.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
    dynamic_state.extendedDynamicState = vk::True;

    // get swapchains extensions
    const std::vector<const char *> device_extensions = {
        vk::KHRSwapchainExtensionName, vk::EXTExtendedDynamicState3ExtensionName};

    vk::DeviceCreateInfo deviceInfo{};
    deviceInfo.pNext = &features;
    deviceInfo.queueCreateInfoCount = 1;
    deviceInfo.pQueueCreateInfos = deviceQueueInfos.data();
    deviceInfo.enabledExtensionCount =
        static_cast<uint32_t>(device_extensions.size());
    deviceInfo.ppEnabledExtensionNames = device_extensions.data();

    this->device = vk::raii::Device{this->physical_device, deviceInfo};

    this->graphics_queue = vk::raii::Queue{
        this->device,
        static_cast<uint32_t>(this->family_indices.graphics_family), 0};

    this->present_queue = vk::raii::Queue{
        this->device,
        static_cast<uint32_t>(this->family_indices.present_family), 0};
};

void VulkanContext::find_queue_families() {
    std::vector<vk::QueueFamilyProperties> familyProperties =
        this->physical_device.getQueueFamilyProperties();

    for (size_t i = 0; i < familyProperties.size(); i++) {
        if (familyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics) {
            this->family_indices.graphics_family = i;
        }

        vk::Bool32 present =
            this->physical_device.getSurfaceSupportKHR(i, this->surface);

        if (present) {
            this->family_indices.present_family = i;
        }

        if (this->family_indices.is_complete()) {
            break;
        }
    }
};

void VulkanContext::create_surface() {
    VkSurfaceKHR surface;

    if (glfwCreateWindowSurface(*this->instance, this->window, nullptr,
                                &surface) != 0) {
        throw std::runtime_error("Failed to create window surface");
    }

    this->surface = vk::raii::SurfaceKHR{this->instance, surface, nullptr};

    surface_config();

    this->config.chosen_format = this->config.formats[0];
    for (const auto &format : this->config.formats) {
        if (format.format == vk::Format::eB8G8R8A8Srgb &&
            format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            this->config.chosen_format = format;
            break;
        }
    }

    this->config.chosen_present_mode =
        vk::PresentModeKHR::eFifo; // V-Sync capped at 60fps

    // choose extent
    if (this->config.capabilities.currentExtent.width != UINT32_MAX) {
        this->config.chosen_extent = this->config.capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(this->window, &width, &height);

        this->config.chosen_extent.width =
            std::clamp(static_cast<uint32_t>(width),
                       this->config.capabilities.minImageExtent.width,
                       this->config.capabilities.maxImageExtent.width);

        this->config.chosen_extent.height =
            std::clamp(static_cast<uint32_t>(height),
                       this->config.capabilities.minImageExtent.height,
                       this->config.capabilities.maxImageExtent.height);
    }

    this->config.image_count = this->config.capabilities.minImageCount + 1;
    if (this->config.capabilities.maxImageCount > 0 &&
        this->config.image_count > this->config.capabilities.maxImageCount) {
        this->config.image_count = this->config.capabilities.maxImageCount;
    }
};

void VulkanContext::surface_config() {
    this->config.capabilities =
        this->physical_device.getSurfaceCapabilitiesKHR(this->surface);

    this->config.formats =
        this->physical_device.getSurfaceFormatsKHR(this->surface);

    this->config.present_modes =
        this->physical_device.getSurfacePresentModesKHR(this->surface);
};
