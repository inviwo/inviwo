# Adapted settings from openexr/CMakeLists.txt and others, such as /openexr/OpenEXR/config/OpenEXRSetup.cmake

set(OPENEXR_BUILD_UTILS OFF)
set(INSTALL_DOCS OFF)
set(BUILD_TESTING OFF)
set(OPENEXR_INSTALL ON)
set(OPENEXR_BUILD_TOOLS OFF)
set(OPENEXR_INSTALL_TOOLS OFF)
set(OPENEXR_INSTALL_EXAMPLES OFF)
set(OPENEXR_FORCE_INTERNAL_IMATH ON)

add_subdirectory(openexr)

ivw_move_targets_in_dir_to_folder(${CMAKE_CURRENT_SOURCE_DIR} ext/openexr)
ivw_suppress_warnings_for_targets_in_dir(${CMAKE_CURRENT_SOURCE_DIR})
