
option(GLM_QUIET "No CMake Message" ON)
option(GLM_TEST_ENABLE "Build unit tests" OFF)
add_subdirectory(glm)

add_library(inviwo::glm ALIAS glm)
target_compile_definitions(glm INTERFACE
    GLM_FORCE_SIZE_T_LENGTH
    GLM_ENABLE_EXPERIMENTAL
    GLM_FORCE_UNRESTRICTED_GENTYPE
    GLM_FORCE_SILENT_WARNINGS
)

ivw_folder(glm_static ext)
ivw_folder(glm_shared ext)
