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
    add_subdirectory(gtest)
    ivw_register_license_file(NAME "Google C++ Testing Framework" TARGET gtest MODULE Core EXT
        URL "https://github.com/google/googletest"
        TYPE "BSD-3-Clause"
        FILES ${IVW_EXTENSIONS_DIR}/gtest/gtest/LICENSE
    )
elseif(IVW_TEST_UNIT_TESTS OR IVW_TEST_INTEGRATION_TESTS)
    find_package(GTest CONFIG REQUIRED)
endif()

# Img formats
option(IVW_USE_EXTERNAL_IMG "Link with external image libraries (zlib, png, jpeg, tiff) instead of building them." OFF)
if (NOT IVW_USE_EXTERNAL_IMG)
    add_subdirectory(zlib-1.2.11) # Defines target ZLIB::ZLIB
    add_subdirectory(libpng)      # Defines target png
    add_subdirectory(libjpeg)     # Defines target JPEG::JPEG
    add_subdirectory(tiff)        # Defines target TIFF::TIFF

    ivw_register_license_file(NAME "zlib" TARGET ZLIB::ZLIB MODULE Core EXT
        URL "http://zlib.net/" TYPE "The zlib/libpng License"
        FILES ${IVW_EXTENSIONS_DIR}/zlib-1.2.11/LICENSE
    )
    ivw_register_license_file(NAME libjpeg-turbo TARGET JPEG::JPEG MODULE Core EXT
        TYPE "BSD-style, acknowledgement: This software is based in part on the work of the Independent JPEG Group"
        URL www.ijg.org FILES ${IVW_EXTENSIONS_DIR}/libjpeg/README
    )
    ivw_register_license_file(NAME tiff TARGET TIFF::TIFF MODULE Core EXT
        TYPE "The MIT License" URL http://www.libtiff.org/
        FILES ${IVW_EXTENSIONS_DIR}/tiff/COPYRIGHT
    )
    ivw_register_license_file(NAME libpng TARGET png MODULE Core EXT 
        TYPE "libpng (BSD-style)" URL http://www.libpng.org/pub/png/libpng.html
        FILES ${IVW_EXTENSIONS_DIR}/libpng/LICENSE.txt
    )
else()
    find_package(ZLIB REQUIRED)
    find_package(JPEG REQUIRED)
    find_package(TIFF REQUIRED)
    find_package(PNG  REQUIRED)
    ivw_vcpkg_install(zlib          MODULE Core EXT)
    ivw_vcpkg_install(libjpeg-turbo MODULE Core EXT)
    ivw_vcpkg_install(tiff          MODULE Core EXT)
    ivw_vcpkg_install(libpng        MODULE Core EXT)
endif()

# Flags
add_subdirectory(flags) # We have local modifications
ivw_register_license_file(NAME "Flags" MODULE Core EXT
    URL "http://grisumbras.github.io/enum-flags/"
    TYPE "The MIT License"
    FILES ${IVW_EXTENSIONS_DIR}/flags/LICENSE
)

# GLM
option(IVW_USE_EXTERNAL_GLM "GLM is provided externaly" OFF)
if(NOT IVW_USE_EXTERNAL_GLM)
    add_subdirectory(glm)
    ivw_register_license_file(NAME "OpenGL Mathematics (GLM)" 
        VERSION 0.9.9.3 MODULE Core EXT URL "https://glm.g-truc.net"
        TYPE "The MIT License" FILES ${IVW_EXTENSIONS_DIR}/glm/LICENSE
    )
else()  
    find_package(glm CONFIG REQUIRED)
    ivw_vcpkg_install(glm MODULE Core EXT)
endif()

add_subdirectory(ticpp)
ivw_register_license_file(NAME "TiCPP" TARGET ticpp MODULE Core EXT
    URL https://github.com/wxFormBuilder/ticpp
    TYPE "The MIT License"
    FILES ${IVW_EXTENSIONS_DIR}/ticpp/LICENSE
)

