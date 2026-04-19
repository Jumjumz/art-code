add_library(api
  artcode.cpp
)

target_include_directories(api
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(api
  PRIVATE glm::glm
)
