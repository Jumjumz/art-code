add_library(ui
  main_nav.cpp
  ui_manager.cpp
  start_screen/artboard_settings.cpp
  start_screen/template_gallery.cpp
  workspace/build_panel.cpp
  workspace/canvas.cpp
  workspace/console.cpp
  workspace/development.cpp
  workspace/text_editor.cpp
)

target_include_directories(ui
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ui
  PRIVATE imgui
  PRIVATE utils
)
