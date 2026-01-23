#################################################################################
#
# Inviwo - Interactive Visualization Workshop
#
# Copyright (c) 2013-2026 Inviwo Foundation
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

# Windows Code Signing Support
#
# This module provides functions for signing Windows executables and installers.
#
# Usage:
#   include(codesign)
#   ivw_sign_target(target_name)
#
# Configuration Options:
#   IVW_SIGN_WINDOWS_BINARIES  - Enable/disable code signing (default: OFF)
#   IVW_WINDOWS_SIGN_IDENTITY  - Certificate thumbprint or subject name
#   IVW_WINDOWS_TIMESTAMP_URL  - Timestamp server URL
#   IVW_WINDOWS_SIGN_METHOD    - Signing method: "signtool", "azuresigntool", or "custom"
#
# For Azure SignTool (EV certificates):
#   IVW_AZURE_KEY_VAULT_URI    - Azure Key Vault URI
#   IVW_AZURE_CERT_NAME        - Certificate name in Key Vault
#
# Environment variables can also be used (GitHub Actions secrets):
#   WINDOWS_SIGNING_CERT_PASSWORD - Certificate password
#   AZURE_CLIENT_ID, AZURE_CLIENT_SECRET, AZURE_TENANT_ID - Azure credentials

option(IVW_SIGN_WINDOWS_BINARIES "Sign Windows executables and installers" OFF)
set(IVW_WINDOWS_SIGN_IDENTITY "" CACHE STRING "Certificate thumbprint or subject name for signing")
set(IVW_WINDOWS_TIMESTAMP_URL "http://timestamp.digicert.com" CACHE STRING "Timestamp server URL")
set(IVW_WINDOWS_SIGN_METHOD "signtool" CACHE STRING "Signing method: signtool, azuresigntool, or custom")
set_property(CACHE IVW_WINDOWS_SIGN_METHOD PROPERTY STRINGS "signtool" "azuresigntool" "custom")

# Azure Key Vault settings (for EV certificates)
set(IVW_AZURE_KEY_VAULT_URI "" CACHE STRING "Azure Key Vault URI for code signing")
set(IVW_AZURE_CERT_NAME "" CACHE STRING "Certificate name in Azure Key Vault")

# Find SignTool
if(WIN32 AND IVW_SIGN_WINDOWS_BINARIES)
    # Try to find signtool in Windows SDK
    if(CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "AMD64")
        set(_arch "x64")
    else()
        set(_arch "x86")
    endif()
    
    # Common Windows SDK paths
    set(_sdk_paths
        "$ENV{ProgramFiles\(x86\)}/Windows Kits/10/bin/10.0.22621.0/${_arch}"
        "$ENV{ProgramFiles\(x86\)}/Windows Kits/10/bin/10.0.22000.0/${_arch}"
        "$ENV{ProgramFiles\(x86\)}/Windows Kits/10/bin/10.0.19041.0/${_arch}"
        "$ENV{ProgramFiles\(x86\)}/Windows Kits/10/bin/${_arch}"
        "$ENV{ProgramFiles\(x86\)}/Windows Kits/8.1/bin/${_arch}"
    )
    
    find_program(SIGNTOOL_EXECUTABLE signtool
        HINTS ${_sdk_paths}
        DOC "Path to Windows SignTool"
    )
    
    if(NOT SIGNTOOL_EXECUTABLE)
        message(WARNING "SignTool not found. Code signing will not be available.")
        set(IVW_SIGN_WINDOWS_BINARIES OFF CACHE BOOL "Sign Windows executables and installers" FORCE)
    else()
        message(STATUS "Found SignTool: ${SIGNTOOL_EXECUTABLE}")
    endif()
    
    # Check for AzureSignTool if using Azure method
    if(IVW_WINDOWS_SIGN_METHOD STREQUAL "azuresigntool")
        find_program(AZURESIGNTOOL_EXECUTABLE AzureSignTool
            DOC "Path to AzureSignTool"
        )
        if(NOT AZURESIGNTOOL_EXECUTABLE)
            message(WARNING "AzureSignTool not found. Install with: dotnet tool install --global AzureSignTool")
        else()
            message(STATUS "Found AzureSignTool: ${AZURESIGNTOOL_EXECUTABLE}")
        endif()
    endif()
