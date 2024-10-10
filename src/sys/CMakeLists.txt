project(inviwo-module-system)

add_library(inviwo-module-system)
add_library(inviwo::module-system ALIAS inviwo-module-system)

ivw_register_modules(
    MODULE_DIRS ${IVW_MODULE_DIR} ${IVW_EXTERNAL_MODULES}
    ENABLED_MODULE_TARGETS_OUT enabled_modules_targets
    ENABLED_MODULES_OUT enabled_modules
    MODULE_REGISTRATION_FILE ${CMAKE_CURRENT_BINARY_DIR}/moduleregistration.cpp
)
source_group("CMake Files" FILES CMakeLists.txt)

target_sources(inviwo-module-system
    PUBLIC 
    FILE_SET HEADERS
    TYPE HEADERS
    BASE_DIRS ${IVW_INCLUDE_DIR}
    FILES 
        ${IVW_INCLUDE_DIR}/inviwo/sys/inviwosysdefine.h
        ${IVW_INCLUDE_DIR}/inviwo/sys/moduleregistration.h
        ${IVW_INCLUDE_DIR}/inviwo/sys/moduleloading.h
    PRIVATE
        moduleloading.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/moduleregistration.cpp
        CMakeLists.txt
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
    target_link_libraries(inviwo-module-system PUBLIC ${enabled_modules_targets})
endif()

ivw_install_helper(TARGET inviwo-module-system
    NAMESPACE inviwo
    DESTINATION inviwo
    ALIAS module-system
)

# Save information for python tools. used for regression
ivw_mod_dep_to_mod_name(ivw_module_mod_names ${enabled_modules})
ivw_mod_name_to_class(ivw_module_classes ${ivw_module_mod_names})
ivw_private_create_pyconfig(
    MODULE_DIRS ${IVW_MODULE_DIR} ${IVW_EXTERNAL_MODULES}
    ENABLED_MODULES ${ivw_module_classes}
)
