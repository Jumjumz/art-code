add_library(artcode
  artcode.cpp
)

target_include_directories(imgui
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(window
  PRIVATE glm::glm
)
