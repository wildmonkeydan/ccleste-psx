# PSn00bSDK example CMake script
# (C) 2021 spicyjpeg - MPL licensed

cmake_minimum_required(VERSION 3.20)

add_compile_definitions(CELESTE_P8_FIXEDP)

project(
	cceleste-psx
	LANGUAGES    C CXX ASM
	VERSION      1.0.0
	DESCRIPTION  "A port of cceleste to the PSX"
	HOMEPAGE_URL "https://github.com/wildmonkeydan/ccleste-psx"
)

SET_SOURCE_FILES_PROPERTIES(${sourceDir}src/celeste.c PROPERTIES LANGUAGE CXX )
psn00bsdk_add_executable(celeste STATIC ${sourceDir}src/psxmain.c ${sourceDir}src/celeste.c)

psn00bsdk_add_cd_image(
	iso      # Target name
	celeste # Output file name (= template.bin + template.cue)
	iso.xml  # Path to config file
	DEPENDS celeste
)
