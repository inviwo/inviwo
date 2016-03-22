#--------------------------------------------------------------------
# Build GLFW
add_subdirectory(ext/glfw)

#--------------------------------------------------------------------
# Inviwo GLFW Module
ivw_module(GLFW)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/canvasglfw.h
    ${CMAKE_CURRENT_SOURCE_DIR}/canvasprocessorwidgetglfw.h
    ${CMAKE_CURRENT_SOURCE_DIR}/glfwexception.h
    ${CMAKE_CURRENT_SOURCE_DIR}/glfwmodule.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/canvasglfw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/canvasprocessorwidgetglfw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/glfwexception.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/glfwmodule.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

#--------------------------------------------------------------------
# Create module
ivw_set_cpack_name(glfw_modules)
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES})