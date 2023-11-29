# Use our own packaging/install locations
set(SKIP_INSTALL_ALL TRUE)


# Try to explicity control what dependencies freetype uses. 
# Otherwise it will go ahread and depend on what ever it finds.
set(FT_REQUIRE_ZLIB ON)
set(FT_REQUIRE_PNG ON)
set(FT_DISABLE_BZIP2 ON)
set(FT_DISABLE_HARFBUZZ ON)
set(FT_DISABLE_BROTLI ON)

set(CMAKE_DISABLE_FIND_PACKAGE_ZLIB OFF)
set(CMAKE_DISABLE_FIND_PACKAGE_PNG OFF)
set(CMAKE_DISABLE_FIND_PACKAGE_BZip2 ON)
set(CMAKE_DISABLE_FIND_PACKAGE_HarfBuzz ON)
set(CMAKE_DISABLE_FIND_PACKAGE_BrotliDec ON)

# Freetype will clear this policy, here we default it to NEW to avoid the warning
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

add_subdirectory(freetype)
ivw_get_targets_in_dir_recursive(targets ${CMAKE_CURRENT_SOURCE_DIR})
ivw_suppress_compiler_warnings(${targets})
set_target_properties(${targets} PROPERTIES MACOSX_RPATH ON)
ivw_default_install_targets(${targets})

# Creates VS folder structure
ivw_define_standard_definitions(freetype freetype)
ivw_folder(freetype ext)

# Make package (for other projects to find)
ivw_register_package(Freetype freetype)

if(NOT TARGET Freetype::Freetype)
    add_library(Freetype::Freetype ALIAS freetype)
endif()