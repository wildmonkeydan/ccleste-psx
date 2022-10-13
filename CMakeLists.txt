# Do not modify this file unless absolutely necessary.
# All project-specific settings can be found in the file:
# project_config.cmake

cmake_minimum_required(VERSION 3.10)

project(SDLexample C CXX)

set(CMAKE_C_STANDARD 90)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/project_config.cmake)

include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)

# SDL2
set(SDL2_VERSION "2.24.0")

if (WIN32)
  if (CMAKE_BUILD_TYPE MATCHES Release)
    set(GUI_TYPE WIN32)
  endif()

  set(SDL2_DEVEL_PKG SDL2-devel-${SDL2_VERSION}-VC.zip)
  set(SDL2_PLATFORM "x64")

  if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(SDL2_PLATFORM "x86")
  endif()

  set(SDL2_PATH ${CMAKE_CURRENT_SOURCE_DIR}/deps/SDL2-${SDL2_VERSION}_${SDL2_PLATFORM})

  ExternalProject_Add(SDL2_devel
    URL https://github.com/libsdl-org/SDL/releases/download/release-${SDL2_VERSION}/${SDL2_DEVEL_PKG}
    URL_HASH SHA1=8a54459189e88c30ba024ee5f18ce4b1a5d1d9e7
    DOWNLOAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps
    DOWNLOAD_NO_PROGRESS true
    TLS_VERIFY true
    SOURCE_DIR ${SDL2_PATH}/
    BUILD_BYPRODUCTS ${SDL2_PATH}/lib/${SDL2_PLATFORM}/SDL2.lib
    BUILD_BYPRODUCTS ${SDL2_PATH}/lib/${SDL2_PLATFORM}/SDL2main.lib

    BUILD_COMMAND ${CMAKE_COMMAND} -E echo "Skipping build step."

    INSTALL_COMMAND ${CMAKE_COMMAND} -E copy
    ${SDL2_PATH}/lib/${SDL2_PLATFORM}/SDL2.dll ${CMAKE_CURRENT_SOURCE_DIR}/export

    PATCH_COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/SDL2_devel.cmake" ${SDL2_PATH}/CMakeLists.txt)

  set(SDL2_INCLUDE_DIR ${SDL2_PATH}/include)
  set(SDL2_LIBRARY     ${SDL2_PATH}/lib/${SDL2_PLATFORM}/SDL2.lib)
  set(SDL2MAIN_LIBRARY ${SDL2_PATH}/lib/${SDL2_PLATFORM}/SDL2main.lib)
endif(WIN32)

if(UNIX)
  set(SDL2_PATH      ${CMAKE_CURRENT_SOURCE_DIR}/deps/SDL2-${SDL2_VERSION})
  set(SDL2_DEVEL_PKG SDL2-${SDL2_VERSION}.tar.gz)

  ExternalProject_Add(SDL2_devel
    URL https://github.com/libsdl-org/SDL/releases/download/release-${SDL2_VERSION}/${SDL2_DEVEL_PKG}
    URL_HASH SHA1=04d7768f4418ba03537ef14a86a0c1c45582f5c3
    DOWNLOAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps
    DOWNLOAD_NO_PROGRESS true
    TLS_VERIFY true
    SOURCE_DIR ${SDL2_PATH}/
    BINARY_DIR ${SDL2_PATH}_build
    BUILD_BYPRODUCTS ${SDL2_PATH}_build/libSDL2.a
    BUILD_BYPRODUCTS ${SDL2_PATH}_build/libSDL2main.a

    INSTALL_COMMAND ${CMAKE_COMMAND} -E echo "Skipping install step."
    PATCH_COMMAND   ${CMAKE_COMMAND} -E echo "Skipping patch step.")

  set(SDL2_INCLUDE_DIR ${SDL2_PATH}/include)
  set(SDL2_LIBRARY     ${SDL2_PATH}_build/libSDL2.a)
  set(SDL2MAIN_LIBRARY ${SDL2_PATH}_build/libSDL2main.a)
endif(UNIX)

include_directories(
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src
  SYSTEM ${SDL2_INCLUDE_DIR})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_CURRENT_SOURCE_DIR}/export)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/export)

add_executable(
  ${PROJECT_TITLE}
  ${GUI_TYPE}
  ${project_sources})

add_dependencies(
  ${PROJECT_TITLE}
  SDL2_devel)

if(WIN32)
  target_link_libraries(
    ${PROJECT_TITLE}
    ${SDL2_LIBRARY}
    ${SDL2MAIN_LIBRARY})
endif(WIN32)

if(UNIX)
  target_link_libraries(
    ${PROJECT_TITLE}
    ${SDL2_LIBRARY}
    ${SDL2MAIN_LIBRARY}
    dl
    m
    pthread)
endif(UNIX)

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
