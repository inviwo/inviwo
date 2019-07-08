set(BUILD_SHARED_LIBS ON)

add_subdirectory(freetype)

#--------------------------------------------------------------------
# Supress warnings
ivw_suppress_compiler_warnings(freetype)

#--------------------------------------------------------------------
# Creates VS folder structure
ivw_define_standard_definitions(freetype freetype)
ivw_folder(freetype ext)

#--------------------------------------------------------------------
# Make package (for other projects to find)
#ivw_default_install_targets(freetype)
ivw_make_package(Freetype freetype)
