#pragma once

#include "main_nav.hpp"
#include "start_screen.hpp"
#include "vulkan_buffers.hpp"
#include "vulkan_context.hpp"
#include "window.hpp"
#include "work_space.hpp"

#include <memory>
#include <vector>

class UIManager {
  public:
    UIManager(const Window& window, const VulkanContext& ctx, VulkanBuffers& vk_buffers);

    void render();

    bool show_main_ui = false;

    glm::vec3 artboard_size;

    void canvas_setup();

    void workspace_events();

    void update_canvas();

  private:
    const Window&        window;
    const VulkanContext& ctx;
    VulkanBuffers&       vk_buffers;

    const MainNavigation main_nav;

    static float     zoom;
    static glm::vec2 panning;
    static glm::vec2 mouse_last_pos;

    // key inputs
    bool mouse_in_canvas    = false;
    bool ctrl_pressed       = false;
    bool spacebar_pressed   = false;
    bool left_click_pressed = false;

    std::vector<std::unique_ptr<StartScreen>> start_ui     = {};
    std::vector<std::unique_ptr<WorkSpace>>   workspace_ui = {};
};
