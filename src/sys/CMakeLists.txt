project(inviwo-module-system)

add_library(inviwo-module-system)
add_library(inviwo::module-system ALIAS inviwo-module-system)

ivw_register_modules(
    ENABLED_MODULE_TARGETS_OUT enabled_modules_targets
    MODULE_REGISTRATION_FILE ${CMAKE_CURRENT_BINARY_DIR}/moduleregistration.cpp
)

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
    add_dependencies(inviwo-module-system ${enabled_modules_targets})
else()
    target_link_libraries(inviwo-module-system PRIVATE ${enabled_modules_targets})
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


install(
    FILES 
        ${IVW_ROOT_DIR}/cmake/compileoptions.cmake
        ${IVW_ROOT_DIR}/cmake/compileresources.cmake
        ${IVW_ROOT_DIR}/cmake/doc.cmake
        ${IVW_ROOT_DIR}/cmake/filegeneration.cmake
        ${IVW_ROOT_DIR}/cmake/globalutils.cmake
        ${IVW_ROOT_DIR}/cmake/installutils.cmake
        ${IVW_ROOT_DIR}/cmake/inviwocoredata.cmake
        ${IVW_ROOT_DIR}/cmake/inviwomoduledata.cmake
        ${IVW_ROOT_DIR}/cmake/inviwocreatemodule.cmake
        ${IVW_ROOT_DIR}/cmake/inviworegistermodules.cmake
        ${IVW_ROOT_DIR}/cmake/licenses.cmake
        ${IVW_ROOT_DIR}/cmake/precompileheaders.cmake
        ${IVW_ROOT_DIR}/cmake/pybind11.cmake
        ${IVW_ROOT_DIR}/cmake/unittests.cmake
        ${IVW_ROOT_DIR}/cmake/vcpkghelpers.cmake
    DESTINATION ${IVW_INSTALL_PREFIX}cmake
    COMPONENT Development
)

install(DIRECTORY ${IVW_ROOT_DIR}/cmake/templates
    DESTINATION ${IVW_INSTALL_PREFIX}cmake/
    COMPONENT Development
)
