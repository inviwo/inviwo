cmake_minimum_required(VERSION 3.23...3.27 FATAL_ERROR)
project(md4c C)

if(${CMAKE_C_COMPILER_ID} MATCHES GNU|Clang)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
elseif(MSVC)
    # Disable warnings about the so-called unsecured functions:
    add_definitions(/D_CRT_SECURE_NO_WARNINGS /W0)
endif()

# shared libs are not supported
add_library(md4c STATIC)
add_library(inviwo::md4c ALIAS md4c)

target_include_directories(md4c 
    INTERFACE 
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>    
    PRIVATE 
        include/md4c)

target_sources(md4c
    PUBLIC
    FILE_SET HEADERS
    TYPE HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        include/md4c/md4c.h
    PRIVATE
        src/md4c.c
)

ivw_suppress_compiler_warnings(md4c)
ivw_default_install_targets(md4c)
ivw_folder(md4c ext)
