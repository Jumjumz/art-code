add_library(imgui
  core/imgui.cpp
  core/imgui_draw.cpp
  core/imgui_tables.cpp
  core/imgui_stdlib.cpp
  core/imgui_widgets.cpp
  core/TextEditor.cpp
  backends/imgui_impl_glfw.cpp
  backends/imgui_impl_vulkan.cpp
)

target_include_directories(imgui
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/core
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/backends
)

target_link_libraries(imgui
PRIVATE Vulkan::Vulkan)
