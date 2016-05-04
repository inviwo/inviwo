#--------------------------------------------------------------------
# TiCPP Package
ivw_project(ticpp)

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

#--------------------------------------------------------------------
# Define defintions
ivw_define_standard_definitions(ticpp ticpp)
ivw_add_definition(TIXML_USE_TICPP)
if(WIN32)
    if(BUILD_SHARED_LIBS)
        ivw_add_definition_to_list(TICPP_DLL)
        add_definitions(-DBUILD_TICPP_DLL)
    endif()
endif()

#--------------------------------------------------------------------
# Creates VS folder structure
ivw_folder(ticpp ext)

#--------------------------------------------------------------------
# Make package (for other projects to find)
ivw_set_cpack_name(core)
ivw_make_package(TiCPP ticpp)

if(APPLE)
  #Ignore warnings in external packages
  set_target_properties(ticpp PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_INHIBIT_ALL_WARNINGS YES)
endif()