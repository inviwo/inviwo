cmake_minimum_required(VERSION 3.13...3.18 FATAL_ERROR)
project(inviwo-tracy DESCRIPTION "Inviwo tracy wrapper" LANGUAGES CXX)

set(HEADER_FILES
    include/inviwo/tracy/tracy.h
    include/inviwo/tracy/tracyopengl.h
)

add_library(inviwo-tracy INTERFACE)
add_library(inviwo::tracy ALIAS inviwo-tracy)

target_include_directories(inviwo-tracy INTERFACE include)

option(IVW_USE_TRACY "Enable profiling using Tracy" OFF)
if(IVW_USE_TRACY)
    find_package(Threads REQUIRED)
    find_package(Tracy CONFIG REQUIRED)
    target_compile_definitions(inviwo-tracy INTERFACE TRACY_ENABLE)
    target_link_libraries(inviwo-tracy INTERFACE Tracy::TracyClient)
endif()

add_custom_target(inviwo-tracy_dummy SOURCES ${HEADER_FILES})
ivw_folder(inviwo-tracy_dummy ext)
