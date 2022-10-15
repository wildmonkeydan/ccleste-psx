# Project configuration.

set(PROJECT_TITLE "ccleste")

set(project_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/src/celeste.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/sdl12main.c)

set(UID3     0x1000c37e) # game.exe UID
set(APP_UID  0x10005731) # launcher.app UID
set(APP_NAME "Celeste")

file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/export/data)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/data/font.bmp ${CMAKE_CURRENT_SOURCE_DIR}/export/data COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/data/gfx.bmp  ${CMAKE_CURRENT_SOURCE_DIR}/export/data COPYONLY)

if(CELESTE_P8_FIXEDP)
  SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_SOURCE_DIR}/src/celeste.c PROPERTIES LANGUAGE CXX )
  add_compile_definitions(${PROJECT_TITLE} CELESTE_P8_FIXEDP)
endif()
