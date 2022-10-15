# Do not modify this file unless absolutely necessary.
# All project-specific settings can be found in the file:
# project_config.cmake

cmake_minimum_required(VERSION 3.10)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Use CMake or Visual Studio to enable these settings.
option(BUILD_FOR_NOKIA_NGAGE      "Build for the Nokia N-Gage"                           OFF)
option(NGAGE_DEBUG                "Enable Nokia N-Gage specific adjustments"             OFF)
option(CELESTE_P8_FIXEDP          "Use fixed-point arithmetic"                           OFF)
option(CELESTE_P8_HACKED_BALLOONS "Always expand the baloon hitbox to their full range." OFF)
option(CELESTE_P8_ENABLE_AUDIO    "Enable SDL2_mixer audio output."                      OFF)

if(BUILD_FOR_NOKIA_NGAGE)
  include(Nokia_N-Gage)
  return()
endif()

project(Celeste C CXX)

set(CMAKE_C_STANDARD 90)
set(CMAKE_CXX_STANDARD 11)

set(EXPORT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/export)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${EXPORT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${EXPORT_DIR})

include(project_config)
include(get_sdl2)

get_sdl2("2.24.1")

if(NOT BUILD_FOR_NOKIA_NGAGE AND CELESTE_P8_ENABLE_AUDIO)
  get_sdl2_mixer("2.6.2")
endif()

add_executable(${PROJECT_TITLE} ${project_sources})

target_include_directories(
  ${PROJECT_TITLE} 
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src
  PRIVATE ${SDL2_INCLUDE_DIR})

if(NGAGE_DEBUG)
  add_compile_definitions(
    ${PROJECT_TITLE}
    NGAGE_DEBUG)
endif()

add_compile_definitions(
  ${PROJECT_TITLE}
  _CRT_SECURE_NO_WARNINGS)

if(NOT BUILD_FOR_NOKIA_NGAGE AND CELESTE_P8_ENABLE_AUDIO)
  target_link_libraries(${PROJECT_TITLE} PRIVATE ${SDL2_MIXER_LIBS})
endif()

target_link_libraries(${PROJECT_TITLE} PRIVATE ${SDL2_LIBRARIES})

if(NINTENDO_3DS)
  set(ROMFS_DIR "${CMAKE_CURRENT_BINARY_DIR}/romfs")
  file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data DESTINATION ${ROMFS_DIR})
  set(SMDH_FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_TITLE}.smdh")
  ctr_generate_smdh("${SMDH_FILE}"
    NAME "Celeste"
    DESCRIPTION "SDL Port of the original Celeste"
    AUTHOR "Original port by: Maddy Thorson & Noel Berry"
    ICON "${CMAKE_CURRENT_SOURCE_DIR}/icon.png")
  ctr_create_3dsx(
    ${PROJECT_TITLE}
    ROMFS "${ROMFS_DIR}"
    SMDH "${SMDH_FILE}")
endif()

if (CMAKE_C_COMPILER_ID STREQUAL "Clang")
  set(COMPILE_OPTIONS
    -Wall
    -Wextra
    -Wpedantic)

elseif (CMAKE_C_COMPILER_ID STREQUAL "GNU")
  set(COMPILE_OPTIONS
    -Wall
    -Wextra
    -Wpedantic)

elseif (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
  set(COMPILE_OPTIONS
    /W4
    /utf-8)
endif()
