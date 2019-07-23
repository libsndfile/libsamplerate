
add_library(samplerate_example_audio_out STATIC audio_out.c)
target_link_libraries(samplerate_example_audio_out PRIVATE samplerate)

if(ALSA_FOUND)
    target_link_libraries(samplerate_example_audio_out PUBLIC ${ALSA_LIBRARY})
    target_include_directories(samplerate_example_audio_out PUBLIC ${ALSA_INCLUDE_DIR})
endif()
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

set(EXAMPLE_SRCS
    timewarp-file.c
    varispeed-play.c)

foreach(exampleSrc ${EXAMPLE_SRCS})
    get_filename_component(exampleName ${exampleSrc} NAME_WE)
    add_executable(${exampleName} ${exampleSrc})
    target_link_libraries(${exampleName} PUBLIC samplerate samplerate_example_audio_out)
endforeach()
