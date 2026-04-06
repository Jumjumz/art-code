add_library(solution
  build.cpp
)

target_include_directories(solution
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(solution
  PRIVATE utils
)
