#ifndef ART_CODE_HPP
#define ART_CODE_HPP

#pragma once

#include "vulkan_context.hpp"
#include "vulkan_graphics.hpp"
#include "vulkan_swapchain.hpp"
#include "window.hpp"

class ArtCode {
  public:
    ArtCode();

    void run();

  private:
    uint32_t width = 1440;
    uint32_t aspect = 16 / 9;

    Window window{this->width, this->aspect};

    VulkanContext ctx{this->window.app_window};

    VulkanSwapchain swapchain{this->ctx.surface,
                              this->ctx.physical_device,
                              this->ctx.device,
                              this->ctx.config.capabilities,
                              this->ctx.config.chosen_format,
                              this->ctx.config.chosen_present_mode,
                              this->ctx.config.chosen_extent,
                              this->ctx.family_indices.graphics_family,
                              this->ctx.family_indices.present_family,
                              this->ctx.config.image_count};

    VulkanGraphics pipeline{this->ctx.physical_device, this->ctx.device,
                            this->swapchain.resources.image_format,
                            this->swapchain.depth_format,
                            this->ctx.family_indices.graphics_family};

    void loop();

    void cleanup() const;
};

#endif // !ART_CODE_HPP
