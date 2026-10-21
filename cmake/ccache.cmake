#################################################################################
#
# Inviwo - Interactive Visualization Workshop
#
# Copyright (c) 2026 Inviwo Foundation
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

option(IVW_CFG_USE_CCACHE_MSVC "build using cache on windows" OFF)
if(IVW_CFG_USE_CCACHE_MSVC)
    # By default Visual Studio generators will use /Zi which is not compatible
    # with ccache, so tell Visual Studio to use /Z7 instead.
    message(STATUS "Setting MSVC debug information format to 'Embedded'")
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<$<CONFIG:Debug,RelWithDebInfo>:Embedded>")

    # this expects a copy of ccache.exe to be in CMAKE_BINARY_DIR renamed to cl.exe
    set(CMAKE_VS_GLOBALS
        "CLToolExe=cl.exe"
        "CLToolPath=${CMAKE_BINARY_DIR}"
        "UseMultiToolTask=true"
    )
endif()
option(IVW_CFG_USE_CCACHE_XCODE "build using cache in xcode" OFF)
if(IVW_CFG_USE_CCACHE_XCODE)
    message(STATUS "Configuring ccache for use with XCode")
    message(STATUS " * CXX = ${CMAKE_CXX_COMPILER}")
    message(STATUS " * C   = ${CMAKE_C_COMPILER}")

    find_program(CCACHE_BIN ccache REQUIRED)

    file(CONFIGURE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/launcher-c   CONTENT [=[#!/bin/sh
        export CCACHE_SLOPPINESS=clang_index_store
        exec @CCACHE_BIN@ @CMAKE_C_COMPILER@ "$@"
    ]=]
    @ONLY)
    file(CONFIGURE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/launcher-cxx CONTENT [=[#!/bin/sh
        export CCACHE_SLOPPINESS=clang_index_store
        exec @CCACHE_BIN@ @CMAKE_CXX_COMPILER@ "$@"
    ]=]
    @ONLY)
    file(CHMOD ${CMAKE_CURRENT_BINARY_DIR}/launcher-c ${CMAKE_CURRENT_BINARY_DIR}/launcher-cxx
         FILE_PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE)

    set(CMAKE_XCODE_ATTRIBUTE_CC          "${CMAKE_CURRENT_BINARY_DIR}/launcher-c")
    set(CMAKE_XCODE_ATTRIBUTE_CXX         "${CMAKE_CURRENT_BINARY_DIR}/launcher-cxx")
    set(CMAKE_XCODE_ATTRIBUTE_LD          "${CMAKE_CURRENT_BINARY_DIR}/launcher-c")
    set(CMAKE_XCODE_ATTRIBUTE_LDPLUSPLUS  "${CMAKE_CURRENT_BINARY_DIR}/launcher-cxx")
endif()

if(IVW_CFG_USE_CCACHE_XCODE)
    message(STATUS "Configuring ccache for use with Xcode")

    find_program(CCACHE_BIN ccache REQUIRED)

    # Find the active Apple Clang toolchain.
    execute_process(
        COMMAND xcrun --find clang
        OUTPUT_VARIABLE XCODE_CLANG
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE XCRUN_RESULT
    )
    if(NOT XCRUN_RESULT EQUAL 0 OR NOT EXISTS "${XCODE_CLANG}")
        message(FATAL_ERROR "Could not find Apple Clang using xcrun")
    endif()

    get_filename_component(XCODE_USR_DIR "${XCODE_CLANG}" DIRECTORY)
    get_filename_component(XCODE_USR_DIR "${XCODE_USR_DIR}" DIRECTORY)
    set(XCODE_TOOLCHAIN_LIB "${XCODE_USR_DIR}/lib")

    if(NOT IS_DIRECTORY "${XCODE_TOOLCHAIN_LIB}")
        message(FATAL_ERROR
            "Could not find Xcode toolchain libraries: ${XCODE_TOOLCHAIN_LIB}")
    endif()

    # Make Xcode's expected compiler-relative runtime layout:
    #
    # <build>/ccache-toolchain/usr/bin/clang
    # <build>/ccache-toolchain/usr/bin/clang++
    # <build>/ccache-toolchain/usr/lib -> <Xcode toolchain>/usr/lib
    #
    set(CCACHE_TOOLCHAIN_ROOT
        "${CMAKE_CURRENT_BINARY_DIR}/ccache-toolchain")
    set(CCACHE_TOOLCHAIN_BIN "${CCACHE_TOOLCHAIN_ROOT}/usr/bin")
    set(CCACHE_TOOLCHAIN_USR_LIB "${CCACHE_TOOLCHAIN_ROOT}/usr/lib")

    file(MAKE_DIRECTORY "${CCACHE_TOOLCHAIN_BIN}")

    # Recreate the symlink so it follows the currently selected toolchain.
    file(REMOVE "${CCACHE_TOOLCHAIN_USR_LIB}")
    file(CREATE_LINK "${XCODE_TOOLCHAIN_LIB}" "${CCACHE_TOOLCHAIN_USR_LIB}" SYMBOLIC)

    message(STATUS " * ccache: ${CCACHE_BIN}")
    message(STATUS " * Clang:  ${XCODE_CLANG}")
    message(STATUS " * Runtime libraries: ${XCODE_TOOLCHAIN_LIB}")
    message(STATUS " * CXX = ${CMAKE_CXX_COMPILER}")
    message(STATUS " * C   = ${CMAKE_C_COMPILER}")

    # Generate wrappers with the expected compiler executable names.
    file(CONFIGURE OUTPUT "${CCACHE_TOOLCHAIN_BIN}/clang" CONTENT [=[#!/bin/sh
            export CCACHE_SLOPPINESS=clang_index_store
            exec "@CCACHE_BIN@" "@CMAKE_C_COMPILER@" "$@"
        ]=]
        @ONLY
    )

    file(CONFIGURE OUTPUT "${CCACHE_TOOLCHAIN_BIN}/clang++" CONTENT [=[#!/bin/sh
            export CCACHE_SLOPPINESS=clang_index_store
            exec "@CCACHE_BIN@" "@CMAKE_CXX_COMPILER@" "$@"
        ]=]
        @ONLY
    )

    file(CHMOD
        "${CCACHE_TOOLCHAIN_BIN}/clang"
        "${CCACHE_TOOLCHAIN_BIN}/clang++"
        FILE_PERMISSIONS
            OWNER_READ OWNER_WRITE OWNER_EXECUTE
            GROUP_READ GROUP_EXECUTE
            WORLD_READ WORLD_EXECUTE
    )

    # Let Xcode identify these as Clang executables and find the runtimes
    # relative to their location.
    set(CMAKE_XCODE_ATTRIBUTE_CC          "${CCACHE_TOOLCHAIN_BIN}/clang")
    set(CMAKE_XCODE_ATTRIBUTE_CXX         "${CCACHE_TOOLCHAIN_BIN}/clang++")
    set(CMAKE_XCODE_ATTRIBUTE_LD          "${CCACHE_TOOLCHAIN_BIN}/clang")
    set(CMAKE_XCODE_ATTRIBUTE_LDPLUSPLUS  "${CCACHE_TOOLCHAIN_BIN}/clang++")
endif()