endif()

# Function to sign a file using SignTool
# Arguments:
#   FILE_PATH - Path to the file to sign
function(ivw_sign_file_signtool FILE_PATH)
    if(NOT IVW_SIGN_WINDOWS_BINARIES OR NOT SIGNTOOL_EXECUTABLE)
        return()
    endif()
    
    set(sign_args sign)
    
    # Add certificate identity if specified
    if(IVW_WINDOWS_SIGN_IDENTITY)
        list(APPEND sign_args /sha1 "${IVW_WINDOWS_SIGN_IDENTITY}")
    else()
        # Auto-select certificate
        list(APPEND sign_args /a)
    endif()
    
    # Add timestamp
    if(IVW_WINDOWS_TIMESTAMP_URL)
        list(APPEND sign_args /tr "${IVW_WINDOWS_TIMESTAMP_URL}" /td sha256)
    endif()
    
    # Use SHA256 digest
    list(APPEND sign_args /fd sha256)
    
    # Add verbose output
    list(APPEND sign_args /v)
    
    # Add file path
    list(APPEND sign_args "${FILE_PATH}")
    
    message(STATUS "Signing: ${FILE_PATH}")
    execute_process(
        COMMAND "${SIGNTOOL_EXECUTABLE}" ${sign_args}
        RESULT_VARIABLE sign_result
        OUTPUT_VARIABLE sign_output
        ERROR_VARIABLE sign_error
    )
    
    if(NOT sign_result EQUAL 0)
        message(WARNING "Failed to sign ${FILE_PATH}: ${sign_error}")
    endif()
endfunction()

# Function to sign a file using AzureSignTool (for EV certificates)
# Arguments:
#   FILE_PATH - Path to the file to sign
function(ivw_sign_file_azuresigntool FILE_PATH)
    if(NOT IVW_SIGN_WINDOWS_BINARIES OR NOT AZURESIGNTOOL_EXECUTABLE)
        return()
    endif()
    
    if(NOT IVW_AZURE_KEY_VAULT_URI OR NOT IVW_AZURE_CERT_NAME)
        message(WARNING "Azure Key Vault settings not configured for signing")
        return()
    endif()
    
    # Get Azure credentials from environment
    set(azure_client_id "$ENV{AZURE_CLIENT_ID}")
    set(azure_client_secret "$ENV{AZURE_CLIENT_SECRET}")
    set(azure_tenant_id "$ENV{AZURE_TENANT_ID}")
    
    if(NOT azure_client_id OR NOT azure_client_secret OR NOT azure_tenant_id)
        message(WARNING "Azure credentials not found in environment")
        return()
    endif()
    
    set(sign_args sign)
    list(APPEND sign_args -kvu "${IVW_AZURE_KEY_VAULT_URI}")
    list(APPEND sign_args -kvi "${azure_client_id}")
    list(APPEND sign_args -kvs "${azure_client_secret}")
    list(APPEND sign_args -kvt "${azure_tenant_id}")
    list(APPEND sign_args -kvc "${IVW_AZURE_CERT_NAME}")
    
    if(IVW_WINDOWS_TIMESTAMP_URL)
        list(APPEND sign_args -tr "${IVW_WINDOWS_TIMESTAMP_URL}" -td sha256)
    endif()
    
    list(APPEND sign_args "${FILE_PATH}")
    
    message(STATUS "Signing with Azure: ${FILE_PATH}")
    execute_process(
        COMMAND "${AZURESIGNTOOL_EXECUTABLE}" ${sign_args}
        RESULT_VARIABLE sign_result
        OUTPUT_VARIABLE sign_output
        ERROR_VARIABLE sign_error
    )
    
    if(NOT sign_result EQUAL 0)
        message(WARNING "Failed to sign ${FILE_PATH}: ${sign_error}")
    endif()
