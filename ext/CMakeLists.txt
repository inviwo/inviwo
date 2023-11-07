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

# Dependencies between libraries are solved using the given order
# Note: We prefer building dependencies our self to using find_library,
# since it does not necessarily give you the dll for packaging.
# It also ensures that we are in control of the used version
# and is built using the same, Inviwo, settings.

ivw_append_install_list(DIRECTORY)

# GTest
option(IVW_USE_EXTERNAL_GTEST "GTest is provided externaly" OFF)
 # don't install, used in build only
if(NOT IVW_USE_EXTERNAL_GTEST AND (IVW_TEST_UNIT_TESTS OR IVW_TEST_INTEGRATION_TESTS))
    message(STATUS "add subdirectory: gtest")
    add_subdirectory(gtest)
    ivw_register_license_file(NAME "Google C++ Testing Framework" TARGET gtest MODULE Core EXT
        URL "https://github.com/google/googletest"
        TYPE "BSD-3-Clause"
        FILES ${CMAKE_CURRENT_SOURCE_DIR}/gtest/gtest/LICENSE
    )
elseif(IVW_TEST_UNIT_TESTS OR IVW_TEST_INTEGRATION_TESTS)
    find_package(GTest CONFIG REQUIRED)
    ivw_vcpkg_install(GTest          MODULE Core EXT)
endif()

# Img formats
option(IVW_USE_EXTERNAL_IMG "Link with external image libraries (zlib, png, jpeg) instead of building them." OFF)
if (NOT IVW_USE_EXTERNAL_IMG)
    message(STATUS "add subdirectory: zlib")
    add_subdirectory(zlib-1.2.11) # Defines target ZLIB::ZLIB
    message(STATUS "add subdirectory: libpng")
    add_subdirectory(libpng)      # Defines target png
    message(STATUS "add subdirectory: libjpeg")
    add_subdirectory(libjpeg)     # Defines target JPEG::JPEG

    ivw_register_license_file(NAME "zlib" TARGET ZLIB::ZLIB MODULE Core EXT
        URL "http://zlib.net/" TYPE "The zlib/libpng License"
        FILES ${CMAKE_CURRENT_SOURCE_DIR}/zlib-1.2.11/LICENSE
    )
    ivw_register_license_file(NAME libjpeg-turbo TARGET JPEG::JPEG MODULE Core EXT
        TYPE "BSD-style, acknowledgement: This software is based in part on the work of the Independent JPEG Group"
        URL www.ijg.org FILES ${CMAKE_CURRENT_SOURCE_DIR}/libjpeg/README
    )
    ivw_register_license_file(NAME libpng TARGET png MODULE Core EXT 
        TYPE "libpng (BSD-style)" URL http://www.libpng.org/pub/png/libpng.html
        FILES ${CMAKE_CURRENT_SOURCE_DIR}/libpng/LICENSE.txt
    )
else()
    find_package(ZLIB REQUIRED)
    find_package(JPEG REQUIRED)
    find_package(PNG  REQUIRED)
    ivw_vcpkg_install(zlib          MODULE Core EXT)
    ivw_vcpkg_install(libjpeg-turbo MODULE Core EXT)
    ivw_vcpkg_install(libpng        MODULE Core EXT)
endif()

# TIFF
option(IVW_USE_EXTERNAL_TIFF "Link with external image library tiff instead of building it." OFF)
if (NOT IVW_USE_EXTERNAL_TIFF)
    message(STATUS "add subdirectory: tiff")
    add_subdirectory(tiff)        # Defines target TIFF::TIFF
    ivw_register_license_file(NAME tiff TARGET TIFF::TIFF MODULE Core EXT
        TYPE "The MIT License" URL http://www.libtiff.org/
        FILES ${CMAKE_CURRENT_SOURCE_DIR}/tiff/COPYRIGHT
    )
else()
    find_package(TIFF REQUIRED)
    ivw_vcpkg_install(tiff          MODULE Core EXT)
endif()

# Flags
message(STATUS "add subdirectory: flags")
add_subdirectory(flags) # We have local modifications
ivw_register_license_file(NAME "Flags" MODULE Core EXT
    URL "http://grisumbras.github.io/enum-flags/"
    TYPE "The MIT License"
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/flags/LICENSE
)

