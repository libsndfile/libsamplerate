cmake_minimum_required(VERSION 3.1..3.18)

# Policies

# Include file check macros honor CMAKE_REQUIRED_LIBRARIES, CMake >= 3.12
if(POLICY CMP0075)
  cmake_policy(SET CMP0075 NEW)
endif()

# MSVC runtime library flags are selected by an abstraction, CMake >= 3.15
# This policy still need to be set even with cmake_minimum_required() command above.
if(POLICY CMP0091)
  cmake_policy(SET CMP0091 NEW)
endif()

project(libsamplerate VERSION 0.1.9 LANGUAGES C)

# Configuration

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  set(IS_ROOT_PROJECT ON)
else()
  set(IS_ROOT_PROJECT OFF)
endif()

option(LIBSAMPLERATE_EXAMPLES "Enable to generate examples" ${IS_ROOT_PROJECT})
option(LIBSAMPLERATE_INSTALL "Enable to add install directives" ${IS_ROOT_PROJECT})
option(LIBSAMPLERATE_COMPATIBLE_NAME "Use old dll name on Windows platform" OFF)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

include(TestBigEndian)
include(CheckFunctionExists)
include(CheckIncludeFile)
include(CheckLibraryExists)
include(CheckSymbolExists)
include(ClipMode)
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

if(DEFINED LIBSAMPLERATE_TESTS)
  message(DEPRECATION "LIBSAMPLERATE_TESTS option deprecated, use BUILD_TESTING option instead.")
  set(BUILD_TESTING ${LIBSAMPLERATE_TESTS})
endif()
include(CTest)

add_definitions(-DHAVE_CONFIG_H)
if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

set(OS_IS_WIN32 ${WIN32})

if(NOT WIN32)
  find_library(MATH_LIBRARY m)
  if(MATH_LIBRARY)
    set(LIBM_REQUIRED 1)
    if(LIBM_REQUIRED)
      list(APPEND CMAKE_REQUIRED_LIBRARIES m)
    endif()
  endif()
endif()

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

test_big_endian(CPU_IS_BIG_ENDIAN)
if(CPU_IS_BIG_ENDIAN)
  set(CPU_IS_LITTLE_ENDIAN 0)
else()
  set(CPU_IS_LITTLE_ENDIAN 1)
endif()

# This will set CPU_CLIPS_NEGATIVE and CPU_CLIPS_POSITIVE
clip_mode()

check_function_exists(alarm HAVE_ALARM)
check_function_exists(signal HAVE_SIGNAL)

check_include_file(stdbool.h HAVE_STDBOOL_H)
check_include_file(sys/times.h HAVE_SYS_TIMES_H)
check_include_file(unistd.h HAVE_UNISTD_H)

check_symbol_exists(SIGALRM signal.h HAVE_SIGALRM)

find_package(ALSA)
set(HAVE_ALSA ${ALSA_FOUND})
# ALSA::ALSA target is exported since CMake >= 3.12, create it for
# old CMake versions
if(ALSA_FOUND)
  if(NOT TARGET ALSA::ALSA)
    add_library(ALSA::ALSA UNKNOWN IMPORTED)
    set_target_properties(ALSA::ALSA PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${ALSA_INCLUDE_DIRS}"
    IMPORTED_LOCATION "${ALSA_LIBRARIES}")
  endif()
endif()

find_package(SndFile)
set(HAVE_SNDFILE ${SndFile_FOUND})

find_package(FFTW3)
set(HAVE_FFTW3 ${FFTW3_FOUND})

configure_file(config.h.in config.h)

# samplerate library target

add_library(samplerate
  include/samplerate.h
  src/common.h
  src/fastest_coeffs.h
  src/high_qual_coeffs.h
  src/mid_qual_coeffs.h
  src/samplerate.c
  src/src_config.h
  src/src_linear.c
  src/src_sinc.c
  src/src_zoh.c
  $<$<AND:$<BOOL:${WIN32}>,$<BOOL:${BUILD_SHARED_LIBS}>>:Win32/libsamplerate-0.def>)

# ALIAS to use if libsamplerate is included from other project with add_subdirectory()
add_library(SampleRate::samplerate ALIAS samplerate)

