# Use our own packaging/install locations
set(SKIP_INSTALL_ALL TRUE)

add_subdirectory(freetype)
ivw_get_targets_in_dir_recursive(targets ${CMAKE_CURRENT_SOURCE_DIR})
ivw_suppress_compiler_warnings(${targets})
set_target_properties(${targets} PROPERTIES MACOSX_RPATH ON)
ivw_default_install_targets(${targets})

#--------------------------------------------------------------------
# Creates VS folder structure
ivw_define_standard_definitions(freetype freetype)
ivw_folder(freetype ext)

#--------------------------------------------------------------------
# Make package (for other projects to find)
ivw_make_package(Freetype freetype)

