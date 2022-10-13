# Project configuration.

set(PROJECT_TITLE "ccleste")

set(project_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/src/celeste.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/sdl12main.c)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/export/data)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/data/font.bmp ${CMAKE_CURRENT_SOURCE_DIR}/export/data COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/data/gfx.bmp  ${CMAKE_CURRENT_SOURCE_DIR}/export/data COPYONLY)