# CMake generates wrong DLL names for MinGW and Cygwin, fix it
if(BUILD_SHARED_LIBS AND WIN32)
  if(LIBSAMPLERATE_COMPATIBLE_NAME)
    if(MSVC)
      set_target_properties(samplerate PROPERTIES OUTPUT_NAME "libsamplerate-${libsamplerate_VERSION_MAJOR}")
    else()
      set_target_properties(samplerate PROPERTIES OUTPUT_NAME "samplerate-${libsamplerate_VERSION_MAJOR}")
  endif()
  else()
    if(MINGW OR CYGWIN)
      set_target_properties(samplerate PROPERTIES RUNTIME_OUTPUT_NAME "samplerate-${libsamplerate_VERSION_MAJOR}")
    endif()
  endif()
endif()

target_include_directories(samplerate
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR})

if(LIBM_REQUIRED)
    target_link_libraries(samplerate PRIVATE m)
endif()

# Set public header
set_property(TARGET samplerate PROPERTY PUBLIC_HEADER ${PROJECT_SOURCE_DIR}/include/samplerate.h)

if(BUILD_SHARED_LIBS)
  # Set ABI version. This is critical for Unix-like OSes
  set_target_properties(samplerate PROPERTIES
    VERSION ${libsamplerate_VERSION}
    SOVERSION ${libsamplerate_VERSION_MAJOR})

    # Use Version_script to export ABI set
    if(UNIX AND (NOT APPLE))
      if((CMAKE_C_COMPILER_ID STREQUAL "GNU") OR
         (CMAKE_C_COMPILER_ID STREQUAL "Clang") OR
         (CMAKE_C_COMPILER_ID STREQUAL "Intel"))

        set(PACKAGE ${PROJECT_NAME})
        configure_file(src/Version_script.in src/Version_script)
        unset(PACKAGE)

        if(CMAKE_VERSION VERSION_LESS 3.13)
          # This works 
          set_property(TARGET samplerate APPEND_STRING PROPERTY
            LINK_FLAGS "-Wl,--version-script,${PROJECT_BINARY_DIR}/src/Version_script")
        else()
          # This works even better, e.g. for Clang it uses `-Xlinker` option,
          # but requires CMake >= 3.13.
          target_link_options(samplerate
            PRIVATE
              "LINKER:--version-script,${PROJECT_BINARY_DIR}/src/Version_script")
        endif()

      endif()
    endif()
endif()

add_library(samplerate_config INTERFACE)
# To allow src_config.h and config.h to be included
target_include_directories(samplerate_config INTERFACE src ${PROJECT_BINARY_DIR})
add_library(samplerate::config ALIAS samplerate_config)

# Tests

