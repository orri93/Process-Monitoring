# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("Process Monitoring"
  VERSION 0.1.0
  DESCRIPTION "Simple Process Monitoring Tool"
  LANGUAGES C)

option(BUILD_TESTS "Build tests" ON)
option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)
option(CLANG_TIDY_FIX_ERRORS
  "Perform fixes with Clang-Tidy even if compilation errors were found" OFF)
option(CLANG_TIDY_FIX "Perform fixes with Clang-Tidy" OFF)
option(CLANG_TIDY "Perform Clang-Tidy check" OFF)
if (MSVC)
  option(GENERATE_PDB_FOR_RELEASE "Generate PDB files for Release" ON)
endif ()

set(BUILD_NUMBER "0" CACHE STRING "The build number")

set(TIDY_ARGUMENTS "-checks=*,-clang-analyzer-alpha.* "
  CACHE STRING "Arguments for Clang-Tidy check")
set(TIDY_FIX_ARGUMENTS "-list-checks=*,-clang-analyzer-alpha.* "
  CACHE STRING "Arguments for Clang-Tidy fix")
set(TIDY_FIX_ERRORS_ARGUMENTS "-checks=*,-clang-analyzer-alpha.* "
  CACHE STRING "Arguments for Clang-Tidy fix errors")

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/include/pm/version.h.in"
  "include/pm/version.h" @ONLY)

list(APPEND pm_include
  "${CMAKE_CURRENT_BINARY_DIR}/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/include")

set(CMAKE_PLATFORM_INDEPENDENT_CODE ON)

string(TIMESTAMP PM_CURRENT_YEAR "%Y" UTC)
set(PM_COMPANYNAME "Geirmundur Orri Sigurdsson")
set(PM_LEGALCOPYRIGHT "Copyright (C) ${PM_CURRENT_YEAR} ${PM_COMPANYNAME}")

set(pm_etc_path "${CMAKE_CURRENT_SOURCE_DIR}/etc")

set(pm_library_target libpm)

list(APPEND pm_library_public_headers
  "${CMAKE_CURRENT_BINARY_DIR}/include/pm/version.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/pm/pm.h")

if(CLANG_TIDY)
  find_program(CLANG_TIDY_EXE
    NAMES "clang-tidy"
    DOC "Path to clang-tidy executable")
  if(CLANG_TIDY_EXE)
    if(CLANG_TIDY_FIX_ERRORS)
      set(CMAKE_CXX_CLANG_TIDY
        "${CLANG_TIDY_EXE}" "${TIDY_FIX_ERRORS_ARGUMENTS}-fix-errors")
      message(STATUS "Using clang-tidy with fix")
      message(STATUS "  ${CLANG_TIDY_EXE} ${TIDY_FIX_ERRORS_ARGUMENTS}-fix-errors")
    elseif(CLANG_TIDY_FIX)
      set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}" "${TIDY_FIX_ARGUMENTS}-fix")
      message(STATUS "Using clang-tidy with fix")
      message(STATUS "  ${CLANG_TIDY_EXE} ${TIDY_FIX_ARGUMENTS}-fix")
    else()
      set(CMAKE_CXX_CLANG_TIDY
        "${CLANG_TIDY_EXE}" "${TIDY_ARGUMENTS}")
      message(STATUS "Using clang-tidy")
      message(STATUS "  ${CLANG_TIDY_EXE} ${TIDY_ARGUMENTS}")
    endif()
  endif()
endif()

# Include sub-projects.
add_subdirectory(extern)

list(APPEND pm_containers_include
  "${CMAKE_CURRENT_SOURCE_DIR}/extern/containers"
  "${CMAKE_CURRENT_SOURCE_DIR}/extern/containers/src/include")
list(APPEND pm_optparse_include
  "${CMAKE_CURRENT_SOURCE_DIR}/extern/optparse")
list(APPEND pm_libyaml_include
  "${CMAKE_CURRENT_SOURCE_DIR}/extern/libyaml/include")

add_subdirectory(src)

if (NOV_BUILD_TESTS)
  enable_testing()
  if (MSVC)
    #if (NOV_FORCE_SHARED_CRT)
      set(gtest_force_shared_crt ON CACHE BOOL "Use Shared CRT")
    #endif ()
  endif (MSVC)
  list(APPEND pm_test_include
    "${CMAKE_CURRENT_SOURCE_DIR}/extern/googletest/googletest/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/extern/googletest/googlemock/include")
  list(APPEND pm_test_libraries
    gtest
    gmock
    gmock_main)
  add_subdirectory(tests)
endif ()

message(STATUS "")
message(STATUS "NOV Wellbore Connect Modules ${PROJECT_VERSION} BUILD SUMMARY")
message(STATUS "  Install prefix            : ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  CMAKE_GENERATOR           : ${CMAKE_GENERATOR}")
message(STATUS "  CMAKE_SOURCE_DIR          : ${CMAKE_SOURCE_DIR}")
message(STATUS "  CMAKE_CURRENT_SOURCE_DIR  : ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "  BUILD_SHARED_LIBS         : ${BUILD_SHARED_LIBS}")
if (MSVC_VERSION)
message(STATUS "  MSVC Version              : ${MSVC_VERSION}")
endif (MSVC_VERSION)
message(STATUS "  C Compiler ID             : ${CMAKE_C_COMPILER_ID}")
message(STATUS "  C Compiler Version        : ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "  C Compiler flags          : ${CMAKE_C_FLAGS}")
if (BUILD_TESTS)
message(STATUS "Building Tests")
if (MSVC)
message(STATUS "  GTest Force Shared CRT    : ${gtest_force_shared_crt}")
endif ()
endif ()
