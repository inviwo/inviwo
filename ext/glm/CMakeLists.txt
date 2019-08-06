
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

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    # Needed when using clang since GLM fails to detect c++11/14 support
    # https://github.com/g-truc/glm/issues/620
    target_compile_definitions(glm INTERFACE GLM_FORCE_CXX14)
endif()

if(TARGET glm_static)
    ivw_folder(glm_static ext)
endif()
if(TARGET glm_shared)
    ivw_folder(glm_shared ext)
endif()
