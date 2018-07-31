
add_subdirectory(glm)

add_library(inviwo::glm ALIAS glm)
target_compile_definitions(glm INTERFACE
    GLM_FORCE_RADIANS
    GLM_FORCE_SIZE_T_LENGTH
    GLM_FORCE_UNRESTRICTED_GENTYPE
    GLM_ENABLE_EXPERIMENTAL
)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    # Needed when using clang since GLM fails to detect c++11/14 support
    # https://github.com/g-truc/glm/issues/620
    target_compile_definitions(glm INTERFACE GLM_FORCE_CXX14)
endif()

set_target_properties(glm_dummy PROPERTIES VERSION 0.9.9)
ivw_folder(glm_dummy ext)
