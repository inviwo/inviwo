# Web browser module based on CEF

#
# CEF configuration.
#
# See https://bitbucket.org/chromiumembedded/cef-project

# Specify the CEF distribution version.
# http://opensource.spotify.com/cefbuilds/index.html
# Test before upgrading version since they are sometimes unstable:
# 3.3202.1680.g700054b Problems with event handling of html-range elements (sliders)
# 3.3359.1774.gd49d25f Crashes when creating browsers
# 3.3325.1758.g9aea513 Crashes on scroll event (Mac)
set(CEF_VERSION "3.3396.1775.g5340bb0")

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
# Inviwo: Remove warnings (-w), and build error due to 'OSAtomicCompareAndSwap32' is deprecated
if(CMAKE_COMPILER_IS_GNUCXX OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    target_compile_options(libcef_dll_wrapper PRIVATE -Wno-deprecated-declarations -w)
endif()
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
    cefimageconverter.h
    interaction/cefinteractionhandler.h
    interaction/cefkeyboardmapping.h
    processors/webbrowserprocessor.h
    properties/boolpropertywidgetcef.h
    properties/buttonpropertywidgetcef.h
    properties/ordinalpropertywidgetcef.h
    properties/propertycefsynchronizer.h
    properties/propertywidgetcef.h
    properties/stringpropertywidgetcef.h
    properties/templatepropertywidgetcef.h
    renderhandlergl.h
    webbrowserapp.h
    webbrowserclient.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    cefimageconverter.cpp
    interaction/cefinteractionhandler.cpp
    interaction/cefkeyboardmapping.cpp
    processors/webbrowserprocessor.cpp
    properties/boolpropertywidgetcef.cpp
    properties/buttonpropertywidgetcef.cpp
    properties/ordinalpropertywidgetcef.cpp
    properties/propertycefsynchronizer.cpp
    properties/propertywidgetcef.cpp
    properties/stringpropertywidgetcef.cpp
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
    app_switches.h
    app_switches.cpp
    webrendererapp.h
    webrendererapp.cpp
    cefprocesshelper.cpp
)
set(WEBBROWSER_RESOURCES_WINDOWS
  inviwo_web_helper.exe.manifest # Resolves "Check failed: fallback_available" in debug mode
)
set(WEBBROWSER_RESOURCES_MACOSX
    mac/helper-Info.plist
)
APPEND_PLATFORM_SOURCES(WEBBROWSER_RESOURCES)
source_group(WebBrowser\\\\resources FILES ${WEBBROWSER_RESOURCES})

set(WEBBROWSER_HELPER_SOURCES
    ${WEBBROWSER_HELPER_SOURCES}
    ${WEBBROWSER_RESOURCES}
)
# place helper in separate executable
set(CEF_HELPER_TARGET "inviwo_web_helper" CACHE INTERNAL "CEF_HELPER_TARGET")

add_executable(${CEF_HELPER_TARGET} MACOSX_BUNDLE ${WEBBROWSER_HELPER_SOURCES})
add_dependencies(${CEF_HELPER_TARGET} libcef_dll_wrapper)
# Build helper before module so that we can copy it
add_dependencies(inviwo-module-webbrowser ${CEF_HELPER_TARGET})
target_link_libraries(${CEF_HELPER_TARGET} PRIVATE libcef_lib libcef_dll_wrapper ${CEF_STANDARD_LIBS})
# Add IVW_ROOT_DIR to include file in "modules/webbrowser/...h"
target_include_directories(${CEF_HELPER_TARGET} PRIVATE "${IVW_ROOT_DIR}")
# IVW_EXTENSIONS_DIR is necessary to be able to include warn
target_include_directories(${CEF_HELPER_TARGET} PRIVATE "${IVW_EXTENSIONS_DIR}")
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

if (OS_MACOSX)
    # Output paths for the app bundles: bin/configuration, for example bin/Debug.
    set(CEF_APP "$<TARGET_BUNDLE_DIR:${CEF_HELPER_TARGET}>/../")
    set(CEF_HELPER_APP "$<TARGET_BUNDLE_DIR:${CEF_HELPER_TARGET}>") # full/path/to/helper.app

    # Copy files into the bin directory (used during development)
    add_custom_command(
        TARGET inviwo-module-webbrowser
        POST_BUILD
        # Copy the CEF framework into the Frameworks directory.
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CEF_BINARY_DIR}/Chromium Embedded Framework.framework"
        "${CEF_APP}/Frameworks/Chromium Embedded Framework.framework"
        VERBATIM
    )
    # Copy the helper app bundle into the Frameworks directory (used by installer)
    install(DIRECTORY "${CEF_HELPER_APP}"
            DESTINATION "Inviwo.app/Contents/Frameworks/${CEF_HELPER_TARGET}.app"
            COMPONENT inviwo-module-webbrowser)
    # Copy the CEF framework into the Frameworks directory (used by installer)
    install(DIRECTORY "${CEF_BINARY_DIR}/Chromium Embedded Framework.framework"
            DESTINATION "Inviwo.app/Contents/Frameworks/Chromium Embedded Framework.framework"
            COMPONENT inviwo-module-webbrowser)

    # Fix the framework rpath in the main executable.
    # The main app is at $app_name.app/Contents/MacOS/$app_name
    # Set rpath to both Contents/ (used by installed app), and output binary dir (during development)
    # so that the loader can find Frameworks/.
    set_target_properties(inviwo-module-webbrowser PROPERTIES INSTALL_RPATH
        "@executable_path/..;@executable_path/../../..")
    set_target_properties(inviwo-module-webbrowser PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)

    # The helper is installed in $app_name.app/Contents/Frameworks/inviwo_web_helper.app/Contents/MacOS/
    # Set rpath up to Contents/ (used by installed app), and output binary dir (during development)
    # so that the loader can find the Frameworks directory.
    set_target_properties(${CEF_HELPER_TARGET} PROPERTIES INSTALL_RPATH
        "@executable_path/../../../..;@executable_path/../../..")
    set_target_properties(${CEF_HELPER_TARGET} PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)

    set_target_properties(${CEF_HELPER_TARGET} PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/mac/helper-Info.plist
    )
else()
    # Copy CEF binary and resource files to the target output directory.
    COPY_FILES("inviwo-module-webbrowser" "${CEF_BINARY_FILES}" "${CEF_BINARY_DIR}" "${CEF_TARGET_OUT_DIR}")
    COPY_FILES("inviwo-module-webbrowser" "${CEF_RESOURCE_FILES}" "${CEF_RESOURCE_DIR}" "${CEF_TARGET_OUT_DIR}")

    # Copy files into the bin directory (used by installer)
    install(DIRECTORY "${CEF_BINARY_DIR}/" # Note importance of / in the end to not put files into subfolder
            "${CEF_RESOURCE_DIR}/"
            DESTINATION bin
            COMPONENT inviwo-module-webbrowser)
	# Copy helper executable into the bin directory (used by installer)
	install(TARGETS ${CEF_HELPER_TARGET} RUNTIME DESTINATION bin COMPONENT inviwo-module-webbrowser)
endif()





