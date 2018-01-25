#--------------------------------------------------------------------
# TiCPP Package
project(ticpp)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${IVW_EXTENSIONS_DIR}/ticpp/ticpp.h
    ${IVW_EXTENSIONS_DIR}/ticpp/ticppapi.h
    ${IVW_EXTENSIONS_DIR}/ticpp/ticpprc.h
    ${IVW_EXTENSIONS_DIR}/ticpp/tinystr.h
    ${IVW_EXTENSIONS_DIR}/ticpp/tinyxml.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${IVW_EXTENSIONS_DIR}/ticpp/ticpp.cpp
    ${IVW_EXTENSIONS_DIR}/ticpp/tinystr.cpp
    ${IVW_EXTENSIONS_DIR}/ticpp/tinyxml.cpp
    ${IVW_EXTENSIONS_DIR}/ticpp/tinyxmlerror.cpp
    ${IVW_EXTENSIONS_DIR}/ticpp/tinyxmlparser.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

#--------------------------------------------------------------------
# Create library
add_library(ticpp ${SOURCE_FILES} ${HEADER_FILES})
set_target_properties(ticpp PROPERTIES VERSION 0.4 SOVERSION 0.4)

#--------------------------------------------------------------------
# Define defintions
ivw_define_standard_definitions(ticpp ticpp)
target_compile_definitions(ticpp PRIVATE TIXML_USE_TICPP)
if(WIN32)
    if(BUILD_SHARED_LIBS)
        target_compile_definitions(ticpp PRIVATE BUILD_TICPP_DLL)
        target_compile_definitions(ticpp INTERFACE TICPP_DLL)
    endif()
endif()

#--------------------------------------------------------------------
# Creates VS folder structure
ivw_folder(ticpp ext)

#--------------------------------------------------------------------
# Make package (for other projects to find)
ivw_default_install_comp_targets(core ticpp)
ivw_make_package(TiCPP ticpp)

ivw_suppress_compiler_warnings(ticpp)