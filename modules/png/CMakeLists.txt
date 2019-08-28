#--------------------------------------------------------------------
# Inviwo png Module
ivw_module(png)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    include/inviwo/png/pngmodule.h
    include/inviwo/png/pngmoduledefine.h
    include/inviwo/png/pngreader.h
    include/inviwo/png/pngutils.h
    include/inviwo/png/pngwriter.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    src/pngmodule.cpp
    src/pngreader.cpp
    src/pngutils.cpp
    src/pngwriter.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

#--------------------------------------------------------------------
# Add shaders
set(SHADER_FILES
)
ivw_group("Shader Files" ${SHADER_FILES})

#--------------------------------------------------------------------
# Add Unittests
set(TEST_FILES
    tests/unittests/png-unittest-main.cpp
    tests/unittests/png-savetobuffer-test.cpp
)
ivw_add_unittest(${TEST_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

find_package(PNG REQUIRED)
target_link_libraries(inviwo-module-png PRIVATE PNG::PNG)

#--------------------------------------------------------------------
# Add shader directory to pack
# ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/glsl)


ivw_register_license_file(NAME libpng TARGET PNG::PNG MODULE CImg TYPE "libpng (BSD-style)"
    URL http://www.libpng.org/pub/png/libpng.html
    FILES ${IVW_EXTENSIONS_DIR}/libpng/LICENSE.txt
)
