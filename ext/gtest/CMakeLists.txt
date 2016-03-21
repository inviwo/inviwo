#--------------------------------------------------------------------
# gtest Package
ivw_project(gtest)

########################################################################
# CMake build script for Google Test.
#
# To run the tests for Google Test itself on Linux, use 'make test' or
# ctest.  You can select which tests to run using 'ctest -R regex'.
# For more options, run 'ctest --help'.

# BUILD_SHARED_LIBS is a standard CMake variable, but we declare it here to
# make it prominent in the GUI.
#option(BUILD_SHARED_LIBS "Build shared libraries (DLLs)." OFF)
# for INVIWO  gtest dont need BUILD_SHARED_LIBS, it already exist in globalconfig.cmake  

# When other libraries are using a shared version of runtime libraries,
# Google Test also has to use one.
if(SHARED_LIBS OR IVW_FORCE_SHARED_CRT)
    set(gtest_force_shared_crt ON CACHE BOOL "Use shared (DLL) run-time lib even when Google Test is built as static lib." FORCE)
else()
    set(gtest_force_shared_crt OFF CACHE BOOL "Use shared (DLL) run-time lib even when Google Test is built as static lib." FORCE)
endif()

option(gtest_disable_pthreads "Disable uses of pthreads in gtest." OFF)

# Defines pre_project_set_up_hermetic_build() and set_up_hermetic_build().
include(cmake/hermetic_build.cmake OPTIONAL)

if (COMMAND pre_project_set_up_hermetic_build)
  pre_project_set_up_hermetic_build()
endif()

########################################################################
#
# Project-wide settings

# Name of the project.
#
# CMake files in this project can refer to the root source directory
# as ${gtest_SOURCE_DIR} and to the root binary directory as
# ${gtest_BINARY_DIR}.
# Language "C" is required for find_package(Threads).
project(gtest CXX C)
cmake_minimum_required(VERSION 2.6.2)

if (COMMAND set_up_hermetic_build)
  set_up_hermetic_build()
endif()

if(POLICY CMP0042)
    cmake_policy(SET CMP0042 NEW) # MacOSX rpath policy.
endif()

# Define helper functions and macros used by Google Test.
include(internal_utils.cmake)

config_compiler_and_linker()  # Defined in internal_utils.cmake.

# Where Google Test's .h files can be found.
include_directories(
  ${gtest_SOURCE_DIR}/include
  ${gtest_SOURCE_DIR})

# Where Google Test's libraries can be found.
# link_directories(${gtest_BINARY_DIR}/src)

########################################################################
#
# Defines the gtest libraries.  User tests should link
# with one of them.

# Google Test libraries.  We build them using more strict warnings than what
# are used for other targets, to ensure that gtest can be compiled by a user
# aggressive about warnings.
if(BUILD_SHARED_LIBS)
    cxx_shared_library(gtest "${cxx_strict}" src/gtest-all.cc)
else()
    cxx_library(gtest "${cxx_strict}" src/gtest-all.cc)
endif()

#--------------------------------------------------------------------
# Creates VS folder structure
ivw_folder(gtest ext)

#--------------------------------------------------------------------
# Make package (for other projects to find)
ivw_set_cpack_name(gtest)
ivw_make_package(gtest gtest)
