#--------------------------------------------------------------------
# TiCPP Package
project(ticpp)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    include/ticpp/attribute.h
    include/ticpp/base.h
    include/ticpp/comment.h
    include/ticpp/declaration.h
    include/ticpp/document.h
    include/ticpp/element.h
    include/ticpp/fwd.h
    include/ticpp/handle.h
    include/ticpp/node.h
    include/ticpp/parsingdata.h
    include/ticpp/printer.h
    include/ticpp/stylesheet.h
    include/ticpp/text.h
    include/ticpp/ticpp.h
    include/ticpp/ticppapi.h
    include/ticpp/ticpprc.h
    include/ticpp/tinyxml.h
    include/ticpp/unknown.h
    include/ticpp/visitor.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    src/attribute.cpp
    src/base.cpp
    src/comment.cpp
    src/declaration.cpp
    src/document.cpp
    src/element.cpp
    src/fwd.cpp
    src/handle.cpp
    src/node.cpp
    src/parsingdata.cpp
    src/printer.cpp
    src/stylesheet.cpp
    src/text.cpp
    src/ticpp.cpp
    src/ticpprc.cpp
    src/tinyxml.cpp
    src/unknown.cpp
    src/visitor.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

#--------------------------------------------------------------------
# Create library
add_library(ticpp ${SOURCE_FILES} ${HEADER_FILES})
set_target_properties(ticpp PROPERTIES VERSION 0.4 SOVERSION 0.4)
target_include_directories(ticpp 
    PUBLIC include
    PRIVATE include/ticpp
)
#--------------------------------------------------------------------
# Define defintions
ivw_define_standard_definitions(ticpp ticpp)
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
ivw_default_install_targets(ticpp)
ivw_make_package(TiCPP ticpp)

ivw_suppress_compiler_warnings(ticpp)
