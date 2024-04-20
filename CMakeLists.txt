cmake_minimum_required(VERSION 3.14.7)

project(raygun LANGUAGES C VERSION 0.1 DESCRIPTION "A C library for prototyping rendering algorithms.")

option(RAYGUN_DEMO "Whether or not to build the demo program." OFF)
option(RAYGUN_NO_COMPILER_WARNINGS "Whether or not to disable the compiler warnings." OFF)

find_package(embree 3 CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(OpenMP REQUIRED COMPONENTS C)

include(pack_files.cmake)

pack_files("${CMAKE_CURRENT_BINARY_DIR}/shaders.h" rg_shaders_
  shaders/quad.vert
  shaders/accumulate.frag
  shaders/tone.frag)

add_library(raygun
  raygun.h
  #raygun.c
  src/api.c
  src/random.h
  src/quad2d.h
  src/quad2d.c
  src/runtime.h
  src/runtime.c
  src/pipeline.h
  src/pipeline.c
  src/shader.h
  src/shader.c
  src/framebuffer.h
  src/framebuffer.c
  "${CMAKE_CURRENT_BINARY_DIR}/shaders.h"
  glad/include/glad/glad.h
  glad/include/KHR/khrplatform.h
  glad/src/glad.c)

target_compile_definitions(raygun
  PRIVATE
    GLFW_INCLUDE_NONE=1)

if(CMAKE_COMPILER_IS_GNUCC AND NOT RAYGUN_NO_COMPILER_WARNINGS)
  target_compile_options(raygun
    PRIVATE
      -Wall -Wextra -Werror -Wfatal-errors -Wconversion)
endif()

target_include_directories(raygun
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
  PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/glad/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/third_party/linmath>)

target_link_libraries(raygun
  PUBLIC
    embree
    glfw
    OpenMP::OpenMP_C)

if(RAYGUN_DEMO)
  enable_language(CXX)
  add_executable(raygun_demo
    demo/main.cpp)
  target_link_libraries(raygun_demo PRIVATE raygun)
  set_target_properties(raygun_demo PROPERTIES OUTPUT_NAME demo)
endif()
