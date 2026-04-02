find_program(GLSL_VALIDATOR glslangValidator)

file(GLOB SHADER_SOURCES "*.vert" "*.frag" "*.comp")

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/shaders")

foreach(SHADER ${SHADER_SOURCES})
  get_filename_component(SHADER_NAME ${SHADER} NAME)
  set(OUTPUT_SPV "${CMAKE_BINARY_DIR}/shaders/${SHADER_NAME}.spv")

  add_custom_command(
    OUTPUT ${OUTPUT_SPV}
    COMMAND glslangValidator -V ${SHADER} -o ${OUTPUT_SPV}
    DEPENDS ${SHADER}
    COMMENT "compiling: ${SHADER_NAME}"
  )

  list(APPEND SPV_SHADERS ${OUTPUT_SPV})
endforeach()

add_custom_target(shaders ALL DEPENDS ${SPV_SHADERS})
