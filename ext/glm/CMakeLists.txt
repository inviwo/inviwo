add_subdirectory(glm)

add_library(inviwo::glm ALIAS glm)
target_compile_definitions(glm INTERFACE 
	GLM_FORCE_RADIANS 
	GLM_FORCE_SIZE_T_LENGTH
	GLM_FORCE_UNRESTRICTED_GENTYPE
	GLM_ENABLE_EXPERIMENTAL
)
set_target_properties(glm_dummy PROPERTIES VERSION 0.9.9)
ivw_folder(glm_dummy ext)
