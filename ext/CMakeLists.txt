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
else()
	find_package(GTest CONFIG REQUIRED)
endif()

# Img foramts
option(IVW_USE_EXTERNAL_IMG "Link with external image libraries (zlib, png, jpeg, tiff) instead of building them." OFF)
if (NOT IVW_USE_EXTERNAL_IMG)
 	add_subdirectory(zlib-1.2.11) # Defines target ZLIB::ZLIB
 	add_subdirectory(libpng)      # Defines target PNG::PNG
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
	ivw_register_license_file(NAME libpng TARGET PNG::PNG MODULE Core EXT 
		TYPE "libpng (BSD-style)" URL http://www.libpng.org/pub/png/libpng.html
	    FILES ${IVW_EXTENSIONS_DIR}/libpng/LICENSE.txt
	)
else()
	find_package(ZLIB REQUIRED)
	find_package(JPEG REQUIRED)
	find_package(TIFF REQUIRED)
	find_package(PNG REQUIRED)
	ivw_vcpkg_install(zlib          MODULE Core EXT)
	ivw_vcpkg_install(libjpeg-turbo MODULE Core EXT)
	ivw_vcpkg_install(tiff          MODULE Core EXT)
	ivw_vcpkg_install(libpng        MODULE Core EXT)
endif()

# Flags
add_subdirectory(flags) # We have local modifications

# GLM
option(IVW_USE_EXTERNAL_GLM "GLM is provided externaly" OFF)
if(NOT IVW_USE_EXTERNAL_GLM)
	add_subdirectory(glm)
	ivw_register_license_file(NAME "OpenGL Mathematics (GLM)" 
		VERSION 0.9.9.3 MODULE Core EXT URL "https://glm.g-truc.net"
	    TYPE "The MIT License" FILES ${IVW_EXTENSIONS_DIR}/glm/LICENSE
	)
else()	
	find_package(glm REQUIRED)
	ivw_vcpkg_install(glm MODULE Core EXT)
endif()

add_subdirectory(ticpp)

# benchmark
option(IVW_TEST_BENCHMARKS "Generate benchmark targets" OFF)
if(IVW_TEST_BENCHMARKS)
	option(IVW_USE_EXTERNAL_BENCHMARK "Goggle Benchmark is provided externaly" OFF)
	 # don't install, used in build only
	if(NOT IVW_USE_EXTERNAL_BENCHMARK)
		add_subdirectory(benchmark)
	else()
		find_package(benchmark REQUIRED)
	endif()
endif()

add_subdirectory(warn)     # No package available
add_subdirectory(tclap)    
add_subdirectory(tinydir)
add_subdirectory(half)
add_subdirectory(utf)

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
	find_package(nlohmann_json REQUIRED)
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
	find_package(fmt REQUIRED)
	ivw_vcpkg_install(fmt MODULE Core EXT)
endif()

add_subdirectory(span)
add_subdirectory(sml)

# Sigar
option(IVW_USE_SIGAR "Use sigar to detect system capabilities" ON)
if(IVW_USE_SIGAR)
	add_subdirectory(sigar)
endif()

# Stackwalker for windows for stack traces in the log
add_subdirectory(stackwalker) 
