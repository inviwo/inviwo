#--------------------------------------------------------------------
# Sigar Library
option(IVW_SIGAR "Use sigar to detect system capabilities" ON)
if(NOT IVW_SIGAR)
    return()
endif()

project(sigar)

#--------------------------------------------------------------------
# Define defintions
if(WIN32)
    ## make sure we only use the smallest set of
    ## headers on win32. Otherwise we get clashes
    ## between winsock2.h and winsock.h
    add_definitions(-DWIN32_LEAN_AND_MEAN)
endif()

source_group("CMake Files" FILES ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt)

set(HEADER_FILES include/sigar.h
    include/sigar_fileinfo.h
    include/sigar_format.h
    include/sigar_getline.h
    include/sigar_log.h
    include/sigar_private.h
    include/sigar_ptql.h
    include/sigar_util.h
)

## linux
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(SIGAR_SRC src/os/linux/linux_sigar.c)
    CHECK_INCLUDE_FILE("rpc/rpc.h" HAS_RPCH)
    if(HAS_RPCH)
        message("Using RPC from glibc")
    else()
        find_package(TIRPC REQUIRED)
        message("Using libtirpc")
    endif()
endif(CMAKE_SYSTEM_NAME STREQUAL "Linux")

## macosx, freebsd
if(CMAKE_SYSTEM_NAME MATCHES "(Darwin|FreeBSD)")
    set(SIGAR_SRC src/os/darwin/darwin_sigar.c)
    include_directories(src/os/darwin/)
    if(CMAKE_SYSTEM_NAME MATCHES "(Darwin)")
        add_definitions(-DDARWIN)
        set(SIGAR_LINK_FLAGS "-framework CoreServices -framework IOKit")
    else()
        ## freebsd needs libkvm
        set(SIGAR_LINK_FLAGS "-lkvm")
    endif()
endif()

if(WIN32)
    set(SIGAR_SRC src/os/win32/peb.c src/os/win32/win32_sigar.c src/os/win32/wmi.cpp)
else()
    add_definitions(-DHAVE_CONFIG_H)
endif()

set(SIGAR_SRC ${SIGAR_SRC}
    src/sigar.c
    src/sigar_cache.c
    src/sigar_fileinfo.c
    src/sigar_format.c
    src/sigar_getline.c
    src/sigar_ptql.c
    src/sigar_signal.c
    src/sigar_util.c
)

add_library(sigar ${SIGAR_SRC} ${HEADER_FILES})
add_library(inviwo::sigar ALIAS sigar)
set_target_properties(sigar PROPERTIES VERSION 1.6.4 SOVERSION 1.6)

target_include_directories(sigar PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/ext/sigar>
)

target_compile_definitions(sigar PUBLIC IVW_SIGAR)

## linux
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(sys_headers src/os/linux/)
    target_link_libraries(sigar PRIVATE tirpc)
endif()
## macosx, freebsd
if(CMAKE_SYSTEM_NAME MATCHES "(Darwin|FreeBSD)")
    set(sys_headers src/os/darwin/)
endif()
if(WIN32)
    if(BUILD_SHARED_LIBS)
        target_compile_definitions(sigar PUBLIC SIGAR_SHARED)
        target_compile_definitions(sigar PUBLIC SIGAR_EXPORTS)
    endif()
    target_compile_definitions(sigar PRIVATE -D_CRT_SECURE_NO_WARNINGS
                                             -D_CRT_SECURE_NO_DEPRECATE)
    target_link_libraries(sigar PRIVATE ws2_32 netapi32 version)
    set(sys_headers src/os/win32/)
endif()
target_include_directories(sigar PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${sys_headers}>
    $<INSTALL_INTERFACE:include/ext/sigar/${sys_headers}>
    ${TIRPC_INCLUDE_DIRS}
)

if(SIGAR_LINK_FLAGS)
    set_property(TARGET sigar APPEND_STRING PROPERTY LINK_FLAGS "${SIGAR_LINK_FLAGS}")
endif()

#--------------------------------------------------------------------
# Creates VS folder structure
ivw_folder(sigar ext)

#--------------------------------------------------------------------
# Make package (for other projects to find)
ivw_default_install_comp_targets(core sigar)
ivw_make_package(Sigar sigar)

ivw_suppress_compiler_warnings(sigar)
