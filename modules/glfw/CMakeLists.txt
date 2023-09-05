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
    utf8cpp
)

option(IVW_USE_EXTERNAL_GLFW "GLFW is provided externaly" OFF)
if(NOT IVW_USE_EXTERNAL_GLFW)
    add_subdirectory(ext/glfw)
    ivw_register_license_file(NAME "GLFW" VERSION ${GLEW_VERSION} MODULE GLFW
        URL http://www.glfw.org
        TYPE "The zlib/libpng License"
        FILES ${CMAKE_CURRENT_SOURCE_DIR}/ext/glfw/glfw/LICENSE.md
    )
    ivw_default_install_targets(glfw)
    ivw_make_package(glfw3 glfw)
else()
    find_package(glfw3 REQUIRED)
    ivw_vcpkg_install(glfw3 MODULE GLFW)
endif()

target_link_libraries(inviwo-module-glfw PUBLIC glfw)

