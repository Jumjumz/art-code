add_library(api SHARED
  artcode.cpp
)

add_custom_command(TARGET api POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy
  ${CMAKE_CURRENT_SOURCE_DIR}/artcode.hpp
  ${CMAKE_CURRENT_BINARY_DIR}/artcode.hpp
)

target_include_directories(api
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(api
  PRIVATE glm::glm
  PRIVATE utils
)
