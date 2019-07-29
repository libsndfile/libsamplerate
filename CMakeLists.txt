cmake_minimum_required(VERSION 3.1)
project(libsamplerate VERSION 0.1.9 LANGUAGES C)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(IS_ROOT_PROJECT ON)
else()
    set(IS_ROOT_PROJECT OFF)
endif()

option(LIBSAMPLERATE_TESTS "Enable to generate test targets" ${IS_ROOT_PROJECT})
option(LIBSAMPLERATE_EXAMPLES "Enable to generate examples" ${IS_ROOT_PROJECT})
option(LIBSAMPLERATE_INSTALL "Enable to add install directives" ${IS_ROOT_PROJECT})

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

include(CheckIncludeFiles)
include(CheckSymbolExists)
include(ClipMode)
add_definitions(-DHAVE_CONFIG_H)

if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID MATCHES "Clang")
  option(LIBSAMPLERATE_ENABLE_SANITIZERS "Enable ASAN and UBSAN" OFF)

  if(LIBSAMPLERATE_ENABLE_SANITIZERS)
    # Use ASAN and UBSAN, make it fail on any error, improve stack traces
    set(sanitizer_flags -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer)

    add_compile_options(${sanitizer_flags})
    string(REPLACE ";" " " sanitizer_flags "${sanitizer_flags}")
    string(APPEND CMAKE_EXE_LINKER_FLAGS " ${sanitizer_flags}")
    string(APPEND CMAKE_MODULE_LINKER_FLAGS " ${sanitizer_flags}")
    string(APPEND CMAKE_SHARED_LINKER_FLAGS " ${sanitizer_flags}")
  endif()
endif()

find_library(LIBSAMPLERATE_MATH_LIBRARY m)

# This will set CPU_CLIPS_NEGATIVE and CPU_CLIPS_POSITIVE
clip_mode()

configure_file(config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

add_library(samplerate
    src/samplerate.c
    src/src_linear.c
    src/src_sinc.c
    src/src_zoh.c)

if(BUILD_SHARED_LIBS AND WIN32)
    if (MSVC)
        set_target_properties(samplerate PROPERTIES OUTPUT_NAME "libsamplerate-0")
    else()
        set_target_properties(samplerate PROPERTIES OUTPUT_NAME "samplerate-0")
    endif()
    target_sources(samplerate PRIVATE Win32/libsamplerate-0.def)
endif()

target_include_directories(samplerate
    PUBLIC include
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

if(LIBSAMPLERATE_MATH_LIBRARY)
    target_link_libraries(samplerate PUBLIC ${LIBSAMPLERATE_MATH_LIBRARY})
endif()
    
if(LIBSAMPLERATE_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(LIBSAMPLERATE_EXAMPLES)
    add_subdirectory(examples)
endif() 

if(LIBSAMPLERATE_INSTALL)
    set(prefix ${CMAKE_INSTALL_PREFIX})
    set(exec_prefix "\${prefix}")
    set(includedir "\${prefix}/include")
    set(libdir "\${exec_prefix}/lib")
    set(VERSION "${PROJECT_VERSION}")
    if(LIBSAMPLERATE_MATH_LIBRARY)
        set(LIBS "-lm")
    endif()
    configure_file(samplerate.pc.in samplerate.pc @ONLY)

    install(TARGETS samplerate DESTINATION lib)
    install(FILES src/samplerate.h DESTINATION include)
    install(DIRECTORY doc/ DESTINATION share/doc/libsamplerate)
    install(FILES ${CMAKE_BINARY_DIR}/samplerate.pc DESTINATION lib/pkgconfig)
endif()
