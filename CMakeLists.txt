cmake_minimum_required(VERSION 3.1)
project(libsamplerate VERSION 0.1.9 LANGUAGES C)

option(LIBSAMPLERATE_TESTS "Enable to generate test targets" ON)
option(LIBSAMPLERATE_INSTALL "Enable to add install directives" ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)

include(TestBigEndian)
include(CheckFunctionExists)
include(CheckIncludeFiles)
include(CheckSymbolExists)
include(CheckTypeSize)
include(ClipMode)

set(SAMPLERATE_SRC
	${PROJECT_SOURCE_DIR}/src/samplerate.c
	${PROJECT_SOURCE_DIR}/src/src_linear.c
	${PROJECT_SOURCE_DIR}/src/src_sinc.c
	${PROJECT_SOURCE_DIR}/src/src_zoh.c)

if(WIN32)
	set(OS_IS_WIN32 TRUE)
	set(SAMPLERATE_SRC
		${SAMPLERATE_SRC}
		${PROJECT_SOURCE_DIR}/Win32/libsamplerate-0.def)
	include_directories(Win32)
endif()

test_big_endian(CPU_IS_BIG_ENDIAN)
if(CPU_IS_BIG_ENDIAN)
	set(CPU_IS_LITTLE_ENDIAN 0)
else()
	set(CPU_IS_LITTLE_ENDIAN 1)
endif()

check_function_exists(pow RESULT)
if(NOT RESULT)
  list(APPEND CMAKE_REQUIRED_LIBRARIES m)
  set(NEED_MATH)
endif()
check_function_exists(alarm HAVE_ALARM)
check_function_exists(lrint HAVE_LRINT)
check_function_exists(lrintf HAVE_LRINTF)
check_function_exists(signal HAVE_SIGNAL)

check_include_files(stdint.h HAVE_STDINT)
check_include_files(sys/times.h HAVE_SYS_TIMES_H)

check_symbol_exists(SIGALRM signal.h HAVE_SIGALRM)

check_type_size(int SIZEOF_INT)
check_type_size(long SIZEOF_LONG)

# This will set CPU_CLIPS_NEGATIVE and CPU_CLIPS_POSITIVE
clip_mode()

find_package(ALSA)
set(HAVE_ALSA ${ALSA_FOUND})
if(ALSA_FOUND)
  include_directories("${ALSA_INCLUDE_DIR}")
endif()

find_package(Sndfile)
set(HAVE_SNDFILE ${SNDFILE_FOUND})
if(SNDFILE_FOUND)
  include_directories("${SNDFILE_INCLUDE_DIR}")
endif()

find_package(FFTW)
set(HAVE_FFTW3 ${FFTW_FOUND})
if(FFTW_FOUND)
  include_directories("${FFTW_INCLUDE_DIR}")
endif()

configure_file(${PROJECT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

add_library(samplerate ${SAMPLERATE_SRC})

if(BUILD_SHARED_LIBS AND WIN32)
	if (MSVC)
		set_target_properties(samplerate PROPERTIES OUTPUT_NAME "libsamplerate-0")
	else()
		set_target_properties(samplerate PROPERTIES OUTPUT_NAME "samplerate-0")
	endif()
endif()

target_include_directories(samplerate PUBLIC
	${PROJECT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR})
    
if(LIBSAMPLERATE_TESTS)

enable_testing()

file(GLOB TEST_SRCS ${PROJECT_SOURCE_DIR}/tests/*_test.c)

foreach(testSrc ${TEST_SRCS})
	get_filename_component(testName ${testSrc} NAME_WE)
	add_executable(${testName}
		${testSrc}
		${PROJECT_SOURCE_DIR}/tests/util.c
		${PROJECT_SOURCE_DIR}/tests/calc_snr.c)
	target_link_libraries(${testName} PUBLIC samplerate ${CMAKE_REQUIRED_LIBRARIES})
	if(FFTW_FOUND)
		target_link_libraries(${testName} PUBLIC ${FFTW_LIBRARY})
	endif()
	add_test(NAME ${testName} COMMAND ${testName})
endforeach(testSrc)

set(EXAMPLE_SRCS
	${PROJECT_SOURCE_DIR}/examples/timewarp-file.c
	${PROJECT_SOURCE_DIR}/examples/varispeed-play.c)

foreach(exampleSrc ${EXAMPLE_SRCS})
	get_filename_component(exampleName ${exampleSrc} NAME_WE)
	add_executable(${exampleName}
		${exampleSrc}
		${PROJECT_SOURCE_DIR}/examples/audio_out.c)
	target_link_libraries(${exampleName} PUBLIC samplerate ${CMAKE_REQUIRED_LIBRARIES})
	if(ALSA_FOUND)
		target_link_libraries(${exampleName} PUBLIC ${ALSA_LIBRARY})
	endif()
	if(SNDFILE_FOUND)
		target_link_libraries(${exampleName} PUBLIC ${SNDFILE_LIBRARY})
	endif()
	if(WIN32)
		target_link_libraries(${exampleName} PUBLIC winmm)
	endif()
	if (APPLE)
		target_link_libraries (${exampleName} PUBLIC  "-framework CoreAudio")
	endif()
endforeach(exampleSrc)
endif() 

if(LIBSAMPLERATE_INSTALL)
set(prefix ${CMAKE_INSTALL_PREFIX})
set(exec_prefix "\${prefix}")
set(includedir "\${prefix}/include")
set(libdir "\${exec_prefix}/lib")
set(VERSION "${PROJECT_VERSION}")
if(NEED_MATH)
	set(LIBS "-lm")
endif()
configure_file(samplerate.pc.in samplerate.pc @ONLY)

install(TARGETS samplerate DESTINATION lib)
install(FILES src/samplerate.h DESTINATION include)
install(DIRECTORY doc/ DESTINATION share/doc/libsamplerate)
install(FILES ${CMAKE_BINARY_DIR}/samplerate.pc DESTINATION lib/pkgconfig)

endif()