endfunction()

# Main signing function - dispatches to appropriate method
# Arguments:
#   FILE_PATH - Path to the file to sign
function(ivw_sign_file FILE_PATH)
    if(NOT IVW_SIGN_WINDOWS_BINARIES)
        return()
    endif()
    
    if(NOT WIN32)
        return()
    endif()
    
    if(IVW_WINDOWS_SIGN_METHOD STREQUAL "azuresigntool")
        ivw_sign_file_azuresigntool("${FILE_PATH}")
    elseif(IVW_WINDOWS_SIGN_METHOD STREQUAL "signtool")
        ivw_sign_file_signtool("${FILE_PATH}")
    elseif(IVW_WINDOWS_SIGN_METHOD STREQUAL "custom")
        message(STATUS "Custom signing method selected - implement in project")
    else()
        message(WARNING "Unknown signing method: ${IVW_WINDOWS_SIGN_METHOD}")
    endif()
endfunction()

# Function to sign all executables and DLLs in a directory
# Arguments:
#   DIRECTORY - Directory to search for files
function(ivw_sign_directory DIRECTORY)
    if(NOT IVW_SIGN_WINDOWS_BINARIES OR NOT WIN32)
        return()
    endif()
    
    file(GLOB_RECURSE exe_files "${DIRECTORY}/*.exe")
    file(GLOB_RECURSE dll_files "${DIRECTORY}/*.dll")
    
    foreach(file IN LISTS exe_files dll_files)
        ivw_sign_file("${file}")
    endforeach()
endfunction()

# Function to add post-build signing command to a target
# Arguments:
#   TARGET_NAME - Name of the CMake target to sign
function(ivw_sign_target TARGET_NAME)
    if(NOT IVW_SIGN_WINDOWS_BINARIES OR NOT WIN32)
        return()
    endif()
    
    if(NOT SIGNTOOL_EXECUTABLE AND NOT AZURESIGNTOOL_EXECUTABLE)
        return()
    endif()
    
    get_target_property(target_type ${TARGET_NAME} TYPE)
    if(NOT target_type MATCHES "EXECUTABLE|SHARED_LIBRARY")
        return()
    endif()
    
    # Build signing command based on method
    if(IVW_WINDOWS_SIGN_METHOD STREQUAL "azuresigntool" AND AZURESIGNTOOL_EXECUTABLE)
        set(sign_cmd "${AZURESIGNTOOL_EXECUTABLE}" sign
            -kvu "${IVW_AZURE_KEY_VAULT_URI}"
            -kvi "$ENV{AZURE_CLIENT_ID}"
            -kvs "$ENV{AZURE_CLIENT_SECRET}"
            -kvt "$ENV{AZURE_TENANT_ID}"
            -kvc "${IVW_AZURE_CERT_NAME}"
            -tr "${IVW_WINDOWS_TIMESTAMP_URL}"
            -td sha256
            "$<TARGET_FILE:${TARGET_NAME}>"
        )
    else()
        set(sign_cmd "${SIGNTOOL_EXECUTABLE}" sign)
        if(IVW_WINDOWS_SIGN_IDENTITY)
            list(APPEND sign_cmd /sha1 "${IVW_WINDOWS_SIGN_IDENTITY}")
        else()
            list(APPEND sign_cmd /a)
        endif()
        if(IVW_WINDOWS_TIMESTAMP_URL)
            list(APPEND sign_cmd /tr "${IVW_WINDOWS_TIMESTAMP_URL}" /td sha256)
        endif()
        list(APPEND sign_cmd /fd sha256 /v "$<TARGET_FILE:${TARGET_NAME}>")
    endif()
    
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${sign_cmd}
        COMMENT "Signing ${TARGET_NAME}"
        VERBATIM
    )
endfunction()
