 #################################################################################
 #
 # Inviwo - Interactive Visualization Workshop
 #
 # Copyright (c) 2018 Inviwo Foundation
 # All rights reserved.
 #
 # Redistribution and use in source and binary forms, with or without
 # modification, are permitted provided that the following conditions are met:
 #
 # 1. Redistributions of source code must retain the above copyright notice, this
 # list of conditions and the following disclaimer.
 # 2. Redistributions in binary form must reproduce the above copyright notice,
 # this list of conditions and the following disclaimer in the documentation
 # and/or other materials provided with the distribution.
 #
 # THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 # ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 # WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 # DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 # ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 # (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 # LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 # ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 # (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 # SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 #
 #################################################################################

ivw_append_install_list(DIRECTORY)

# GTest
find_package(GTest CONFIG REQUIRED)
ivw_vcpkg_install(GTest          MODULE Core EXT)

# Img formats
find_package(ZLIB REQUIRED)
ivw_vcpkg_install(zlib          MODULE Core EXT)

find_package(JPEG REQUIRED)
ivw_vcpkg_install(libjpeg-turbo MODULE Core EXT)

find_package(PNG  REQUIRED)
ivw_vcpkg_install(libpng        MODULE Core EXT)

# TIFF
find_package(TIFF REQUIRED)
ivw_vcpkg_install(tiff          MODULE Core EXT)

# GLM
find_package(glm CONFIG REQUIRED)
ivw_vcpkg_install(glm MODULE Core EXT)

# Flags
message(STATUS "add subdirectory: flags")
add_subdirectory(flags) # We have local modifications
ivw_register_license_file(NAME "Flags" MODULE Core EXT
    URL "http://grisumbras.github.io/enum-flags/"
    TYPE "The MIT License"
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/flags/LICENSE
)

# ticpp
message(STATUS "add subdirectory: ticpp")
add_subdirectory(ticpp) # We have local modifications
ivw_register_license_file(NAME "TiCPP" TARGET ticpp MODULE Core EXT
    URL https://github.com/wxFormBuilder/ticpp
    TYPE "The MIT License"
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/ticpp/LICENSE
)

# benchmark
option(IVW_TEST_BENCHMARKS "Generate benchmark targets" OFF)
if(IVW_TEST_BENCHMARKS)
     # don't install, used in build only
    find_package(benchmark CONFIG REQUIRED)
endif()

# Warn
find_package(warn CONFIG REQUIRED)
ivw_vcpkg_install(warn MODULE Core EXT)

# tclap
# tclap 1.2.5 in vcpkg as of 20230303 does not use CMake
# but 1.4.0 does, when that is in vcpkg we should refactor this
find_path(TCLAP_INCLUDE_DIRS "tclap/Arg.h")
ivw_vcpkg_install(TCLAP MODULE Core EXT)
add_library(tclap INTERFACE)
ivw_register_package(tclap tclap)
add_library(inviwo::tclap ALIAS tclap)
target_sources(tclap
    PUBLIC
    FILE_SET HEADERS
    TYPE HEADERS
    BASE_DIRS ${TCLAP_INCLUDE_DIRS}
    FILES
        ${TCLAP_INCLUDE_DIRS}/tclap/Arg.h
        ${TCLAP_INCLUDE_DIRS}/tclap/ArgException.h
        ${TCLAP_INCLUDE_DIRS}/tclap/ArgTraits.h
        ${TCLAP_INCLUDE_DIRS}/tclap/CmdLine.h
        ${TCLAP_INCLUDE_DIRS}/tclap/CmdLineInterface.h
        ${TCLAP_INCLUDE_DIRS}/tclap/CmdLineOutput.h
        ${TCLAP_INCLUDE_DIRS}/tclap/Constraint.h
        ${TCLAP_INCLUDE_DIRS}/tclap/DocBookOutput.h
        ${TCLAP_INCLUDE_DIRS}/tclap/HelpVisitor.h
        ${TCLAP_INCLUDE_DIRS}/tclap/IgnoreRestVisitor.h
        ${TCLAP_INCLUDE_DIRS}/tclap/MultiArg.h
        ${TCLAP_INCLUDE_DIRS}/tclap/MultiSwitchArg.h
        ${TCLAP_INCLUDE_DIRS}/tclap/OptionalUnlabeledTracker.h
        ${TCLAP_INCLUDE_DIRS}/tclap/sstream.h
        ${TCLAP_INCLUDE_DIRS}/tclap/StandardTraits.h
        ${TCLAP_INCLUDE_DIRS}/tclap/StdOutput.h
        ${TCLAP_INCLUDE_DIRS}/tclap/SwitchArg.h
        ${TCLAP_INCLUDE_DIRS}/tclap/UnlabeledMultiArg.h
        ${TCLAP_INCLUDE_DIRS}/tclap/UnlabeledValueArg.h
        ${TCLAP_INCLUDE_DIRS}/tclap/ValueArg.h
        ${TCLAP_INCLUDE_DIRS}/tclap/ValuesConstraint.h
        ${TCLAP_INCLUDE_DIRS}/tclap/VersionVisitor.h
        ${TCLAP_INCLUDE_DIRS}/tclap/Visitor.h
        ${TCLAP_INCLUDE_DIRS}/tclap/XorHandler.h
        ${TCLAP_INCLUDE_DIRS}/tclap/ZshCompletionOutput.h
)
ivw_folder(tclap vcpkg)
ivw_install_helper(TARGET tclap NAMESPACE inviwo DESTINATION inviwo)

