#.rst:
# FindCURL
# --------
#
# Find curl
#
# Find the native CURL headers and libraries.
#
# ::
#
#   CURL_INCLUDE_DIRS   - where to find curl/curl.h, etc.
#   CURL_LIBRARIES      - List of libraries when using curl.
#   CURL_FOUND          - True if curl found.
#   CURL_VERSION_STRING - the version of curl found (since CMake 2.8.8)

#=============================================================================
# Copyright 2006-2009 Kitware, Inc.
# Copyright 2012 Rolf Eike Beer <eike@sf-mail.de>
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

# Look for the header file in Homebrew directory
find_path(CURL_INCLUDE_DIR NAMES curl.h
    PATHS
    /usr/local/opt/curl/include/curl  # Homebrew
    ~/.linuxbrew/include/curl  # Linuxbrew
    /usr/include/curl  # Linux
    NO_DEFAULT_PATH )

find_path(CURL_INCLUDE_DIR NAMES curl/curl.h)
mark_as_advanced(CURL_INCLUDE_DIR)

# if(SBOL_BUILD_SHARED)
# if(CURL_LIBRARY MATCHES "(.*).(a|lib)")
# unset(CURL_LIBRARY CACHE)
# endif()
# else()
# if(CURL_LIBRARY MATCHES "(.*).(so|dylib|dll)")
# unset(CURL_LIBRARY CACHE)
# endif()
# endif()

# Look for the library (sorted from most current/relevant entry to least).
find_library(CURL_LIBRARY NAMES
    curl
  # Windows MSVC prebuilts:
    curllib
    libcurl_imp
    curllib_static
  # Windows older "Win32 - MSVC" prebuilts (libcurl.lib, e.g. libcurl-7.15.5-win32-msvc.zip):
    libcurl

    PATHS
    /usr/local/opt/curl/lib # Homebrew path
    ~/.linuxbrew/lib
    /usr/lib/i386-linux-gnu  #Linux 32
    /usr/lib/x86_64-linux-gnu #Linux 64
    NO_DEFAULT_PATH )

find_library(CURL_LIBRARY NAMES
    curl
  # Windows MSVC prebuilts:
    curllib
    libcurl_imp
    curllib_static
  # Windows older "Win32 - MSVC" prebuilts (libcurl.lib, e.g. libcurl-7.15.5-win32-msvc.zip):
    libcurl )
mark_as_advanced(CURL_LIBRARY)

if(CURL_INCLUDE_DIR)
  foreach(_curl_version_header curlver.h curl.h)
    if(EXISTS "${CURL_INCLUDE_DIR}/curl/${_curl_version_header}")
      file(STRINGS "${CURL_INCLUDE_DIR}/curl/${_curl_version_header}" curl_version_str REGEX "^#define[\t ]+LIBCURL_VERSION[\t ]+\".*\"")

      string(REGEX REPLACE "^#define[\t ]+LIBCURL_VERSION[\t ]+\"([^\"]*)\".*" "\\1" CURL_VERSION_STRING "${curl_version_str}")
      unset(curl_version_str)
      break()
    endif()
  endforeach()
endif()

# handle the QUIETLY and REQUIRED arguments and set CURL_FOUND to TRUE if
# all listed variables are TRUE
include(${CMAKE_ROOT}/Modules/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(CURL
                                  REQUIRED_VARS CURL_LIBRARY CURL_INCLUDE_DIR
                                  VERSION_VAR CURL_VERSION_STRING)

if(CURL_FOUND)
  set(CURL_LIBRARIES ${CURL_LIBRARY})
  set(CURL_INCLUDE_DIRS ${CURL_INCLUDE_DIR})
endif()