# GLM
option(IVW_USE_EXTERNAL_GLM "GLM is provided externaly" OFF)
if(NOT IVW_USE_EXTERNAL_GLM)
    message(STATUS "add subdirectory: glm")
    add_subdirectory(glm)
    ivw_register_license_file(NAME "OpenGL Mathematics (GLM)" 
        VERSION 0.9.9.3 MODULE Core EXT URL "https://glm.g-truc.net"
        TYPE "The MIT License" FILES ${CMAKE_CURRENT_SOURCE_DIR}/glm/LICENSE
    )
else()  
    find_package(glm CONFIG REQUIRED)
    ivw_vcpkg_install(glm MODULE Core EXT)
endif()

message(STATUS "add subdirectory: ticpp")
add_subdirectory(ticpp)
ivw_register_license_file(NAME "TiCPP" TARGET ticpp MODULE Core EXT
    URL https://github.com/wxFormBuilder/ticpp
    TYPE "The MIT License"
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/ticpp/LICENSE
)

# benchmark
option(IVW_TEST_BENCHMARKS "Generate benchmark targets" OFF)
if(IVW_TEST_BENCHMARKS)
    option(IVW_USE_EXTERNAL_BENCHMARK "Goggle Benchmark is provided externaly" OFF)
     # don't install, used in build only
    if(NOT IVW_USE_EXTERNAL_BENCHMARK)
        message(STATUS "add subdirectory: benchmark")
        add_subdirectory(benchmark)
    else()
        find_package(benchmark CONFIG REQUIRED)
    endif()
endif()

# Warn
option(IVW_USE_EXTERNAL_WARN "Warn is provided externaly" OFF)
if(NOT IVW_USE_EXTERNAL_WARN)
    message(STATUS "add subdirectory: warn")
    add_subdirectory(warn)     # No package available
    ivw_register_license_file(NAME "warn" MODULE Core EXT
        URL "https://github.com/petersteneteg/warn"
        TYPE "BSD 2-clause"
        FILES ${IVW_ROOT_DIR}/tools/codegen/warn/warn/LICENSE
    )
else()
    find_package(warn CONFIG REQUIRED)
    ivw_vcpkg_install(warn MODULE Core EXT)
endif()

# tclap
option(IVW_USE_EXTERNAL_TCLAP "TCLAP is provided externaly" OFF)
if(NOT IVW_USE_EXTERNAL_TCLAP)
    message(STATUS "add subdirectory: tclap")
    add_subdirectory(tclap)
    ivw_register_license_file(NAME "tclap" TARGET tclap_dummy MODULE Core EXT
        URL http://tclap.sourceforge.net/
        TYPE "The MIT License"
        FILES ${CMAKE_CURRENT_SOURCE_DIR}/tclap/COPYING
    )
else()
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
    )

    ivw_install_helper(TARGET tclap NAMESPACE inviwo DESTINATION inviwo)

endif()

# Half
message(STATUS "add subdirectory: help")
add_subdirectory(half)
ivw_register_license_file(NAME "half" VERSION 1.12.0 MODULE Core EXT
    URL "http://half.sourceforge.net"
    TYPE "The MIT License"
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/half/LICENSE.txt
)

# utfcpp
option(IVW_USE_EXTERNAL_UTFCPP "UTFCPP is provided externaly" OFF)
if(NOT IVW_USE_EXTERNAL_UTFCPP)
    message(STATUS "add subdirectory: utf")
    add_subdirectory(utf)
    ivw_register_license_file(NAME "UTF8-CPP" VERSION ${utf8cpp_VERSION} MODULE Core EXT
        URL "https://github.com/nemtrif/utfcpp"
        TYPE "Boost Software License"
        FILES ${CMAKE_CURRENT_SOURCE_DIR}/utf/LICENSE
    )
else()  
    find_package(utf8cpp CONFIG REQUIRED)
    ivw_vcpkg_install(utfcpp MODULE Core EXT)
endif()