# BUILD_TESTING is declared by CTest module and is ON by default
if(BUILD_TESTING)

  add_executable(misc_test tests/misc_test.c tests/util.c tests/util.h)
  target_link_libraries(misc_test PRIVATE samplerate samplerate::config $<$<BOOL:${LIBM_REQUIRED}>:m>)
  add_test(NAME misc_test COMMAND misc_test)

  add_executable(termination_test tests/termination_test.c tests/util.c tests/util.h)
  target_link_libraries(termination_test PRIVATE samplerate samplerate::config $<$<BOOL:${LIBM_REQUIRED}>:m>)
  add_test(NAME termination_test COMMAND termination_test)

  add_executable(callback_hang_test tests/callback_hang_test.c tests/util.c tests/util.h)
  target_include_directories(callback_hang_test
    PRIVATE
      $<$<NOT:$<BOOL:${HAVE_UNISTD_H}>>:${PROJECT_SOURCE_DIR}/Win32>)
  target_link_libraries(callback_hang_test PRIVATE samplerate samplerate::config $<$<BOOL:${LIBM_REQUIRED}>:m>)
  add_test(NAME callback_hang_test COMMAND callback_hang_test)

  add_executable(downsample_test tests/downsample_test.c tests/util.c tests/util.h)
  target_link_libraries(downsample_test PRIVATE samplerate samplerate::config $<$<BOOL:${LIBM_REQUIRED}>:m>)
  add_test(NAME downsample_test COMMAND downsample_test)

  add_executable(simple_test tests/simple_test.c tests/util.c tests/util.h)
  target_link_libraries(simple_test PRIVATE samplerate samplerate::config $<$<BOOL:${LIBM_REQUIRED}>:m>)
  add_test(NAME simple_test COMMAND simple_test)

  add_executable(callback_test tests/callback_test.c tests/util.c tests/util.h)
  target_include_directories(callback_test
    PRIVATE
      $<$<NOT:$<BOOL:${HAVE_UNISTD_H}>>:${PROJECT_SOURCE_DIR}/Win32>)
  target_link_libraries(callback_test PRIVATE samplerate samplerate::config $<$<BOOL:${LIBM_REQUIRED}>:m>)
  add_test(NAME callback_test COMMAND callback_test)

  add_executable(reset_test tests/reset_test.c tests/util.c tests/util.h)
  target_link_libraries(reset_test PRIVATE samplerate samplerate::config $<$<BOOL:${LIBM_REQUIRED}>:m>)
  add_test(NAME reset_test COMMAND reset_test)

  add_executable(clone_test tests/clone_test.c tests/util.c tests/util.h)
  target_link_libraries(clone_test PRIVATE samplerate samplerate::config $<$<BOOL:${LIBM_REQUIRED}>:m>)
  add_test(NAME clone_test COMMAND clone_test)

  add_executable(nullptr_test tests/nullptr_test.c tests/util.c tests/util.h)
  target_link_libraries(nullptr_test PRIVATE samplerate samplerate::config $<$<BOOL:${LIBM_REQUIRED}>:m>)
  add_test(NAME nullptr_test COMMAND nullptr_test)

  add_executable(multi_channel_test tests/multi_channel_test.c tests/calc_snr.c tests/util.c tests/util.h)
  target_link_libraries(multi_channel_test
    PRIVATE
      samplerate samplerate::config
      $<$<BOOL:${FFTW3_FOUND}>:FFTW3::fftw3>
      $<$<BOOL:${LIBM_REQUIRED}>:m>)
  add_test(NAME multi_channel_test COMMAND multi_channel_test)

  add_executable(varispeed_test tests/varispeed_test.c tests/calc_snr.c tests/util.c tests/util.h)
  target_link_libraries(varispeed_test
    PRIVATE samplerate samplerate::config
      $<$<BOOL:${FFTW3_FOUND}>:FFTW3::fftw3>
      $<$<BOOL:${LIBM_REQUIRED}>:m>)

  add_test(NAME varispeed_test COMMAND varispeed_test tests/util.c tests/util.h)

  add_executable(float_short_test tests/float_short_test.c)
  target_link_libraries(float_short_test PRIVATE samplerate samplerate::config $<$<BOOL:${LIBM_REQUIRED}>:m>)
  add_test(NAME float_short_test COMMAND float_short_test)

  add_executable(snr_bw_test tests/snr_bw_test.c tests/calc_snr.c tests/util.c tests/util.h)
  target_link_libraries(snr_bw_test
    PRIVATE
      samplerate samplerate::config
      $<$<BOOL:${FFTW3_FOUND}>:FFTW3::fftw3>
      $<$<BOOL:${LIBM_REQUIRED}>:m>)
  add_test(NAME snr_bw_test COMMAND snr_bw_test tests/util.c tests/util.h)

  add_executable(throughput_test tests/throughput_test.c tests/util.c tests/util.h)
  target_include_directories(throughput_test
    PRIVATE
      $<$<NOT:$<BOOL:${HAVE_UNISTD_H}>>:${PROJECT_SOURCE_DIR}/Win32>)
  target_link_libraries(throughput_test PRIVATE samplerate samplerate::config $<$<BOOL:${LIBM_REQUIRED}>:m>)

  add_executable(multichan_throughput_test tests/multichan_throughput_test.c tests/util.c tests/util.h)
  target_include_directories(multichan_throughput_test
    PRIVATE
      $<$<NOT:$<BOOL:${HAVE_UNISTD_H}>>:${PROJECT_SOURCE_DIR}/Win32>)
  target_link_libraries(multichan_throughput_test PRIVATE samplerate samplerate::config $<$<BOOL:${LIBM_REQUIRED}>:m>)

  add_executable(src-evaluate tests/src-evaluate.c tests/util.c tests/util.h tests/calc_snr.c)
  target_include_directories(src-evaluate
    PRIVATE
      $<$<NOT:$<BOOL:${HAVE_UNISTD_H}>>:${PROJECT_SOURCE_DIR}/Win32>)
  target_link_libraries(src-evaluate
    PRIVATE
      samplerate samplerate::config
      $<$<BOOL:${FFTW3_FOUND}>:FFTW3::fftw3>
      $<$<BOOL:${SndFile_FOUND}>:SndFile::sndfile>
      $<$<BOOL:${LIBM_REQUIRED}>:m>)

