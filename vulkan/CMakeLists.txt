add_library(vulkan
  vulkan_context.cpp
  vulkan_graphics.cpp
  vulkan_swapchain.cpp
)

target_include_directories(vulkan
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(vulkan
  PRIVATE utils
)
