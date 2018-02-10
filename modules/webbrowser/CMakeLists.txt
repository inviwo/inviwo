# Web browser module based on CEF

#
# CEF configuration.
#
# See https://bitbucket.org/chromiumembedded/cef-project

# Specify the CEF distribution version.
# http://opensource.spotify.com/cefbuilds/index.html
set(CEF_VERSION "3.3202.1680.g700054b")

# Determine the platform.
include("cmake/cef_platform.cmake")
determineCEFPlatform(CEF_PLATFORM)

# Download and extract the CEF binary distribution (executes cmake/DownloadCEF.cmake).
include("cmake/DownloadCEF.cmake")
DownloadCEF("${CEF_PLATFORM}" "${CEF_VERSION}" "${PROJECT_BINARY_DIR}/ext/cef")

# Add the CEF binary distribution's cmake/ directory to the module path.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CEF_ROOT}/cmake")

#
# Load the CEF configuration.
#
# Execute FindCEF.cmake which must exist in CMAKE_MODULE_PATH.
find_package(CEF REQUIRED)

# Inviwo: libcef_dll_wrapper must use same compiler settings to be able to link.
set(CEF_COMPILER_FLAGS ${CMAKE_CXX_FLAGS})
set(CEF_COMPILER_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
set(CEF_COMPILER_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
set(CEF_SHARED_LINKER_FLAGS_DEBUG ${CMAKE_SHARED_LINKER_FLAGS_DEBUG})
set(CEF_SHARED_LINKER_FLAGS_RELEASE ${CMAKE_SHARED_LINKER_FLAGS_RELASE}})

# Include the libcef_dll_wrapper target.
# Comes from the libcef_dll/CMakeLists.txt file in the binary distribution
# directory.
# Inviwo: Linker errors if libcef_dll_wrapper is not built statically
set(BUILD_SHARED_LIBS_TMP ${BUILD_SHARED_LIBS})
set(BUILD_SHARED_LIBS OFF)
add_subdirectory(${CEF_LIBCEF_DLL_WRAPPER_PATH} libcef_dll_wrapper)
# Group in folder 
ivw_folder(libcef_dll_wrapper ext/CEF)
# Logical target used to link the libcef library.
ADD_LOGICAL_TARGET("libcef_lib" "${CEF_LIB_DEBUG}" "${CEF_LIB_RELEASE}")

set(BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS_TMP})

# Determine the target output directory.
# Inviwo: use module target directory as output dir$<TARGET_FILE_DIR:inviwo-module-webbrowser>
#SET_CEF_TARGET_OUT_DIR()
set(CEF_TARGET_OUT_DIR $<TARGET_FILE_DIR:inviwo-module-webbrowser>)

# Inviwo: Hide option, currently no support for sandbox and then set default value
unset(USE_SANDBOX CACHE)
set(USE_SANDBOX FALSE)

# Display configuration settings.
#PRINT_CEF_CONFIG()

#--------------------------------------------------------------------
# Inviwo WebBrowser Module
ivw_module(WebBrowser)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    interaction/cefinteractionhandler.h
    interaction/cefkeyboardmapping.h
    processors/webbrowserprocessor.h
    renderhandlergl.h
    webbrowserapp.h
    webbrowserclient.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    interaction/cefinteractionhandler.cpp
    interaction/cefkeyboardmapping.cpp
    processors/webbrowserprocessor.cpp
    renderhandlergl.cpp
    webbrowserapp.cpp
    webbrowserclient.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

#--------------------------------------------------------------------
# Add shaders
set(SHADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/img_convert_cef.frag
)
ivw_group("Shader Files" ${SHADER_FILES})

#--------------------------------------------------------------------
# Add Unittests
set(TEST_FILES
    #${CMAKE_CURRENT_SOURCE_DIR}/tests/unittests/webbrowser-test.cpp
)
ivw_add_unittest(${TEST_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

target_include_directories(inviwo-module-webbrowser PUBLIC ${CEF_INCLUDE_PATH})
target_link_libraries(inviwo-module-webbrowser PUBLIC libcef_lib libcef_dll_wrapper ${CEF_STANDARD_LIBS})

#--------------------------------------------------------------------
# Package or build shaders into resources
ivw_handle_shader_resources(${CMAKE_CURRENT_SOURCE_DIR}/glsl ${SHADER_FILES})

# Inviwo: CEF only provides release/debug configurations. Set RelWithDebInfo and MinSizeRel to use release 
set(CEF_BINARY_DIR "${_CEF_ROOT}/$<$<CONFIG:RelWithDebInfo>:Release>$<$<CONFIG:MinSizeRel>:Release>$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>")

ivw_register_license_file(NAME "Chromium Embedded Framework" VERSION ${CEF_VERSION} MODULE WebBrowser
    URL "https://bitbucket.org/chromiumembedded/cef"
    TYPE "BSD-license"
    FILES ${CEF_ROOT}/LICENSE.txt
)

#
# Setup CEF helper executable. Started from the web browser module as a sub-process.
#

# Process running CEF browser
set(WEBBROWSER_HELPER_SOURCES
    # Should include webbrowserapp .h/.cpp but get precompiled header errors
    # Tried to ignore cotire in all sorts of ways but did not work. 
    # Hack: webbrowserapp-cotire-fix is a copy of webbrowserapp, resolves cotire issue.
    webbrowserapp-cotire-fix.h
    webbrowserapp-cotire-fix.cpp
    cefprocesshelper.cpp
)
# place helper in separate executable
set(CEF_HELPER_TARGET "inviwo_web_helper" CACHE INTERNAL "CEF_HELPER_TARGET")

add_executable(${CEF_HELPER_TARGET} ${WEBBROWSER_HELPER_SOURCES})
add_dependencies(${CEF_HELPER_TARGET} libcef_dll_wrapper)
target_link_libraries(${CEF_HELPER_TARGET} PRIVATE libcef_lib libcef_dll_wrapper ${CEF_STANDARD_LIBS})
# Add IVW_ROOT_DIR to include file in "modules/webbrowser/...h"
target_include_directories(${CEF_HELPER_TARGET} PRIVATE "${IVW_ROOT_DIR}")
target_include_directories(${CEF_HELPER_TARGET} PRIVATE "${CEF_INCLUDE_PATH}")
# Use inviwo preprocessor definitions  
ivw_define_standard_definitions(WebBrowser ${CEF_HELPER_TARGET})
ivw_define_standard_properties(${CEF_HELPER_TARGET})
ivw_suppress_compiler_warnings(${CEF_HELPER_TARGET})
# Tried to ignore cotire to resolve webbrowserapp issue but it does not work
# This function is not for turning off cotire... It is for setting the cotire ignore include paths.
#ivw_cotire_ignore_on_target(inviwo-module-webbrowser)

# Inviwo: Needed? (Windows only) Add the custom manifest files to the executable.
#ADD_WINDOWS_CEF_MANIFEST("${CEF_TARGET_OUT_DIR}" "${WEBBROWSER_MODULE_PATH}" "${CEF_HELPER_TARGET}" "exe")
# Put in ext folder in developer IDE
ivw_folder(${CEF_HELPER_TARGET} ext/CEF)

# Copy CEF binary and resource files to the target output directory.
COPY_FILES("inviwo-module-webbrowser" "${CEF_BINARY_FILES}" "${CEF_BINARY_DIR}" "${CEF_TARGET_OUT_DIR}")
COPY_FILES("inviwo-module-webbrowser" "${CEF_RESOURCE_FILES}" "${CEF_RESOURCE_DIR}" "${CEF_TARGET_OUT_DIR}")





