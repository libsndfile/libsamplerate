cmake_minimum_required(VERSION 3.1)
project(LibSamplerate LANGUAGES C)

set(SRC 
src/samplerate.c
src/src_linear.c
src/src_sinc.c
src/src_zoh.c)

add_library(samplerate ${SRC})
target_include_directories(samplerate PUBLIC src)
if(WIN32)
target_include_directories(samplerate PUBLIC Win32)
endif()
