# Web browser module based on CEF

# CEF configuration.
# See https://bitbucket.org/chromiumembedded/cef-project

# Specify the CEF distribution version.
# https://cef-builds.spotifycdn.com/index.html
# Test before upgrading version since they are sometimes unstable:
# 3.3202.1680.g700054b Problems with event handling of html-range elements (sliders)
# 3.3359.1774.gd49d25f Crashes when creating browsers
# 3.3325.1758.g9aea513 Crashes on scroll event (Mac)
# 3.3396.1775.g5340bb0 Crashes when switching web page
# 3.3396.1775.g5340bb0 Crashes when using two fingers on trackpad (Mac)
# 3.3538.1849.g458cc98
# 75.0.7+g19229b6+chromium-75.0.3770.80 Rendering stops working, https://bitbucket.org/chromiumembedded/cef/issues/2618/onacceleratedpaint-is-not-called-with-off#comment-51661853
# FATAL:async_layer_tree_frame_sink.cc
# Known issue: Dropdown does not work.
# Probably due to https://magpcss.org/ceforum/viewtopic.php?f=6&t=11618
set(CEF_VERSION "111.2.4+g49cae37+chromium-111.0.5563.65")

# Determine the platform.
include("cmake/cef_platform.cmake")
determineCEFPlatform(CEF_PLATFORM)

# Download and extract the CEF binary distribution (executes cmake/DownloadCEF.cmake).
include("cmake/DownloadCEF.cmake")
DownloadCEF("${CEF_PLATFORM}" "${CEF_VERSION}" "${IVW_BINARY_DIR}/ext/cef")

# Add the CEF binary distribution's cmake/ directory to the module path.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CEF_ROOT}/cmake")

# sandbox lib is built with MT_StaticRelease but Inviwo is built with 
# MD_DynamicRelease and hence are not compatible
if(WIN32)
    option(USE_SANDBOX "Enable or disable use of the sandbox." OFF)
endif()

if(NOT PROJECT_ARCH)
    set(PROJECT_ARCH ${CMAKE_SYSTEM_PROCESSOR}) # CEF Wants this on OS X.
endif()

# Save the original value of CMAKE_OSX_DEPLOYMENT_TARGET since FindCEF sets it to a very old 10.13
set(ORG_CMAKE_OSX_DEPLOYMENT_TARGET ${CMAKE_OSX_DEPLOYMENT_TARGET})

# Inviwo: make sure we use the same c runtime library linkage as Inviwo
if(WIN32 AND MSVC)
    if(BUILD_SHARED_LIBS OR IVW_CFG_MSVC_FORCE_SHARED_CRT)
        set(CEF_RUNTIME_LIBRARY_FLAG "/MD" CACHE STRING "" FORCE)
    endif()
endif()

# Load the CEF configuration.
# Execute FindCEF.cmake which must exist in CMAKE_MODULE_PATH.
find_package(CEF REQUIRED)

set(CMAKE_OSX_DEPLOYMENT_TARGET ${ORG_CMAKE_OSX_DEPLOYMENT_TARGET})

# Include the libcef_dll_wrapper target.
# Comes from the libcef_dll/CMakeLists.txt file in the binary distribution
# directory.
# Inviwo: Linker errors if libcef_dll_wrapper is not built statically
set(BUILD_SHARED_LIBS_TMP ${BUILD_SHARED_LIBS})
set(BUILD_SHARED_LIBS OFF)
add_subdirectory(${CEF_LIBCEF_DLL_WRAPPER_PATH} libcef_dll_wrapper)
# Group in folder 

ivw_install_helper(TARGET libcef_dll_wrapper
    NAMESPACE cef
    DESTINATION cef
)

ivw_folder(libcef_dll_wrapper ext/CEF)
# Inviwo: Remove warnings (-w), and build error due to 'OSAtomicCompareAndSwap32' is deprecated
if(CMAKE_COMPILER_IS_GNUCXX OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    target_compile_options(libcef_dll_wrapper PRIVATE -Wno-deprecated-declarations -w)
endif()
# Logical target used to link the libcef library.
ADD_LOGICAL_TARGET("libcef_lib" "${CEF_LIB_DEBUG}" "${CEF_LIB_RELEASE}")
set(BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS_TMP})

# Determine the target output directory.
# Inviwo: use EXECUTABLE_OUTPUT_PATH directory as output dir
if(IVW_GENERATOR_IS_MULTI_CONFIG) 
    set(CEF_TARGET_OUT_DIR ${EXECUTABLE_OUTPUT_PATH}/$<CONFIG>)
else()
    set(CEF_TARGET_OUT_DIR ${EXECUTABLE_OUTPUT_PATH})
