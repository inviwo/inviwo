option(GLFW_BUILD_EXAMPLES "Build the GLFW example programs" OFF)
option(GLFW_BUILD_TESTS "Build the GLFW test programs" OFF)
option(GLFW_BUILD_DOCS "Build the GLFW documentation" OFF)
option(GLFW_INSTALL "Generate installation target" OFF)
option(GLFW_DOCUMENT_INTERNALS "Include internals in documentation" OFF)

add_subdirectory(glfw)
ivw_folder(glfw ext)

ivw_get_targets_in_dir_recursive(targets ${CMAKE_CURRENT_SOURCE_DIR})
ivw_suppress_compiler_warnings(${targets})
ivw_default_install_targets(${targets})
