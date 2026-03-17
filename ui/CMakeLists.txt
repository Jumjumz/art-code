add_library(ui
  navigation.cpp
)

target_include_directories(ui
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)
