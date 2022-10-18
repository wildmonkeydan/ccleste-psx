# Project configuration.

set(PROJECT_TITLE "ccleste")

set(project_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/src/celeste.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/sdl12main.c)

set(UID3     0x1000c37e) # game.exe UID
set(APP_UID  0x10005731) # launcher.app UID
set(APP_NAME "Celeste")

if(NOT BUILD_FOR_NOKIA_NGAGE)
  file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data DESTINATION ${EXPORT_DIR})
endif()

if(CELESTE_P8_FIXEDP)
  SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_SOURCE_DIR}/src/celeste.c PROPERTIES LANGUAGE CXX )
  add_compile_definitions(${PROJECT_TITLE} CELESTE_P8_FIXEDP)
endif()

if(CELESTE_P8_ENABLE_AUDIO AND NOT BUILD_FOR_NOKIA_NGAGE)
  add_compile_definitions(${PROJECT_TITLE} CELESTE_P8_ENABLE_AUDIO)
endif()

if(CELESTE_P8_HACKED_BALLOONS)
  add_compile_definitions(${PROJECT_TITLE} CELESTE_P8_HACKED_BALLOONS)
endif()

if(CELESTE_P8_N3DS_DEBUG)
  add_compile_definitions(${PROJECT_NAME} N3DS_DEBUG)
endif()