# benchmark
option(IVW_TEST_BENCHMARKS "Generate benchmark targets" OFF)
if(IVW_TEST_BENCHMARKS)
    option(IVW_USE_EXTERNAL_BENCHMARK "Goggle Benchmark is provided externaly" OFF)
     # don't install, used in build only
    if(NOT IVW_USE_EXTERNAL_BENCHMARK)
        add_subdirectory(benchmark)
    else()
        find_package(benchmark CONFIG REQUIRED)
    endif()
endif()

add_subdirectory(warn)     # No package available
ivw_register_license_file(NAME "warn" MODULE Core EXT
    URL "https://github.com/petersteneteg/warn"
    TYPE "BSD 2-clause"
    FILES ${IVW_TOOLS_DIR}/codegen/warn/warn/LICENSE
)

# tclap
add_subdirectory(tclap)    
ivw_register_license_file(NAME "tclap" TARGET tclap_dummy MODULE Core EXT
    URL http://tclap.sourceforge.net/
    TYPE "The MIT License"
    FILES ${IVW_EXTENSIONS_DIR}/tclap/COPYING
)

# tinydir
option(IVW_USE_EXTERNAL_TINYDIR "Tinydir is provided externaly" OFF)
if(NOT IVW_USE_EXTERNAL_TINYDIR)
    add_subdirectory(tinydir)
    ivw_register_license_file(NAME "TinyDir" MODULE Core EXT
        URL https://github.com/cxong/tinydir
        TYPE "Simplified BSD"
        FILES ${IVW_EXTENSIONS_DIR}/tinydir/COPYING
    )
else()
    ivw_vcpkg_paths(INCLUDE vcpkgInclude) # get the vcpkg include path as a hint if available
    find_path(TINYDIR_INCLUDE_DIR NAMES tinydir.h HINTS ${vcpkgInclude} REQUIRED)
    add_library(tinydir INTERFACE)
    add_library(inviwo::tinydir ALIAS tinydir)
    target_include_directories(tinydir INTERFACE $<BUILD_INTERFACE:${TINYDIR_INCLUDE_DIR}>)
    ivw_vcpkg_install(tinydir MODULE Core EXT)
    ivw_register_package(tinydir tinydir)
endif()

# Half
add_subdirectory(half)
ivw_register_license_file(NAME "half" VERSION 1.12.0 MODULE Core EXT
    URL "http://half.sourceforge.net"
    TYPE "The MIT License"
    FILES ${IVW_EXTENSIONS_DIR}/half/LICENSE.txt
)

# utfcpp
option(IVW_USE_EXTERNAL_UTFCPP "UTFCPP is provided externaly" OFF)
if(NOT IVW_USE_EXTERNAL_UTFCPP)
    add_subdirectory(utf)
    ivw_register_license_file(NAME "UTF8-CPP" VERSION ${utf8cpp_VERSION} MODULE Core EXT
        URL "https://github.com/nemtrif/utfcpp"
        TYPE "Boost Software License"
        FILES ${IVW_EXTENSIONS_DIR}/utf/LICENSE
    )
else()  
    find_package(utf8cpp CONFIG REQUIRED)
    ivw_vcpkg_install(utfcpp MODULE Core EXT)
endif()

# nlohmann json
option(IVW_USE_EXTERNAL_JSON "JSON is provided externaly" OFF)
if(NOT IVW_USE_EXTERNAL_JSON)
    add_subdirectory(json)
    ivw_register_license_file(NAME "JSON for Modern C++" target nlohmann_json MODULE Core EXT
        URL "https://github.com/nlohmann/json"
        TYPE "The MIT License"
        FILES  ${IVW_EXTENSIONS_DIR}/json/LICENSE.MIT
    )
else()
    find_package(nlohmann_json CONFIG REQUIRED)
    ivw_vcpkg_install(nlohmann-json MODULE Core EXT)
endif()

# Fmt
option(IVW_USE_EXTERNAL_FMT "Fmt is provided externaly" OFF)
if(NOT IVW_USE_EXTERNAL_FMT)
    add_subdirectory(fmt)
    ivw_register_license_file(NAME "{fmt}" TARGET fmt::fmt MODULE Core EXT
        URL "https://github.com/fmtlib/fmt"
        TYPE "TBSD 2-Clause"
        FILES ${IVW_EXTENSIONS_DIR}/fmt/fmt/LICENSE.rst
    )
    ivw_default_install_targets(fmt)
