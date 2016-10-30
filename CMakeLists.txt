cmake_minimum_required(VERSION 3.1)
project(LibSamplerate LANGUAGES C)

set(SAMPLERATE_SRC 
	${PROJECT_SOURCE_DIR}/src/samplerate.c
	${PROJECT_SOURCE_DIR}/src/src_linear.c
	${PROJECT_SOURCE_DIR}/src/src_sinc.c
	${PROJECT_SOURCE_DIR}/src/src_zoh.c)

if(WIN32)
	set(SAMPLERATE_SRC 
		${SAMPLERATE_SRC}
		${PROJECT_SOURCE_DIR}/Win32/libsamplerate-0.def)
endif()

add_library(samplerate ${SAMPLERATE_SRC})

if(WIN32)
	target_compile_definitions(samplerate PRIVATE "WIN32" "_USRDLL")
	target_include_directories(samplerate PUBLIC ${PROJECT_SOURCE_DIR}/Win32)
endif()

target_include_directories(samplerate PUBLIC ${PROJECT_SOURCE_DIR}/src)

