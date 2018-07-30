
add_subdirectory(glm)

add_library(inviwo::glm ALIAS glm)
target_compile_definitions(glm INTERFACE
    GLM_FORCE_RADIANS
    GLM_FORCE_SIZE_T_LENGTH
    GLM_FORCE_UNRESTRICTED_GENTYPE
    GLM_ENABLE_EXPERIMENTAL
)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    target_compile_definitions(glm INTERFACE  GLM_LANG_STL11_FORCED)
endif()

set_target_properties(glm_dummy PROPERTIES VERSION 0.9.9)
ivw_folder(glm_dummy ext)