else()
    find_package(fmt CONFIG REQUIRED)
    ivw_vcpkg_install(fmt MODULE Core EXT)
endif()

# span
add_subdirectory(span)
ivw_register_license_file(NAME "tcbrindle/span" target inviwo::span MODULE Core EXT
    URL "https://github.com/tcbrindle/span"
    TYPE " Boost Software License"
    FILES ${IVW_EXTENSIONS_DIR}/span/span/LICENSE_1_0.txt
)

# sml
add_subdirectory(sml)
ivw_register_license_file(NAME "[Boost].SML" VERSION 1.1.0 target inviwo::sml MODULE Core EXT
    URL "https://github.com/boost-experimental/sml"
    TYPE " Boost Software License"
    FILES ${IVW_EXTENSIONS_DIR}/sml/LICENSE_1_0.txt
)

# Sigar
if(APPLE)
    # Disable by default since SIGAR does not compile on newer versions of OSX 
    option(IVW_USE_SIGAR "Use sigar to detect system capabilities" OFF)
else()
    option(IVW_USE_SIGAR "Use sigar to detect system capabilities" ON)
endif()
if(IVW_USE_SIGAR)
    add_subdirectory(sigar)
    ivw_register_license_file(NAME "Sigar" TARGET inviwo::sigar MODULE Core EXT
        URL "http://sigar.hyperic.com"
        TYPE "Apache License"
        FILES ${IVW_EXTENSIONS_DIR}/sigar/LICENSE
    )
endif()

# CRoaring
option(IVW_USE_EXTERNAL_ROARING "CRoaring is provided externaly" OFF)
if(NOT IVW_USE_EXTERNAL_ROARING)
    add_subdirectory(CRoaring)
    ivw_register_license_file(NAME "CRoaring" VERSION 0.3.4 TARGET roaring MODULE Core EXT
        URL "https://github.com/RoaringBitmap/CRoaring"
        TYPE "Apache License 2.0"
        FILES ${IVW_EXTENSIONS_DIR}/CRoaring/CRoaring/LICENSE
    )
    ivw_default_install_targets(roaring)
else()
    find_package(roaring CONFIG REQUIRED)
    ivw_vcpkg_install(roaring MODULE Core EXT)
endif()

if(WIN32) # Stackwalker for windows for stack traces in the log
    add_subdirectory(stackwalker) 
    ivw_register_license_file(NAME "stackwalker" TARGET inviwo::stackwalker MODULE Core EXT
        URL http://stackwalker.codeplex.com/
        TYPE "BSD-License"
        FILES ${IVW_EXTENSIONS_DIR}/stackwalker/LICENSE.txt
    )
endif()

if(WIN32 AND MSVC)
    option(IVW_CFG_MSVC_MEMLEAK_TEST "Run memoryleak test within Visual Studio" OFF)
    if(IVW_CFG_MSVC_MEMLEAK_TEST)
        add_subdirectory(vld)
        ivw_register_license_file(NAME "Visual Leak Detector (VLD)" VERSION 2.5.1 MODULE Core EXT
            URL "https://kinddragon.github.io/vld/"
            TYPE "GNU LESSER GENERAL PUBLIC LICENSE"
            FILES ${IVW_EXTENSIONS_DIR}/vld/COPYING.txt
        )
    endif()
endif()

if(Python3_FOUND)
    option(IVW_USE_EXTERNAL_PYBIND11 "Pybind11 is provided externaly" OFF)
    if(NOT IVW_USE_EXTERNAL_PYBIND11)
        add_subdirectory(pybind11)
        ivw_register_license_file(NAME "PyBind11" VERSION 2.6.1 MODULE Core EXT
            URL "https://github.com/pybind/pybind11"
            TYPE "BSD-style"
            FILES ${IVW_EXTENSIONS_DIR}/pybind11/LICENSE
        )
        ivw_register_package(pybind11 pybind11::embed)
    else()
        find_package(pybind11 CONFIG REQUIRED)
        ivw_vcpkg_install(pybind11 COPYRIGHT copyright/LICENSE MODULE Core EXT)
    endif()
endif()
