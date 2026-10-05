cmake_minimum_required(VERSION 3.16)

# Check each path independently and retain prefix-relative pkg-config variables
# for relative install directories.
foreach(layout relative absolute absolute_include absolute_lib)
  set(build_dir "${BINARY_DIR}/build/${layout}")
  set(install_dir "${BINARY_DIR}/install/${layout}")
  set(include_dir "include/custom")
  set(lib_dir "lib/custom")
  set(expected_include "\${prefix}/${include_dir}")
  set(expected_lib "\${exec_prefix}/${lib_dir}")

  if(layout STREQUAL "absolute" OR layout STREQUAL "absolute_include")
    set(include_dir "${install_dir}/installed-include")
    set(expected_include "${include_dir}")
  endif()
  if(layout STREQUAL "absolute" OR layout STREQUAL "absolute_lib")
    set(lib_dir "${install_dir}/installed-lib")
    set(expected_lib "${lib_dir}")
  endif()

  execute_process(
    COMMAND "${CMAKE_COMMAND}" -S "${SOURCE_DIR}" -B "${build_dir}"
      -G "${GENERATOR}"
      "-DCMAKE_C_COMPILER=${C_COMPILER}"
      "-DCMAKE_INSTALL_PREFIX=${install_dir}/prefix"
      "-DCMAKE_INSTALL_INCLUDEDIR=${include_dir}"
      "-DCMAKE_INSTALL_LIBDIR=${lib_dir}"
      -DBUILD_TESTING=OFF
      -DLIBSAMPLERATE_EXAMPLES=OFF
      -DLIBSAMPLERATE_INSTALL=ON
      -DLIBSAMPLERATE_INSTALL_PKGCONFIG_MODULE=ON
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error)
  if(NOT result EQUAL 0)
    message(FATAL_ERROR "${layout} configure failed: ${output}\n${error}")
  endif()

  file(STRINGS "${build_dir}/src/samplerate.pc" actual_include
    REGEX "^includedir=")
  file(STRINGS "${build_dir}/src/samplerate.pc" actual_lib
    REGEX "^libdir=")
  if(NOT actual_include STREQUAL "includedir=${expected_include}")
    message(FATAL_ERROR "${layout}: unexpected ${actual_include}")
  endif()
  if(NOT actual_lib STREQUAL "libdir=${expected_lib}")
    message(FATAL_ERROR "${layout}: unexpected ${actual_lib}")
  endif()
endforeach()