endif()

# Examples

if(LIBSAMPLERATE_EXAMPLES)

  add_executable(timewarp-file examples/timewarp-file.c)
  target_include_directories(timewarp-file
    PRIVATE
      $<$<NOT:$<BOOL:${HAVE_UNISTD_H}>>:${PROJECT_SOURCE_DIR}/Win32>)
  target_link_libraries(timewarp-file
    PRIVATE
      samplerate samplerate::config
      $<$<BOOL:${SndFile_FOUND}>:SndFile::sndfile>
      $<$<BOOL:${LIBM_REQUIRED}>:m>)

  add_executable(varispeed-play examples/varispeed-play.c examples/audio_out.c examples/audio_out.h)
  target_include_directories(varispeed-play
    PRIVATE
      $<$<NOT:$<BOOL:${HAVE_UNISTD_H}>>:${PROJECT_SOURCE_DIR}/Win32>)
  target_link_libraries(varispeed-play
    PRIVATE
      samplerate samplerate::config
      $<$<BOOL:${SndFile_FOUND}>:SndFile::sndfile>
      $<$<BOOL:${LIBM_REQUIRED}>:m>)
  if(WIN32)
    target_link_libraries(varispeed-play PRIVATE winmm)
  elseif(APPLE)
    target_link_libraries(varispeed-play PRIVATE "-framework CoreAudio")
  elseif(ALSA_FOUND)
    target_link_libraries(varispeed-play PRIVATE ALSA::ALSA)
  endif()

endif() 

# Istallation

if(LIBSAMPLERATE_INSTALL)

  # pkg-config module

  set(prefix ${CMAKE_INSTALL_PREFIX})
  set(exec_prefix "\${prefix}")
  set(includedir "\${prefix}/${CMAKE_INSTALL_INCLUDEDIR}")
  set(libdir "\${exec_prefix}/${CMAKE_INSTALL_LIBDIR}")
  set(VERSION "${PROJECT_VERSION}")
  if(LIBM_REQUIRED)
    set(LIBS "-lm")
  endif()
  configure_file(samplerate.pc.in samplerate.pc @ONLY)
  install(FILES ${CMAKE_BINARY_DIR}/samplerate.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

  # samplerate library

  install(TARGETS samplerate EXPORT SampleRateTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

  # samplerate config-file package

  if(WIN32 AND (NOT MINGW) AND (NOT CYGWIN))
    set(CMAKE_INSTALL_PACKAGEDIR cmake) 
  else()
    set(CMAKE_INSTALL_PACKAGEDIR ${CMAKE_INSTALL_LIBDIR}/cmake/SampleRate)
  endif()

  install(EXPORT SampleRateTargets
    NAMESPACE SampleRate::
    DESTINATION ${CMAKE_INSTALL_PACKAGEDIR})

  configure_package_config_file(cmake/SampleRateConfig.cmake.in SampleRateConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_PACKAGEDIR})

  write_basic_package_version_file(SampleRateConfigVersion.cmake COMPATIBILITY SameMajorVersion)
    install(
      FILES
        ${PROJECT_BINARY_DIR}/SampleRateConfig.cmake
        ${PROJECT_BINARY_DIR}/SampleRateConfigVersion.cmake
      DESTINATION
        ${CMAKE_INSTALL_PACKAGEDIR})

  # Documentation

  install(DIRECTORY docs/ DESTINATION ${CMAKE_INSTALL_DOCDIR})
endif()

# Packaging support

include(CPack)
