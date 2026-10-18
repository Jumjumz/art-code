add_library(api SHARED
  artcode.cpp
)

if (WIN32)
  # for windows dll exports
  target_compile_definitions(api PRIVATE
    $<$<BOOL:${WIN32}>:API_EXPORTS>
  )
endif()
add_custom_command(TARGET api POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy
  ${CMAKE_CURRENT_SOURCE_DIR}/artcode.hpp
  ${CMAKE_CURRENT_BINARY_DIR}/artcode.hpp
)
# include glm headers
add_custom_command(TARGET api POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${glm_SOURCE_DIR}/glm
  ${CMAKE_CURRENT_BINARY_DIR}/glm
)

target_include_directories(api
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(api
  PRIVATE
    GRAPHICS_STACK
    utils
)
