file(GLOB TEST_SRCS *_test.c)

add_library(samplerate_test_utils STATIC
    util.c
    calc_snr.c
)

target_link_libraries(samplerate_test_utils PRIVATE samplerate)

if(FFTW_FOUND)
    target_link_libraries(samplerate_test_utils PUBLIC ${FFTW_LIBRARY})
    target_include_directories(samplerate_test_utils PUBLIC ${FFTW_INCLUDE_DIR})
endif()

foreach(testSrc ${TEST_SRCS})
    get_filename_component(testName ${testSrc} NAME_WE)
    add_executable(${testName} ${testSrc})
    target_link_libraries(${testName} PUBLIC samplerate samplerate_test_utils)
    add_test(NAME ${testName} COMMAND ${testName})
endforeach()
