cmake_minimum_required(VERSION 3.15...3.22 FATAL_ERROR)
project(md4c C)

if(${CMAKE_C_COMPILER_ID} MATCHES GNU|Clang)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
elseif(MSVC)
    # Disable warnings about the so-called unsecured functions:
    add_definitions(/D_CRT_SECURE_NO_WARNINGS /W0)
endif()

set(HEADER_FILES 
    include/md4c/md4c.h
)
set(SOURCE_FILES 
    src/md4c.c
)

ivw_group("Header Files" ${HEADER_FILES} )
ivw_group("Source Files" ${SOURCE_FILES} )

# shared libs are not supported
add_library(md4c STATIC ${HEADER_FILES} ${SOURCE_FILES})
add_library(inviwo::md4c ALIAS md4c)

target_include_directories(md4c INTERFACE include PRIVATE include/md4c)

ivw_folder(md4c ext)
