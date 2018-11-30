#--------------------------------------------------------------------
# Build GLFW
add_subdirectory(ext/glfw)

#--------------------------------------------------------------------
# Inviwo GLFW Module
ivw_module(GLFW)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    include/modules/glfw/canvasglfw.h
    include/modules/glfw/canvasprocessorwidgetglfw.h
    include/modules/glfw/glfwexception.h
    include/modules/glfw/glfwmodule.h
    include/modules/glfw/glfwmoduledefine.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    src/canvasglfw.cpp
    src/canvasprocessorwidgetglfw.cpp
    src/glfwexception.cpp
    src/glfwmodule.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(inviwo-module-glfw PUBLIC glfw)

ivw_register_license_file(NAME "GLFW" VERSION 3.2.1 MODULE GLFW
    URL http://www.glfw.org
    TYPE "The zlib/libpng License"
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/ext/glfw/glfw/COPYING.txt
)
ivw_make_package(InviwoGLFWModule inviwo-module-glfw)
