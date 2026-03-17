add_library(ui
  core/imgui.cpp
  core/imgui_draw.cpp
  core/imgui_tables.cpp
  core/imgui_widgets.cpp
  backends/imgui_impl_glfw.cpp
  backends/imgui_impl_vulkan.cpp
)

target_include_directories(ui
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/core
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/backends
)
