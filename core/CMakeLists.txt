add_library(core
  art_code.cpp
)

target_include_directories(core
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(core
PRIVATE utils
PRIVATE vulkan
PRIVATE window
PRIVATE imgui
)
