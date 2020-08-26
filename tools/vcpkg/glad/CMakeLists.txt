cmake_minimum_required(VERSION 3.12)

project(GLAD VERSION 0.1.33 LANGUAGES C)

# Options
option(GLAD_INSTALL "Generate installation target" OFF)
if (MSVC)
    option(USE_MSVC_RUNTIME_LIBRARY_DLL "Use MSVC runtime library DLL" ON)
endif()

file(GLOB glad_headers include/*)
file(GLOB glad_src src/*)

add_library(glad ${glad_headers} ${glad_src})
target_include_directories(glad 
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)


# Are we building a shared library? If so, define the macro GLAD_API_EXPORT on the command line.
get_target_property(library_type glad TYPE)
if(library_type STREQUAL SHARED_LIBRARY)
    target_compile_definitions(glad PUBLIC GLAD_GLAPI_EXPORT PRIVATE GLAD_GLAPI_EXPORT_BUILD)
endif()

if(MSVC AND NOT USE_MSVC_RUNTIME_LIBRARY_DLL)
    foreach(flag CMAKE_C_FLAGS
                CMAKE_C_FLAGS_DEBUG
                CMAKE_C_FLAGS_RELEASE
                CMAKE_C_FLAGS_MINSIZEREL
                CMAKE_C_FLAGS_RELWITHDEBINFO)
        if (${flag} MATCHES "/MD")
            string(REGEX REPLACE "/MD" "/MT" ${flag} "${${flag}}")
        endif()
        if (${flag} MATCHES "/MDd")
            string(REGEX REPLACE "/MDd" "/MTd" ${flag} "${${flag}}")
        endif()
    endforeach()
endif()

# Install
if(GLAD_INSTALL)
    set(version_config "${CMAKE_CURRENT_BINARY_DIR}/gladConfigVersion.cmake")
    set(project_config "${CMAKE_CURRENT_BINARY_DIR}/gladConfig.cmake")
    set(targets_export_name "gladTargets")

    # Include module with fuction 'write_basic_package_version_file'
    include(CMakePackageConfigHelpers)

    # Configure 'gladConfigVersion.cmake'
    # PROJECT_VERSION is used as a VERSION
    write_basic_package_version_file(gladConfigVersion.cmake COMPATIBILITY SameMajorVersion)

    # Configure 'gladConfig.cmake'
    # Uses targets_export_name variable.
    configure_package_config_file(Config.cmake.in gladConfig.cmake INSTALL_DESTINATION lib/cmake/glad)

    install(TARGETS glad EXPORT gladTargets LIBRARY ARCHIVE RUNTIME)

    install(DIRECTORY include/ DESTINATION include)

    # Install gladConfig.cmake, gladConfigVersion.cmake
    install(
    FILES 
        "${CMAKE_CURRENT_BINARY_DIR}/gladConfig.cmake" 
        "${CMAKE_CURRENT_BINARY_DIR}/gladConfigVersion.cmake" 
    DESTINATION 
        lib/cmake/glad
    )

    # Create and install gladTargets.cmake
    install(EXPORT gladTargets NAMESPACE glad:: DESTINATION lib/cmake/glad)
endif()