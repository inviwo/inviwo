cmake_minimum_required(VERSION 3.23...3.27 FATAL_ERROR)
project(inviwo-tracy DESCRIPTION "Inviwo tracy wrapper" LANGUAGES CXX)

set(HEADER_FILES
    include/inviwo/tracy/tracy.h
    include/inviwo/tracy/tracyopengl.h
)

add_library(tracywrap INTERFACE)
add_library(inviwo::tracywrap ALIAS tracywrap)

target_sources(
    tracywrap
    INTERFACE
    FILE_SET HEADERS
    TYPE HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        ${HEADER_FILES}
)

option(IVW_ENABLE_TRACY "Enable profiling using Tracy" OFF)
if(IVW_ENABLE_TRACY)
    find_package(Threads REQUIRED)
    find_package(Tracy CONFIG REQUIRED)
    target_compile_definitions(tracywrap INTERFACE TRACY_ENABLE)
    target_link_libraries(tracywrap INTERFACE Tracy::TracyClient)
endif()

add_custom_target(tracywrap_dummy SOURCES ${HEADER_FILES})
ivw_folder(tracywrap_dummy ext)

ivw_register_package(tracywrap tracywrap)
ivw_install_helper(TARGET tracywrap NAMESPACE inviwo DESTINATION inviwo)