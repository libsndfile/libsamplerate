cmake_minimum_required(VERSION 3.1)
project(LibSamplerate LANGUAGES C)


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)

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

include(TestBigEndian)
test_big_endian(IS_BIG_ENDIAN)
if(IS_BIG_ENDIAN)
	set(CPU_IS_BIG_ENDIAN 1)
	set(CPU_IS_LITTLE_ENDIAN 0)
else()
	set(CPU_IS_BIG_ENDIAN 0)
	set(CPU_IS_LITTLE_ENDIAN 1)
endif()

include(CheckFunctionExists)
check_function_exists(alarm IS_ALARM)
if(IS_ALARM)
	set(HAVE_ALARM 1)
else()
	set(HAVE_ALARM 0)
endif()

check_function_exists(lrint IS_LRINT)
if(IS_LRINT)
	set(HAVE_LRINT 1)
else()
	set(HAVE_LRINT 0)
endif()

check_function_exists(lrintf IS_LRINTF)
if(IS_LRINTF)
	set(HAVE_LRINTF 1)
else()
	set(HAVE_LRINTF 0)
endif()

check_function_exists(signal IS_SIGNAL)
if(IS_SIGNAL)
	set(HAVE_SIGNAL 1)
else()
	set(HAVE_SIGNAL 0)
endif()

include(CheckIncludeFiles)
check_include_files(stdint.h IS_STDINT)
if(IS_STDINT)
	set(HAVE_STDINT_H 1)
else()
	set(HAVE_STDINT_H 0)
endif()

check_include_files(sys/times.h IS_SYS_TIMES)
if(IS_SYS_TIMES)
	set(HAVE_SYS_TIMES_H 1)
else()
	set(HAVE_SYS_TIMES_H 0)
endif()

check_include_files(alsa/asoundlib.h IS_ALSA_ASOUNDLIB)
if(IS_ALSA_ASOUNDLIB)
	set(HAVE_ALSA_ASOUNDLIB_H 1)
else()
	set(HAVE_ALSA_ASOUNDLIB_H 0)
endif()

include(CheckTypeSize)
check_type_size("double" SIZEOF_DOUBLE)
check_type_size("float" SIZEOF_FLOAT)
check_type_size("int" SIZEOF_INT)
check_type_size("long" SIZEOF_LONG)


set(ENV{SNDFILE_ROOT} "${PROJECT_SOURCE_DIR}/extern/yaml-cpp")

find_package(Sndfile)

if(SNDFILE_FOUND)
	set(HAVE_SNDFILE 1)
else()
	set(HAVE_SNDFILE 0)
endif()

configure_file(${PROJECT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

add_library(samplerate ${SAMPLERATE_SRC})

if(WIN32)
	target_compile_definitions(samplerate PRIVATE "WIN32" "_USRDLL")
	target_include_directories(samplerate PUBLIC ${PROJECT_SOURCE_DIR}/Win32)
endif()

target_include_directories(samplerate PUBLIC
	${PROJECT_SOURCE_DIR}/src
	${CMAKE_CURRENT_BINARY_DIR})

