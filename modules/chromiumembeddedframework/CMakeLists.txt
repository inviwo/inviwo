#--------------------------------------------------------------------
# Inviwo ChromiumEmbeddedFramework Module
ivw_module(ChromiumEmbeddedFramework)

#
# CEF_ROOT setup.
# This variable must be set to locate the binary distribution.
# Example 2: The binary distribution is in a separate directory from your
#            project. Locate the binary distribution using the CEF_ROOT CMake
#            variable.
set(CEF_ROOT "E:/src/cef_binary_3.2883.1539.gd7f087e_windows64")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CEF_ROOT}/cmake")

#
# Load the CEF configuration.
#
# Execute FindCEF.cmake which must exist in CMAKE_MODULE_PATH.
find_package(CEF REQUIRED)
set(CEF_COMPILER_FLAGS ${CMAKE_CXX_FLAGS})
set(CEF_COMPILER_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
set(CEF_COMPILER_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
#ivw_message(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
# Include the libcef_dll_wrapper target.
# Comes from the libcef_dll/CMakeLists.txt file in the binary distribution
# directory.
set(CEF_SHARED_LINKER_FLAGS_DEBUG ${CMAKE_SHARED_LINKER_FLAGS_DEBUG})
set(CEF_SHARED_LINKER_FLAGS_RELEASE ${CMAKE_SHARED_LINKER_FLAGS_RELASE}})
#
list(REMOVE_ITEM CEF_COMPILER_DEFINES
    UNICODE _UNICODE
    )
#set(CEF_COMPILER_FLAGS CMAKE_COMPILER_FLAGS)
# Use flags from Inviwo. Treating level 4 warnings as errors will 
# prevent Inviwo from compiling
# /GR- produces Warning	C4541 'dynamic_cast' used on polymorphic type 
list(REMOVE_ITEM CEF_COMPILER_FLAGS
    /Gy           # Enable function-level linking
    /W4           # Warning level 4
    /WX           # Treat warnings as errors
    /GR-          # Disable run-time type information
    )
add_subdirectory(${CEF_LIBCEF_DLL_WRAPPER_PATH} libcef_dll_wrapper)

# Logical target used to link the libcef library.
ADD_LOGICAL_TARGET("libcef_lib" "${CEF_LIB_DEBUG}" "${CEF_LIB_RELEASE}")
# Display configuration settings.
PRINT_CEF_CONFIG()
ivw_message(CEF_STANDARD_LIBS ${CEF_STANDARD_LIBS})

ivw_message(CEF_LIBTYPE ${CEF_LIBTYPE})
ivw_message(CEF_LIB_DEBUG ${CEF_LIB_DEBUG})
ivw_message(CEF_LIB_RELEASE ${CEF_LIB_RELEASE})

ivw_include_directories(${CEF_INCLUDE_PATH})
add_dependency_libs_to_module(${CEF_STANDARD_LIBS} libcef_lib libcef_dll_wrapper)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    #${CMAKE_CURRENT_SOURCE_DIR}/chromiumembeddedframeworkprocessor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/chromiumprocessor.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    #${CMAKE_CURRENT_SOURCE_DIR}/chromiumembeddedframeworkprocessor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/chromiumprocessor.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})


#--------------------------------------------------------------------
# Add shaders
set(SHADER_FILES
    #${CMAKE_CURRENT_SOURCE_DIR}/glsl/chromiumembeddedframeworkprocessor.frag
)
ivw_group("Shader Files" ${SHADER_FILES})


#--------------------------------------------------------------------
# Add Unittests
set(TEST_FILES
	#${CMAKE_CURRENT_SOURCE_DIR}/tests/unittests/chromiumembeddedframework-test.cpp
)
ivw_add_unittest(${TEST_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

#--------------------------------------------------------------------
# Add shader directory to pack
# ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/glsl)
# Determine the target output directory.
SET_CEF_TARGET_OUT_DIR()
# Copy CEF binary and resource files to the target output directory.
COPY_FILES("inviwo-module-chromiumembeddedframework" "${CEF_BINARY_FILES}" "${CEF_BINARY_DIR}" "${CEF_TARGET_OUT_DIR}")
COPY_FILES("inviwo-module-chromiumembeddedframework" "${CEF_RESOURCE_FILES}" "${CEF_RESOURCE_DIR}" "${CEF_TARGET_OUT_DIR}")