# utfcpp
find_package(utf8cpp CONFIG REQUIRED)
ivw_vcpkg_install(utfcpp MODULE Core EXT)

# nlohmann json
find_package(nlohmann_json CONFIG REQUIRED)
ivw_vcpkg_install(nlohmann-json MODULE Core EXT)

# Fmt
find_package(fmt CONFIG REQUIRED)
ivw_vcpkg_install(fmt MODULE Core EXT)

# sml
find_package(sml CONFIG REQUIRED)
ivw_vcpkg_install(bext-sml MODULE Core EXT)

# CRoaring
find_package(roaring CONFIG REQUIRED)
ivw_vcpkg_install(roaring MODULE Core EXT)

# Units
find_package(llnl-units CONFIG REQUIRED)
ivw_vcpkg_install(llnl-units MODULE Core EXT)

# md4c
find_package(md4c CONFIG REQUIRED)
ivw_vcpkg_install(md4c MODULE Core EXT)

# FastFloat
find_package(FastFloat CONFIG REQUIRED)
ivw_vcpkg_install(fast-float MODULE Core EXT)

# curlcpp
find_package(curlcpp CONFIG REQUIRED)
ivw_vcpkg_install(curlcpp MODULE Core EXT)

if(IVW_ENABLE_PYTHON)
    find_package(pybind11 CONFIG REQUIRED)
    ivw_vcpkg_install(pybind11 MODULE Core EXT)
endif()

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/bzip2-config.cmake [=[
if(TARGET BZip2::BZip2)
    return()
endif()

# Compute the installation prefix relative to this file.
get_filename_component(_IMPORT_PREFIX "${CMAKE_CURRENT_LIST_FILE}" PATH)
get_filename_component(_IMPORT_PREFIX "${_IMPORT_PREFIX}" PATH)
get_filename_component(_IMPORT_PREFIX "${_IMPORT_PREFIX}" PATH)
if(_IMPORT_PREFIX STREQUAL "/")
  set(_IMPORT_PREFIX "")
endif()

add_library(BZip2::BZip2 SHARED IMPORTED)

set_target_properties(BZip2::BZip2 PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${_IMPORT_PREFIX}/include
)

set_target_properties(BZip2::BZip2 PROPERTIES
  IMPORTED_CONFIGURATIONS "DEBUG;RELEASE"
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/debug/lib/bz2d.lib"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/debug/bin/bz2d.dll"
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/bz2.lib"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/bz2.dll"
)

set(BZip2_FOUND TRUE)
]=])

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/bzip2-config.cmake
    DESTINATION ${IVW_SHARE_INSTALL_DIR}/bzip2
    COMPONENT Application
)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/zlib-config.cmake [=[
if(TARGET ZLIB::ZLIB)
    return()
endif()

# Compute the installation prefix relative to this file.
get_filename_component(_IMPORT_PREFIX "${CMAKE_CURRENT_LIST_FILE}" PATH)
get_filename_component(_IMPORT_PREFIX "${_IMPORT_PREFIX}" PATH)
get_filename_component(_IMPORT_PREFIX "${_IMPORT_PREFIX}" PATH)
if(_IMPORT_PREFIX STREQUAL "/")
  set(_IMPORT_PREFIX "")
endif()

add_library(ZLIB::ZLIB SHARED IMPORTED)

set_target_properties(ZLIB::ZLIB PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${_IMPORT_PREFIX}/include
)

set_target_properties(ZLIB::ZLIB PROPERTIES
  IMPORTED_CONFIGURATIONS "DEBUG;RELEASE"
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/debug/lib/zlibd.lib"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/debug/bin/zlibd.dll"
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/zlib.lib"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/zlib.dll"
)

set(ZLIB_INCLUDE_DIR ${_IMPORT_PREFIX}/include)
set(ZLIB_LIBRARY_DEBUG "${_IMPORT_PREFIX}/debug/lib/zlibd.lib")
set(ZLIB_LIBRARY_RELEASE "${_IMPORT_PREFIX}/bin/zlib.dll")
set(ZLIB_FOUND TRUE)
]=])

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/zlib-config.cmake
    DESTINATION ${IVW_SHARE_INSTALL_DIR}/zlib
    COMPONENT Application
)