# nlohmann json
option(IVW_USE_EXTERNAL_JSON "JSON is provided externaly" OFF)
if(NOT IVW_USE_EXTERNAL_JSON)
    message(STATUS "add subdirectory: json")
    add_subdirectory(json)
    ivw_register_license_file(NAME "JSON for Modern C++" target nlohmann_json MODULE Core EXT
        URL "https://github.com/nlohmann/json"
        TYPE "The MIT License"
        FILES  ${CMAKE_CURRENT_SOURCE_DIR}/json/LICENSE.MIT
    )
else()
    find_package(nlohmann_json CONFIG REQUIRED)
    ivw_vcpkg_install(nlohmann-json MODULE Core EXT)
endif()

# Fmt
option(IVW_USE_EXTERNAL_FMT "Fmt is provided externaly" OFF)
if(NOT IVW_USE_EXTERNAL_FMT)
    message(STATUS "add subdirectory: fmt")
    add_subdirectory(fmt)
    ivw_register_license_file(NAME "{fmt}" TARGET fmt::fmt MODULE Core EXT
        URL "https://github.com/fmtlib/fmt"
        TYPE "MIT license"
        FILES ${CMAKE_CURRENT_SOURCE_DIR}/fmt/fmt/LICENSE.rst
    )
else()
    find_package(fmt CONFIG REQUIRED)
    ivw_vcpkg_install(fmt MODULE Core EXT)
endif()

# sml
message(STATUS "add subdirectory: sml")
add_subdirectory(sml)
ivw_register_license_file(NAME "[Boost].SML" VERSION 1.1.0 target inviwo::sml MODULE Core EXT
    URL "https://github.com/boost-experimental/sml"
    TYPE " Boost Software License"
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/sml/LICENSE_1_0.txt
)

# CRoaring
option(IVW_USE_EXTERNAL_ROARING "CRoaring is provided externaly" OFF)
if(NOT IVW_USE_EXTERNAL_ROARING)
    message(STATUS "add subdirectory: CRoaring")
    add_subdirectory(CRoaring)
    ivw_register_license_file(NAME "CRoaring" VERSION 0.3.4 TARGET roaring MODULE Core EXT
        URL "https://github.com/RoaringBitmap/CRoaring"
        TYPE "Apache License 2.0"
        FILES ${CMAKE_CURRENT_SOURCE_DIR}/CRoaring/CRoaring/LICENSE
    )
    ivw_default_install_targets(roaring)
else()
    find_package(roaring CONFIG REQUIRED)
    ivw_vcpkg_install(roaring MODULE Core EXT)
endif()

# Units
option(IVW_USE_EXTERNAL_UNITS "Units is provided externaly (https://github.com/LLNL/units)" OFF)
if(NOT IVW_USE_EXTERNAL_UNITS)
    message(STATUS "add subdirectory: units")
    add_subdirectory(units)
    ivw_register_license_file(NAME "Units" VERSION 0.5.0 TARGET llnl-units::units MODULE Core EXT
        URL "https://github.com/LLNL/units"
        TYPE "BSD 3-Clause"
        FILES ${CMAKE_CURRENT_SOURCE_DIR}/units/units/LICENSE
    )
else()
    find_package(llnl-units CONFIG REQUIRED)
    ivw_vcpkg_install(llnl-units MODULE Core EXT)
endif()

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
    option(IVW_USE_EXTERNAL_PYBIND11 "Pybind11 is provided externaly" OFF)
    if(NOT IVW_USE_EXTERNAL_PYBIND11)
        message(STATUS "add subdirectory: pybind11")
        add_subdirectory(pybind11)
        ivw_register_license_file(NAME "PyBind11" VERSION 2.6.1 MODULE Core EXT
            URL "https://github.com/pybind/pybind11"
            TYPE "BSD-style"
            FILES ${CMAKE_CURRENT_SOURCE_DIR}/pybind11/LICENSE
        )
        ivw_register_package(pybind11 pybind11::embed)
    else()
        find_package(pybind11 CONFIG REQUIRED)
        ivw_vcpkg_install(pybind11 COPYRIGHT copyright/LICENSE MODULE Core EXT)
    endif()
endif()
