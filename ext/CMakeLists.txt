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

option(IVW_USE_EXTERNAL_IMG "Link with external image libraries (zlib, png, jpeg, tiff) instead of building them." OFF)
option(IVW_TEST_BENCHMARKS "Generate benchmark targets" OFF)
option(IVW_USE_SIGAR "Use sigar to detect system capabilities" ON)

if (NOT IVW_USE_EXTERNAL_IMG)
  add_subdirectory(zlib-1.2.11) # Defines target ZLIB::ZLIB
  add_subdirectory(libpng)      # Defines target PNG::PNG
  add_subdirectory(libjpeg)     # Defines target JPEG::JPEG
  add_subdirectory(tiff)        # Defines target TIFF::TIFF
endif()
add_subdirectory(flags)
add_subdirectory(glm)
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
add_subdirectory(fmt)
add_subdirectory(span)
add_subdirectory(sml)
if(IVW_USE_SIGAR)
	add_subdirectory(sigar)
endif()
add_subdirectory(stackwalker) # Add stackwalker for windows for stack traces in the log
