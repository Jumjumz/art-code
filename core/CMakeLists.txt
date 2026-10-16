add_library(core
  app.cpp
)

target_include_directories(core
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(core
  PRIVATE
    GRAPHICS_STACK
    imgui
    renderer
    ui
    utils
    vulkan
    window
)