endif()

# Display configuration settings.
#PRINT_CEF_CONFIG()

# Inviwo WebBrowser Module
ivw_module(WebBrowser)

# Add header files
set(HEADER_FILES
    include/modules/webbrowser/cefimageconverter.h
    include/modules/webbrowser/interaction/cefinteractionhandler.h
    include/modules/webbrowser/interaction/cefkeyboardmapping.h
    include/modules/webbrowser/processors/processorcefsynchronizer.h
    include/modules/webbrowser/processors/progressbarobservercef.h
    include/modules/webbrowser/processors/webbrowserprocessor.h
    include/modules/webbrowser/properties/propertycefsynchronizer.h
    include/modules/webbrowser/properties/propertywidgetcef.h
    include/modules/webbrowser/properties/propertywidgetceffactory.h
    include/modules/webbrowser/properties/propertywidgetceffactoryobject.h
    include/modules/webbrowser/renderhandlergl.h
    include/modules/webbrowser/webbrowserapp.h
    include/modules/webbrowser/webbrowserclient.h
    include/modules/webbrowser/webbrowsermodule.h
    include/modules/webbrowser/webbrowsermoduledefine.h
    include/modules/webbrowser/webbrowsersettings.h
    include/modules/webbrowser/webbrowserutil.h
)
ivw_group("Header Files" ${HEADER_FILES})

# Add source files
set(SOURCE_FILES
    src/cefimageconverter.cpp
    src/interaction/cefinteractionhandler.cpp
    src/interaction/cefkeyboardmapping.cpp
    src/processors/processorcefsynchronizer.cpp
    src/processors/progressbarobservercef.cpp
    src/processors/webbrowserprocessor.cpp
    src/properties/propertycefsynchronizer.cpp
    src/properties/propertywidgetcef.cpp
    src/properties/propertywidgetceffactory.cpp
    src/properties/propertywidgetceffactoryobject.cpp
    src/renderhandlergl.cpp
    src/webbrowserapp.cpp
    src/webbrowserclient.cpp
    src/webbrowsermodule.cpp
    src/webbrowsersettings.cpp
    src/webbrowserutil.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

# Add shaders
set(SHADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/img_convert_cef.frag
)
ivw_group("Shader Files" ${SHADER_FILES})

# Add javascript
set(JS_FILES
    data/js/inviwoapi.js
)
ivw_group("Javascript Files" BASE "${CMAKE_CURRENT_SOURCE_DIR}/data/js" ${JS_FILES})

# Add Unittests
set(TEST_FILES
    #${CMAKE_CURRENT_SOURCE_DIR}/tests/unittests/webbrowser-test.cpp
)
ivw_add_unittest(${TEST_FILES})

# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES} ${JS_FILES})
target_include_directories(inviwo-module-webbrowser
    PUBLIC
        $<BUILD_INTERFACE:${CEF_INCLUDE_PATH}>
        $<INSTALL_INTERFACE:include>
)

find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(inviwo-module-webbrowser PUBLIC
    $<$<NOT:$<BOOL:${OS_MAC}>>:libcef_lib>
    libcef_dll_wrapper 
    nlohmann_json::nlohmann_json
    ${CEF_STANDARD_LIBS}
)
target_compile_definitions(inviwo-module-webbrowser PRIVATE
    $<$<BOOL:${USE_SANDBOX}>:CEF_USE_SANDBOX>
)

# Package or build shaders into resources
ivw_handle_shader_resources(${CMAKE_CURRENT_SOURCE_DIR}/glsl ${SHADER_FILES})

ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/data/js)

find_package(utf8cpp CONFIG REQUIRED)
target_link_libraries(inviwo-module-webbrowser PRIVATE
    utf8cpp
)

ivw_register_license_file(NAME "Chromium Embedded Framework" VERSION ${CEF_VERSION} MODULE WebBrowser
    URL "https://bitbucket.org/chromiumembedded/cef"
    TYPE "BSD-license"
    FILES ${CEF_ROOT}/LICENSE.txt
)

# Inviwo: CEF only provides release/debug configurations. Set RelWithDebInfo and MinSizeRel to use release 
set(CEF_BINARY_DIR 
    "${_CEF_ROOT}/$<$<CONFIG:RelWithDebInfo,MinSizeRel,Release>:Release>$<$<CONFIG:Debug>:Debug>"
)

