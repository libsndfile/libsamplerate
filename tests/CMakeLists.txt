if(WIN32)
    set(OS_IS_WIN32 TRUE)
else()
    set(OS_IS_WIN32 FALSE)
endif()

find_package(Sndfile)
set(HAVE_SNDFILE ${SNDFILE_FOUND})

find_package(FFTW)
set(HAVE_FFTW3 ${FFTW_FOUND})

include(CheckSymbolExists)
include(CheckIncludeFiles)

check_symbol_exists(signal signal.h HAVE_SIGNAL)
check_symbol_exists(SIGALRM signal.h HAVE_SIGALRM)
check_include_files(sys/times.h HAVE_SYS_TIMES_H)

configure_file(config.h.in config.h)

add_library(samplerate_test_utils STATIC
    util.c
    calc_snr.c
)

target_include_directories(samplerate_test_utils PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

if(WIN32)
    # unistd.h dummy
    target_include_directories(samplerate_test_utils PUBLIC ${PROJECT_SOURCE_DIR}/Win32)
endif()

if(FFTW_FOUND)
    target_link_libraries(samplerate_test_utils PUBLIC ${FFTW_LIBRARY})
    target_include_directories(samplerate_test_utils PUBLIC ${FFTW_INCLUDE_DIR})
endif()

if(SNDFILE_FOUND)
    target_link_libraries(samplerate_test_utils PUBLIC ${SNDFILE_LIBRARY})
    target_include_directories(samplerate_test_utils PUBLIC ${SNDFILE_INCLUDE_DIR})
endif()

file(GLOB TEST_SRCS *_test.c)

foreach(testSrc ${TEST_SRCS})
    get_filename_component(testName ${testSrc} NAME_WE)
    add_executable(${testName} ${testSrc})
    target_link_libraries(${testName} PUBLIC samplerate samplerate_test_utils)
    add_test(NAME ${testName} COMMAND ${testName})
endforeach()
