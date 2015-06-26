#--------------------------------------------------------------------
# Inviwo Unittest Application
ivw_project(unittests)

add_definitions(-DGTEST_HAS_TR1_TUPLE=0)


#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${CMAKE_SOURCE_DIR}/apps/unittests/inviwounittest.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

if(NOT IVW_MODULE_GLFW)
    set(IVW_MODULE_GLFW ON CACHE BOOL "" FORCE)
    ivw_message("IVW_MODULE_GLFW was set to build, due to dependency towards IVW_UNIT_TEST_APPLICATION")
endif()


if(NOT IVW_MODULE_UNITTESTS)
    set(IVW_MODULE_UNITTESTS ON CACHE BOOL "" FORCE)
    ivw_message("IVW_MODULE_UNITTESTS was set to build, due to dependency towards IVW_UNIT_TEST_APPLICATION")
endif()

#--------------------------------------------------------------------
# Define defintions
ivw_define_standard_definitions(unittests)

#--------------------------------------------------------------------
# Define libraries that should be linked
ivw_retrieve_all_modules(package_list)
list(APPEND package_list InviwoCore)

foreach(module ${package_list})
    string(TOUPPER ${module} u_module)
    if(u_module MATCHES "QT+")
        list(REMOVE_ITEM package_list ${module})
    endif()
    if(u_module MATCHES "VRN+")
        list(REMOVE_ITEM package_list ${module})
    endif()
endforeach()
list(REMOVE_ITEM package_list InviwoGLUTModule)

#--------------------------------------------------------------------
# Register the use of modules
ivw_register_use_of_modules(${package_list})

#--------------------------------------------------------------------
# Need to add dependent directories before creating application
ivw_add_dependency_directories(${package_list})

#--------------------------------------------------------------------
# Include generation directory to register modules etc
#include_directories(${IVW_MODULE_DIR}/unittests/ext/gtest-1.7.0/include)
include_directories(${CMAKE_BINARY_DIR}/modules/_generated)
ivw_link_directories(${LIBRARY_OUTPUT_PATH})


#--------------------------------------------------------------------
# Create application
add_executable(unittests MACOSX_BUNDLE WIN32 ${SOURCE_FILES})

#--------------------------------------------------------------------
# Define standard properties
ivw_define_standard_properties(unittests)

#--------------------------------------------------------------------
# Add dependencies
ivw_add_dependencies(${package_list})

#--------------------------------------------------------------------
# Optimize compilation with pre-compilied headers based on inviwo-core
ivw_compile_optimize_inviwo_core()


if(IVW_ENABLE_MSVC_MEMLEAK_TEST)
    IF(WIN32)
        IF(MSVC)
            if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
                add_custom_command(TARGET unittests POST_BUILD 
                                    COMMAND ${CMAKE_COMMAND}  -E copy_if_different 
                                        ${IVW_EXTENSIONS_DIR}/vld/bin/Win64/vld_x64.dll 
                                        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$(ConfigurationName)/vld_x64.dll)    
                                        
                add_custom_command(TARGET unittests POST_BUILD 
                                    COMMAND ${CMAKE_COMMAND}  -E copy_if_different 
                                        ${IVW_EXTENSIONS_DIR}/vld/bin/Win64/dbghelp.dll 
                                        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$(ConfigurationName)/dbghelp.dll)    
                                        
                add_custom_command(TARGET unittests POST_BUILD 
                                    COMMAND ${CMAKE_COMMAND}  -E copy_if_different 
                                        ${IVW_EXTENSIONS_DIR}/vld/bin/Win64/Microsoft.DTfW.DHL.manifest 
                                        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$(ConfigurationName)/Microsoft.DTfW.DHL.manifest)    
            else ()
                add_custom_command(TARGET unittests POST_BUILD 
                                    COMMAND ${CMAKE_COMMAND}  -E copy_if_different 
                                        ${IVW_EXTENSIONS_DIR}/vld/bin/Win32/vld_x86.dll 
                                        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$(ConfigurationName)/vld_x86.dll)    
                                        
                add_custom_command(TARGET unittests POST_BUILD 
                                    COMMAND ${CMAKE_COMMAND}  -E copy_if_different 
                                        ${IVW_EXTENSIONS_DIR}/vld/bin/Win32/dbghelp.dll 
                                        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$(ConfigurationName)/dbghelp.dll)    
                                        
                add_custom_command(TARGET unittests POST_BUILD 
                                    COMMAND ${CMAKE_COMMAND}  -E copy_if_different 
                                        ${IVW_EXTENSIONS_DIR}/vld/bin/Win32/Microsoft.DTfW.DHL.manifest 
                                        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$(ConfigurationName)/Microsoft.DTfW.DHL.manifest)    
            endif()
        ENDIF(MSVC)
    ENDIF(WIN32)
endif()