if (OS_MAC)
	# Output path for the main app bundle.
	set(CEF_APP "${CEF_TARGET_OUT_DIR}/${IVW_APP_INSTALL_NAME}.app")
	# Copy the CEF framework into the Frameworks directory
    add_custom_command(
        TARGET inviwo-module-webbrowser
        POST_BUILD
        # Copy the CEF framework into the Frameworks directory.
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CEF_BINARY_DIR}/Chromium Embedded Framework.framework"
        "${CEF_APP}/Contents/Frameworks/Chromium Embedded Framework.framework"
        VERBATIM
    )
    
    # Application does not seem to find the framework resource files, so
    # find and create links to all framework resources from the bundle
    # Contents/Resource directory.
    # See documentation about settings in cef_types.h:
    # "This value [locales_dir_path] is ignored on MacOS where pack files are always
    # loaded from the app bundle Resources directory"
    # The files are the same for debug/release, simply use release
    # (file does not support generator expressions).
    file(GLOB resource_files 
        LIST_DIRECTORIES TRUE 
        "${_CEF_ROOT}/Release/Chromium Embedded Framework.framework/Resources/*"
    )
    foreach(cur_file IN LISTS resource_files)
        get_filename_component(name ${cur_file} NAME)
        add_custom_command(
            TARGET inviwo-module-webbrowser
            POST_BUILD
            WORKING_DIRECTORY "${CEF_APP}/Contents/Resources"
            # Copy the CEF framework into the Frameworks directory.
            COMMAND ${CMAKE_COMMAND} -E create_symlink
            "../Frameworks/Chromium Embedded Framework.framework/Resources/${name}"
            "${name}"
            VERBATIM
        )
    endforeach()

    # Copy the CEF framework into the Frameworks directory (used by installer)
    # TODO: Investigate if the .proj workaround above is required for packaging as well.
    install(DIRECTORY "${CEF_BINARY_DIR}/Chromium Embedded Framework.framework"
            DESTINATION "${IVW_APP_INSTALL_NAME}.app/Contents/Frameworks/Chromium Embedded Framework.framework"
            COMPONENT Application) 
elseif (WIN32)
    if(USE_SANDBOX)
        # Logical target used to link the cef_sandbox library.
        ADD_LOGICAL_TARGET("cef_sandbox_lib" "${CEF_SANDBOX_LIB_DEBUG}" "${CEF_SANDBOX_LIB_RELEASE}")
        target_link_libraries(inviwo-module-webbrowser PRIVATE cef_sandbox_lib ${CEF_SANDBOX_STANDARD_LIBS})
    endif()

    # Copy CEF binary and resource files to the target output directory.
    COPY_FILES("inviwo-module-webbrowser" "${CEF_BINARY_FILES}" "${CEF_BINARY_DIR}" "${CEF_TARGET_OUT_DIR}")
    COPY_FILES("inviwo-module-webbrowser" "${CEF_RESOURCE_FILES}" "${CEF_RESOURCE_DIR}" "${CEF_TARGET_OUT_DIR}")

    # Copy files into the bin directory (used by installer)
    install(DIRECTORY "${CEF_BINARY_DIR}/" # Note importance of / in the end to not put files into subfolder
            "${CEF_RESOURCE_DIR}/"
            DESTINATION "${IVW_RUNTIME_INSTALL_DIR}"
            COMPONENT Application)

else () # Linux
    # Set rpath so that libraries can be placed next to the executable.
    set_target_properties(inviwo-module-webbrowser PROPERTIES 
        INSTALL_RPATH "$ORIGIN"
        BUILD_WITH_INSTALL_RPATH TRUE
        RUNTIME_OUTPUT_DIRECTORY ${CEF_TARGET_OUT_DIR}
    )
    # Copy CEF binary and resource files to the target output directory.
    COPY_FILES("inviwo-module-webbrowser" "${CEF_BINARY_FILES}" "${CEF_BINARY_DIR}" "${CEF_TARGET_OUT_DIR}")
    COPY_FILES("inviwo-module-webbrowser" "${CEF_RESOURCE_FILES}" "${CEF_RESOURCE_DIR}" "${CEF_TARGET_OUT_DIR}")

    # Set SUID permissions on the chrome-sandbox target.
    SET_LINUX_SUID_PERMISSIONS("inviwo-module-webbrowser" "${CEF_TARGET_OUT_DIR}/chrome-sandbox")

    # Copy files into the bin directory (used by installer)
    install(DIRECTORY "${CEF_BINARY_DIR}/" # Note importance of / in the end to not put files into subfolder
            "${CEF_RESOURCE_DIR}/"
            DESTINATION "${IVW_RUNTIME_INSTALL_DIR}"
            COMPONENT Application)
endif()

# Setup CEF helper executable. Started from the web browser module as a sub-process.
add_subdirectory(webhelper)



