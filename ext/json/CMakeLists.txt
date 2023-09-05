cmake_minimum_required(VERSION 3.8)
project(nlohmann_json VERSION 3.9.1 LANGUAGES CXX)

set(NLOHMANN_JSON_TARGET_NAME         ${PROJECT_NAME})
set(NLOHMANN_JSON_INCLUDE_BUILD_DIR "${PROJECT_SOURCE_DIR}/single_include/")

add_library(${NLOHMANN_JSON_TARGET_NAME} INTERFACE)
add_library(${PROJECT_NAME}::${NLOHMANN_JSON_TARGET_NAME} ALIAS ${NLOHMANN_JSON_TARGET_NAME})
target_compile_features(${NLOHMANN_JSON_TARGET_NAME} INTERFACE cxx_std_11)

target_include_directories(
    ${NLOHMANN_JSON_TARGET_NAME}
    INTERFACE
    $<BUILD_INTERFACE:${NLOHMANN_JSON_INCLUDE_BUILD_DIR}>
    $<INSTALL_INTERFACE:include>
)

## add debug view definition file for msvc (natvis)
if (MSVC)
    set(NLOHMANN_NATVIS_FILE "nlohmann_json.natvis")
    target_sources(
        ${NLOHMANN_JSON_TARGET_NAME} 
        INTERFACE 
            $<INSTALL_INTERFACE:${NLOHMANN_NATVIS_FILE}>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${NLOHMANN_NATVIS_FILE}>  
    )
endif()

#  HACK: have the files showing in the IDE
add_custom_target(json_dummy SOURCES ${PROJECT_SOURCE_DIR}/single_include/nlohmann/json.hpp)
# Creates VS folder structure
ivw_folder(json_dummy ext)
ivw_register_package(nlohmann_json nlohmann_json)