project(sml LANGUAGES CXX)

add_library(sml INTERFACE)
add_library(inviwo::sml ALIAS sml)

target_sources(sml
    INTERFACE
    FILE_SET HEADERS
    TYPE HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/sml/sml.hpp
)

#  HACK: have the files showing in the IDE
add_custom_target(sml_dummy SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/include/sml/sml.hpp)
# Creates VS folder structure
ivw_folder(sml_dummy ext)

ivw_register_package(sml sml)
ivw_install_helper(TARGET sml NAMESPACE inviwo DESTINATION inviwo)
