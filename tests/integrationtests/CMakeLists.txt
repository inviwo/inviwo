#--------------------------------------------------------------------
# Inviwo GLFW Application
ivw_project(inviwo-integrationtests)

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    inviwo-integrationtests.cpp
    image-test.cpp
    inviwoapplication-test.cpp
    network-test.cpp
    processorcreation-test.cpp
    volume-test.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})
        
#--------------------------------------------------------------------
# Define libraries that should be linked
ivw_retrieve_all_modules(package_list)
# Core dependencies are linked and will never be loaded at runtime
set(core_dependencies InviwoCore)
list(APPEND core_dependencies InviwoBaseModule)
list(APPEND core_dependencies InviwoGLFWModule)
list(APPEND package_list ${core_dependencies})

#--------------------------------------------------------------------
# Remove Qt stuff from list
foreach(module ${package_list})
    string(TOUPPER ${module} u_module)
    if(u_module MATCHES "QT+")
        list(REMOVE_ITEM package_list ${module})
    endif()
endforeach()

#--------------------------------------------------------------------
# Register the use of modules
ivw_register_use_of_modules(${package_list})

#--------------------------------------------------------------------
# Determine dendendencies and set runtime module loading defines
set(dependencies "")
ivw_configure_application_module_dependencies("inviwo-integrationtests"
                                              "${core_dependencies}"
                                              "${package_list}"
                                              dependencies)

#--------------------------------------------------------------------
# Need to add dependent directories before creating application
ivw_add_dependency_directories(${dependencies})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

#--------------------------------------------------------------------
# Create application
add_executable(inviwo-integrationtests MACOSX_BUNDLE WIN32 ${SOURCE_FILES})
target_link_libraries(inviwo-integrationtests PRIVATE gtest)

#--------------------------------------------------------------------
# Define defintions
ivw_define_standard_definitions(inviwo-integrationtests inviwo-integrationtests)

#--------------------------------------------------------------------
# Define standard properties
ivw_define_standard_properties(inviwo-integrationtests)

ivw_memleak_setup(inviwo-integrationtests)

#--------------------------------------------------------------------
# Add dependencies
ivw_add_dependencies(${dependencies})

#--------------------------------------------------------------------
# Optimize compilation with pre-compilied headers based on inviwo-core
ivw_compile_optimize_inviwo_core()
