add_library(ui
  navigation.cpp
  text_editor.cpp
)

target_include_directories(ui
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ui
  PRIVATE imgui
  PRIVATE utils
)
