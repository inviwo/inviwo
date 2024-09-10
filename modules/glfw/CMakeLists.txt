# Inviwo GLFW Module
ivw_module(GLFW)

set(HEADER_FILES
    include/modules/glfw/canvasglfw.h
    include/modules/glfw/canvasprocessorwidgetglfw.h
    include/modules/glfw/filewatcher.h
    include/modules/glfw/glfwexception.h
    include/modules/glfw/glfwmodule.h
    include/modules/glfw/glfwmoduledefine.h
    include/modules/glfw/glfwuserdata.h
    include/modules/glfw/glfwwindoweventmanager.h
)
ivw_group("Header Files" ${HEADER_FILES})

set(SOURCE_FILES
    src/canvasglfw.cpp
    src/canvasprocessorwidgetglfw.cpp
    src/filewatcher.cpp
    src/glfwexception.cpp
    src/glfwmodule.cpp
    src/glfwuserdata.cpp
    src/glfwwindoweventmanager.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES})

find_package(utf8cpp CONFIG REQUIRED)
target_link_libraries(inviwo-module-glfw PRIVATE
    utf8cpp::utf8cpp
)

find_package(glfw3 REQUIRED)
ivw_vcpkg_install(glfw3 MODULE GLFW)

target_link_libraries(inviwo-module-glfw PUBLIC glfw)
