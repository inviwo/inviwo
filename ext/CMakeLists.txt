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
message(STATUS "add subdirectory: sml")
add_subdirectory(sml)
ivw_register_license_file(NAME "[Boost].SML" VERSION 1.1.0 target inviwo::sml MODULE Core EXT
    URL "https://github.com/boost-experimental/sml"
    TYPE " Boost Software License"
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/sml/LICENSE_1_0.txt
)

# CRoaring
find_package(roaring CONFIG REQUIRED)
ivw_vcpkg_install(roaring MODULE Core EXT)

# Units
find_package(llnl-units CONFIG REQUIRED)
ivw_vcpkg_install(llnl-units MODULE Core EXT)

# Markdown
message(STATUS "add subdirectory: md4c")
add_subdirectory(md4c)
ivw_register_license_file(NAME "MD4C" VERSION 0.4.8 TARGET inviwo::md4c MODULE Core EXT
    URL "http://github.com/mity/md4c"
    TYPE "MIT License"
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/md4c/LICENSE.md
)

if(WIN32) # Stackwalker for windows for stack traces in the log
    message(STATUS "add subdirectory: stackwalker")
    add_subdirectory(stackwalker) 
    ivw_register_license_file(NAME "stackwalker" TARGET inviwo::stackwalker MODULE Core EXT
        URL http://stackwalker.codeplex.com/
        TYPE "BSD-License"
        FILES ${CMAKE_CURRENT_SOURCE_DIR}/stackwalker/LICENSE.txt
    )
endif()

if(IVW_ENABLE_PYTHON)
    find_package(pybind11 CONFIG REQUIRED)
    ivw_vcpkg_install(pybind11 MODULE Core EXT)
endif()
