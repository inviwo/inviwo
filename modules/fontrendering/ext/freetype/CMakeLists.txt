project(freetype)

set(FREETYPE_src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/autofit/autofit.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/bdf/bdf.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cff/cff.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/ftbase.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/ftbbox.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/ftbitmap.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cache/ftcache.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/ftcid.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/ftdebug.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/ftfstype.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/ftgasp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/ftglyph.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/ftgxval.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gzip/ftgzip.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/ftinit.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/ftlcdfil.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lzw/ftlzw.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/ftmm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/ftotval.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/ftpatent.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/ftpfr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/ftstroke.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/ftsynth.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/ftsystem.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/fttype1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/ftwinfnt.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/ftxf86.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pcf/pcf.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pfr/pfr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/psaux/psaux.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pshinter/pshinter.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/psnames/psmodule.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/raster/raster.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/smooth/smooth.c        
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sfnt/sfnt.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/truetype/truetype.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/type1/type1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cid/type1cid.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/type42/type42.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/winfonts/winfnt.c
)

SET(freetype_HDRS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ft2build.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/freetype/config/ftconfig.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/freetype/config/ftheader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/freetype/config/ftmodule.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/freetype/config/ftoption.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/freetype/config/ftstdlib.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/freetype/freetype.h
)

add_library(freetype ${FREETYPE_src} ${freetype_HDRS})

set_target_properties(freetype PROPERTIES
    COMPILE_DEFINITIONS "WIN32;_LIB;_CRT_SECURE_NO_WARNINGS;FT2_BUILD_LIBRARY")

#--------------------------------------------------------------------
# Supress warnings
ivw_suppress_compiler_warnings(freetype)

target_include_directories(freetype PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    #$<INSTALL_INTERFACE:???>
)

#--------------------------------------------------------------------
# Creates VS folder structure
ivw_define_standard_definitions(freetype freetype)
ivw_folder(freetype ext)

#--------------------------------------------------------------------
# Make package (for other projects to find)
ivw_default_install_targets(freetype)
ivw_make_package(Freetype freetype)

