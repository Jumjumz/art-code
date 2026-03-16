add_library(vulkan
  vulkan_context.cpp
)

target_include_directories(vulkan
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)
