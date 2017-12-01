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
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(inviwo-module-glfw PUBLIC glfw)

ivw_register_license_file(ID glfw NAME "GLFW" VERSION 3.1.0 MODULE GLFW
    URL http://www.glfw.org
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/ext/glfw/COPYING.txt
)
ivw_make_package(InviwoGLFWModule inviwo-module-glfw)