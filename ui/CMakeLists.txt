add_library(ui
  ui_manager.cpp
  workspace/canvas.cpp
  workspace/navigation.cpp
  workspace/text_editor.cpp
)

target_include_directories(ui
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ui
  PRIVATE imgui
  PRIVATE utils
)
