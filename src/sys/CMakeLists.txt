project(inviwo-module-system)

add_library(inviwo-module-system)
add_library(inviwo::module-system ALIAS inviwo-module-system)

ivw_register_modules(ENABLED_MODULES_OUT enabled_modules)

# Generate module registration file
ivw_private_generate_module_registration_files(
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/moduleregistration.cpp
    MODULES ${enabled_modules})

target_sources(inviwo-module-system
    PUBLIC 
    FILE_SET HEADERS
    TYPE HEADERS
    BASE_DIRS ${IVW_INCLUDE_DIR}
    FILES 
        ${IVW_INCLUDE_DIR}/inviwo/sys/inviwosysdefine.h
        ${IVW_INCLUDE_DIR}/inviwo/sys/moduleregistration.h
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}/moduleregistration.cpp
)

ivw_define_standard_definitions(IVW_SYS inviwo-module-system)
ivw_define_standard_properties(inviwo-module-system)

target_link_libraries(inviwo-module-system PUBLIC 
    inviwo::core
)

if(IVW_CFG_RUNTIME_MODULE_LOADING)
    target_compile_definitions(${target} PUBLIC IVW_RUNTIME_MODULE_LOADING)
    # Dependencies to build before this project when they are changed.
    # Needed if modules are loaded at runtime since they should be built
    # when this project is set as startup project
    ivw_mod_name_to_alias(dep_targets ${enabled_modules})
    add_dependencies(inviwo-module-system ${dep_targets})
else()
    ivw_mod_name_to_alias(dep_targets ${enabled_modules})
    target_link_libraries(inviwo-module-system PRIVATE ${dep_targets})
endif()

ivw_install_helper(TARGET inviwo-module-system
    NAMESPACE inviwo 
    DESTINATION inviwo
    POSTCONFIG
        "
        if(NOT TARGET inviwo::module-system)
            add_library(inviwo::module-system ALIAS inviwo::inviwo-module-system)
        endif()
        "
)

# Save information for python tools.
ivw_mod_dep_to_mod_name(ivw_module_mod_names ${enabled_modules})
ivw_mod_name_to_class(ivw_module_classes ${ivw_module_mod_names})
ivw_private_create_pyconfig("${IVW_MODULE_DIR};${IVW_EXTERNAL_MODULES}" "${ivw_module_classes}" inviwo)
