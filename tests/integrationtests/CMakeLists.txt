#--------------------------------------------------------------------
# Inviwo GLFW Application
project(inviwo-integrationtests)

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

#--------------------------------------------------------------------
# Remove Qt stuff from list
foreach(module ${package_list})
    string(TOUPPER ${module} u_module)
    if(u_module MATCHES "QT+")
        list(REMOVE_ITEM package_list ${module})
    endif()
endforeach()

#--------------------------------------------------------------------
# Create application
add_executable(inviwo-integrationtests MACOSX_BUNDLE WIN32 ${SOURCE_FILES})
ivw_register_use_of_modules(inviwo-integrationtests ${package_list})
target_link_libraries(inviwo-integrationtests PRIVATE inviwo::core gtest)
ivw_add_module_dependencies(inviwo-integrationtests ${package_list})
ivw_define_standard_definitions(inviwo-integrationtests inviwo-integrationtests)
ivw_define_standard_properties(inviwo-integrationtests)
ivw_memleak_setup(inviwo-integrationtests)

#--------------------------------------------------------------------
# Optimize compilation with pre-compilied headers based on inviwo-core
ivw_compile_optimize_inviwo_core_on_target(inviwo-integrationtests)
