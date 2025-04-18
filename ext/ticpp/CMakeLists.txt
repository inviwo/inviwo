# TiCPP Package
project(ticpp)

add_library(ticpp)
add_library(ticpp::ticpp ALIAS ticpp)
set_target_properties(ticpp PROPERTIES VERSION 0.5 SOVERSION 0.5)
target_include_directories(ticpp PRIVATE include/ticpp)
target_compile_definitions(ticpp 
    INTERFACE 
        $<$<AND:$<BOOL:${WIN32}>,$<BOOL:${BUILD_SHARED_LIBS}>>:TICPP_DLL>
    PRIVATE 
        $<$<AND:$<BOOL:${WIN32}>,$<BOOL:${BUILD_SHARED_LIBS}>>:BUILD_TICPP_DLL>
)
ivw_define_standard_definitions(ticpp ticpp)
ivw_define_standard_properties(ticpp)

target_sources(ticpp
    PUBLIC
    FILE_SET HEADERS
    TYPE HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
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
        include/ticpp/ticppapi.h
        include/ticpp/tinyxml.h
        include/ticpp/unknown.h
        include/ticpp/visitor.h
    PRIVATE
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
        src/tinyxml.cpp
        src/unknown.cpp
        src/visitor.cpp
)

find_package(fmt REQUIRED)
target_link_libraries(ticpp PUBLIC fmt::fmt)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC" AND IVW_CFG_MSVC_ADDRESS_SANITIZER)
    target_compile_options(ticpp PRIVATE "/fsanitize=address")
endif()

# Creates VS folder structure
ivw_folder(ticpp ext)

# Make package (for other projects to find)
ivw_make_package(ticpp ticpp)

ivw_install_helper(TARGET ticpp NAMESPACE ticpp DESTINATION inviwo)
