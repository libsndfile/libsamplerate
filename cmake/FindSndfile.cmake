# Variables defined:
#  SNDFILE_FOUND
#  SNDFILE_INCLUDE_DIR
#  SNDFILE_LIBRARY
#
# Environment variables used:
#  SNDFILE_ROOT

find_path(SNDFILE_INCLUDE_DIR sndfile.h
  HINTS $ENV{SNDFILE_ROOT}/include)

find_library(SNDFILE_LIBRARY NAMES sndfile
    HINTS $ENV{SNDFILE_ROOT}/lib)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SNDFILE DEFAULT_MSG SNDFILE_LIBRARY SNDFILE_INCLUDE_DIR)

mark_as_advanced(SNDFILE_LIBRARY SNDFILE_INCLUDE_DIR)
