project(sml LANGUAGES CXX)

add_library(sml INTERFACE)
add_library(inviwo::sml ALIAS sml)

target_include_directories(sml INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)