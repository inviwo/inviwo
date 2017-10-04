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
ivw_retrieve_all_modules(enabled_modules)

#--------------------------------------------------------------------
# Remove Qt stuff from list
foreach(module ${enabled_modules})
    string(TOUPPER ${module} u_module)
    if(u_module MATCHES "QT+")
        list(REMOVE_ITEM enabled_modules ${module})
    endif()
endforeach()

#--------------------------------------------------------------------
# Determine dendendencies and set runtime module loading defines
ivw_configure_application_module_dependencies(inviwo-integrationtests
                                              "${enabled_modules}"
                                              dependencies)

# Create application
add_executable(inviwo-integrationtests MACOSX_BUNDLE WIN32 ${SOURCE_FILES})
ivw_register_use_of_modules(inviwo-integrationtests ${enabled_modules})
target_link_libraries(inviwo-integrationtests PRIVATE 
    inviwo::core
    inviwo::module::glfw
    inviwo::module::base
    gtest
)
ivw_add_module_dependencies(inviwo-integrationtests ${dependencies})
ivw_define_standard_definitions(inviwo-integrationtests inviwo-integrationtests)
ivw_define_standard_properties(inviwo-integrationtests)
ivw_memleak_setup(inviwo-integrationtests)

# Dependencies to build before this project when they are changed.
# Needed if modules are loaded at runtime since they should be built
# when this project is set as startup project
ivw_mod_name_to_target_name(target_names ${enabled_modules})
add_dependencies(inviwo-integrationtests ${target_names})

# Optimize compilation with pre-compilied headers based on inviwo-core
ivw_compile_optimize_inviwo_core_on_target(inviwo-integrationtests)
