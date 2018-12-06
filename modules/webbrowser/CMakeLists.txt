# Web browser module based on CEF

# CEF configuration.
# See https://bitbucket.org/chromiumembedded/cef-project

# Specify the CEF distribution version.
# http://opensource.spotify.com/cefbuilds/index.html
# Test before upgrading version since they are sometimes unstable:
# 3.3202.1680.g700054b Problems with event handling of html-range elements (sliders)
# 3.3359.1774.gd49d25f Crashes when creating browsers
# 3.3325.1758.g9aea513 Crashes on scroll event (Mac)
# 3.3396.1775.g5340bb0 Crashes when switching web page
# 3.3396.1775.g5340bb0 Crashes when using two fingers on trackpad (Mac)
set(CEF_VERSION "3.3538.1849.g458cc98")

# Determine the platform.
include("cmake/cef_platform.cmake")
determineCEFPlatform(CEF_PLATFORM)

# Download and extract the CEF binary distribution (executes cmake/DownloadCEF.cmake).
include("cmake/DownloadCEF.cmake")
DownloadCEF("${CEF_PLATFORM}" "${CEF_VERSION}" "${PROJECT_BINARY_DIR}/ext/cef")

# Add the CEF binary distribution's cmake/ directory to the module path.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CEF_ROOT}/cmake")

# sandbox lib is built with MT_StaticRelease but Inviwo is built with 
# MD_DynamicRelease and hence are not compatible
if (WIN32)
    option(USE_SANDBOX "Enable or disable use of the sandbox." OFF)
endif()

# Load the CEF configuration.
# Execute FindCEF.cmake which must exist in CMAKE_MODULE_PATH.
find_package(CEF REQUIRED)

# Inviwo: libcef_dll_wrapper must use same compiler settings to be able to link.
set(CEF_COMPILER_FLAGS ${CMAKE_CXX_FLAGS})
set(CEF_COMPILER_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
set(CEF_COMPILER_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
set(CEF_SHARED_LINKER_FLAGS_DEBUG ${CMAKE_SHARED_LINKER_FLAGS_DEBUG})
set(CEF_SHARED_LINKER_FLAGS_RELEASE ${CMAKE_SHARED_LINKER_FLAGS_RELASE}})

# Inviwo: Split space separated flags into a list (fixes mac compile errors)
separate_arguments(CEF_COMPILER_FLAGS_RELEASE)

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

# Display configuration settings.
#PRINT_CEF_CONFIG()

#--------------------------------------------------------------------
# Inviwo WebBrowser Module
ivw_module(WebBrowser)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    include/modules/webbrowser/cefimageconverter.h
    include/modules/webbrowser/interaction/cefinteractionhandler.h
    include/modules/webbrowser/interaction/cefkeyboardmapping.h
    include/modules/webbrowser/processors/propertysyncexampleprocessor.h
    include/modules/webbrowser/processors/webbrowserprocessor.h
    include/modules/webbrowser/properties/boolpropertywidgetcef.h
    include/modules/webbrowser/properties/buttonpropertywidgetcef.h
    include/modules/webbrowser/properties/ordinalpropertywidgetcef.h
    include/modules/webbrowser/properties/propertycefsynchronizer.h
    include/modules/webbrowser/properties/propertywidgetcef.h
    include/modules/webbrowser/properties/stringpropertywidgetcef.h
    include/modules/webbrowser/properties/templatepropertywidgetcef.h
    include/modules/webbrowser/renderhandlergl.h
    include/modules/webbrowser/webbrowserapp.h
    include/modules/webbrowser/webbrowserclient.h
    include/modules/webbrowser/webbrowsermodule.h
    include/modules/webbrowser/webbrowsermoduledefine.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    src/cefimageconverter.cpp
    src/interaction/cefinteractionhandler.cpp
    src/interaction/cefkeyboardmapping.cpp
    src/processors/propertysyncexampleprocessor.cpp
    src/processors/webbrowserprocessor.cpp
    src/properties/boolpropertywidgetcef.cpp
    src/properties/buttonpropertywidgetcef.cpp
    src/properties/ordinalpropertywidgetcef.cpp
    src/properties/propertycefsynchronizer.cpp
    src/properties/propertywidgetcef.cpp
    src/properties/stringpropertywidgetcef.cpp
    src/renderhandlergl.cpp
    src/webbrowserapp.cpp
    src/webbrowserclient.cpp
    src/webbrowsermodule.cpp
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
target_link_libraries(inviwo-module-webbrowser PUBLIC
    $<$<NOT:$<BOOL:${OS_MACOSX}>>:libcef_lib>
    libcef_dll_wrapper 
    ${CEF_STANDARD_LIBS}
)

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

if (OS_MACOSX)
    # Copy the CEF framework into the Frameworks directory (used by installer)
    install(DIRECTORY "${CEF_BINARY_DIR}/Chromium Embedded Framework.framework"
            DESTINATION "Inviwo.app/Contents/Frameworks/Chromium Embedded Framework.framework"
            COMPONENT inviwo-module-webbrowser) 
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
            DESTINATION bin
            COMPONENT inviwo-module-webbrowser)

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
            DESTINATION bin
            COMPONENT inviwo-module-webbrowser)
endif()

# Setup CEF helper executable. Started from the web browser module as a sub-process.
add_subdirectory(webhelper)



