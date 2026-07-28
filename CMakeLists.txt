cmake_minimum_required(VERSION 3.10)
project(Art-Code VERSION 0.10)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

# dependencies
FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG 3.3.10
)

# turn off docs/examples
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG 0.9.9.8
)

FetchContent_MakeAvailable(glfw glm)

set(SOURCES
  src/main.cpp
)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

find_package(Vulkan REQUIRED)

add_subdirectory(api)
add_subdirectory(assets)
add_subdirectory(core)
add_subdirectory(imgui)
add_subdirectory(renderer)
add_subdirectory(shaders)
add_subdirectory(solution)
add_subdirectory(ui)
add_subdirectory(utils)
add_subdirectory(vulkan)
add_subdirectory(window)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE
  Vulkan::Vulkan
  glm::glm
  api
  core
  imgui
  renderer
  solution
  ui
  utils
  vulkan
  window
)

add_dependencies(${PROJECT_NAME} assets)
add_dependencies(${PROJECT_NAME} shaders)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_options(${PROJECT_NAME} PRIVATE
  -fsanitize=address
  -fno-omit-frame-pointer
  -g3
  -O0
  -Wall
  -Wextra
)

  target_link_options(${PROJECT_NAME} PRIVATE -fsanitize=address)
endif()
