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
list(APPEND package_list InviwoCore)

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
# Need to add dependent directories before creating application
ivw_add_dependency_directories(${package_list})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

#--------------------------------------------------------------------
# Create application
add_executable(inviwo-integrationtests MACOSX_BUNDLE WIN32 ${SOURCE_FILES})
target_link_libraries(inviwo-integrationtests gtest)

#--------------------------------------------------------------------
# Define defintions
ivw_define_standard_definitions(inviwo-integrationtests inviwo-integrationtests)

#--------------------------------------------------------------------
# Define standard properties
ivw_define_standard_properties(inviwo-integrationtests)

ivw_memleak_setup(inviwo-integrationtests)

#--------------------------------------------------------------------
# Move to folder
#ivw_folder(inviwo-integrationtests minimals)

#--------------------------------------------------------------------
# Add application to pack
if(IVW_PACKAGE_PROJECT)
if(WIN32)
    install(TARGETS inviwo-integrationtests
            RUNTIME DESTINATION bin
            COMPONENT integrationtest_app)
else()
    install(TARGETS inviwo-integrationtests
            RUNTIME DESTINATION bin
            BUNDLE DESTINATION bin
            ARCHIVE DESTINATION lib
            LIBRARY DESTINATION lib
            COMPONENT integrationtest_app)
endif()
endif()

#--------------------------------------------------------------------
# Add dependencies
ivw_add_dependencies(${package_list})

#--------------------------------------------------------------------
# Optimize compilation with pre-compilied headers based on inviwo-core
ivw_compile_optimize_inviwo_core()
