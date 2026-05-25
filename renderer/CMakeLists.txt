add_library(renderer
  canvas_renderer.cpp
  artcode_commands.cpp
  artcode_graphics.cpp
)

target_include_directories(renderer
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(renderer
  PRIVATE imgui
  PRIVATE vulkan
  PRIVATE utils
)
