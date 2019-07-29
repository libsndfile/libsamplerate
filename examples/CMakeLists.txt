add_library(samplerate_example_audio_out STATIC audio_out.c)
target_link_libraries(samplerate_example_audio_out PRIVATE samplerate)

if(WIN32)
    # unistd.h dummy
    target_include_directories(samplerate_example_audio_out PUBLIC ${PROJECT_SOURCE_DIR}/Win32)
endif()

find_package(ALSA)
set(HAVE_ALSA ${ALSA_FOUND})
if(ALSA_FOUND)
    target_link_libraries(samplerate_example_audio_out PUBLIC ${ALSA_LIBRARY})
    target_include_directories(samplerate_example_audio_out PUBLIC ${ALSA_INCLUDE_DIR})
endif()

find_package(Sndfile)
set(HAVE_SNDFILE ${SNDFILE_FOUND})
if(SNDFILE_FOUND)
    target_link_libraries(samplerate_example_audio_out PUBLIC ${SNDFILE_LIBRARY})
    target_include_directories(samplerate_example_audio_out PUBLIC ${SNDFILE_INCLUDE_DIR})
endif()

if(WIN32)
    target_link_libraries(samplerate_example_audio_out PUBLIC winmm)
endif()
if (APPLE)
    target_link_libraries (samplerate_example_audio_out PUBLIC  "-framework CoreAudio")
endif()

include(TestBigEndian)
test_big_endian(CPU_IS_BIG_ENDIAN)

configure_file(config.h.in config.h)
target_include_directories(samplerate_example_audio_out PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

set(EXAMPLE_SRCS
    timewarp-file.c
    varispeed-play.c)

foreach(exampleSrc ${EXAMPLE_SRCS})
    get_filename_component(exampleName ${exampleSrc} NAME_WE)
    add_executable(${exampleName} ${exampleSrc})
    target_link_libraries(${exampleName} PUBLIC samplerate samplerate_example_audio_out)
endforeach()
