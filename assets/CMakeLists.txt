add_custom_target(assets ALL
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_SOURCE_DIR}/assets/fonts
  ${CMAKE_BINARY_DIR}/assets/fonts
  COMMENT "Copying fonts to build dir..."
)
