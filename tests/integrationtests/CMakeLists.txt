#--------------------------------------------------------------------
# Inviwo integration tests
if(NOT IVW_INTEGRATION_TESTS)
    return()
endif()

project(inviwo-integrationtests)

# Add source files
set(SOURCE_FILES
    inviwo-integrationtests.cpp
    image-test.cpp
    inviwoapplication-test.cpp
    network-test.cpp
    processorcreation-test.cpp
    propertycreation-test.cpp
    volume-test.cpp
    shader-test.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

ivw_retrieve_all_modules(enabled_modules)
# Remove Qt stuff from list
foreach(module ${enabled_modules})
    string(TOUPPER ${module} u_module)
    if(u_module MATCHES "QT+")
        list(REMOVE_ITEM enabled_modules ${module})
    endif()
endforeach()

# Create application
add_executable(inviwo-integrationtests ${SOURCE_FILES})
target_link_libraries(inviwo-integrationtests PRIVATE 
    inviwo::core
    inviwo::module::glfw
    inviwo::module::base
    inviwo::testutil
    gtest
)
ivw_configure_application_module_dependencies(inviwo-integrationtests ${enabled_modules})
ivw_define_standard_definitions(inviwo-integrationtests inviwo-integrationtests)
ivw_define_standard_properties(inviwo-integrationtests)
ivw_memleak_setup(inviwo-integrationtests)
