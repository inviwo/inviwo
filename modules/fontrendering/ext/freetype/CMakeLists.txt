# Use our own packaging/install locations
set(SKIP_INSTALL_ALL TRUE)


# Try to explicity control what dependencies freetype uses. 
# Otherwise it will go ahread and depend on what ever it finds.
option(FT_WITH_ZLIB "Use system zlib instead of internal library." ON)
option(FT_WITH_BZIP2 "Support bzip2 compressed fonts." OFF)
option(FT_WITH_PNG "Support PNG compressed OpenType embedded bitmaps." ON)
option(FT_WITH_HARFBUZZ "Improve auto-hinting of OpenType fonts." OFF)
option(FT_WITH_BROTLI "Support compressed WOFF2 fonts." OFF)

set(CMAKE_DISABLE_FIND_PACKAGE_ZLIB FALSE)
set(CMAKE_DISABLE_FIND_PACKAGE_BZip2 FALSE)
set(CMAKE_DISABLE_FIND_PACKAGE_PNG FALSE)
set(CMAKE_DISABLE_FIND_PACKAGE_HarfBuzz TRUE)
set(CMAKE_DISABLE_FIND_PACKAGE_BrotliDec TRUE)

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

