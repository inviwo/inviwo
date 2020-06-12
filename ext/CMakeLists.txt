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

option(IVW_TEST_BENCHMARKS "Generate benchmark targets" OFF)
option(IVW_USE_SIGAR "Use sigar to detect system capabilities" ON)

option(IVW_USE_EXTERNAL_FMT "Fmt is provided externaly" OFF)


# Img foramts
option(IVW_USE_EXTERNAL_IMG "Link with external image libraries (zlib, png, jpeg, tiff) instead of building them." OFF)
if (NOT IVW_USE_EXTERNAL_IMG)
  add_subdirectory(zlib-1.2.11) # Defines target ZLIB::ZLIB
  add_subdirectory(libpng)      # Defines target PNG::PNG
  add_subdirectory(libjpeg)     # Defines target JPEG::JPEG
  add_subdirectory(tiff)        # Defines target TIFF::TIFF
endif()

# Flags
add_subdirectory(flags)

# GLM
option(IVW_USE_EXTERNAL_GLM "GLM is provided externaly" OFF)
if(NOT IVW_USE_EXTERNAL_GLM)
	add_subdirectory(glm)
	set(glmCopyright ${IVW_EXTENSIONS_DIR}/glm/LICENSE)
	set(glmVersion 0.9.9.3)
else()	
	find_package(glm REQUIRED)
	ivw_vcpkg_install(glm COPYRIGHT glmCopyright VERSION glmVersion)
endif()
ivw_register_license_file(NAME "OpenGL Mathematics (GLM)" 
	VERSION ${glmVersion} 
	MODULE Core EXT
    URL "https://glm.g-truc.net"
    TYPE "The MIT License"
    FILES ${glmCopyright}
)


add_subdirectory(ticpp)
if(IVW_TEST_UNIT_TESTS OR IVW_TEST_INTEGRATION_TESTS)
	add_subdirectory(gtest)
endif()
if(IVW_TEST_BENCHMARKS)
	add_subdirectory(benchmark)
endif()
add_subdirectory(warn)
add_subdirectory(tclap)
add_subdirectory(tinydir)
add_subdirectory(half)
add_subdirectory(utf)
add_subdirectory(json)

# Fmt
if(NOT IVW_USE_EXTERNAL_FMT)
	add_subdirectory(fmt)
	set(fmtCopyright ${IVW_EXTENSIONS_DIR}/fmt/fmt/LICENSE.rst)
else()
	find_package(fmt REQUIRED)
	ivw_vcpkg_install(fmt COPYRIGHT fmtCopyright)
endif()
message("LIc: ${fmtCopyright}")
ivw_register_license_file(NAME "{fmt}" TARGET fmt::fmt MODULE Core EXT
    URL "https://github.com/fmtlib/fmt"
    TYPE "TBSD 2-Clause"
    FILES ${fmtCopyright}
)


add_subdirectory(span)
add_subdirectory(sml)
if(IVW_USE_SIGAR)
	add_subdirectory(sigar)
endif()
add_subdirectory(stackwalker) # Add stackwalker for windows for stack traces in the log
