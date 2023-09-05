project(GLEW)

set(GLEW_MAJOR_VERSION 2)
set(GLEW_MINOR_VERSION 0)
set(GLEW_PATCH_VERSION 0)
set(GLEW_VERSION ${GLEW_MAJOR_VERSION}.${GLEW_MINOR_VERSION}.${GLEW_PATCH_VERSION})

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    include/GL/eglew.h
    include/GL/glew.h
    include/GL/glxew.h
    include/GL/wglew.h
)

source_group("Header Files" FILES ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    src/glew.c
)

source_group("Source Files" FILES ${SOURCE_FILES})

#--------------------------------------------------------------------
# Locate OpenGL
find_package(OpenGL QUIET REQUIRED)
source_group("CMake Files" FILES sMakeLists.txt)

#--------------------------------------------------------------------
# Create a target for the library
add_library(GLEW ${SOURCE_FILES} ${HEADER_FILES})
add_library(inviwo::GLEW ALIAS GLEW)
target_link_libraries(GLEW PUBLIC ${OPENGL_LIBRARIES})
set_target_properties(GLEW PROPERTIES VERSION ${GLEW_VERSION} SOVERSION ${GLEW_MAJOR_VERSION}.${GLEW_MINOR_VERSION})
target_include_directories(GLEW PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    ${OPENGL_INCLUDE_DIR}
)

if(BUILD_SHARED_LIBS)
    target_compile_definitions(GLEW PRIVATE GLEW_BUILD)
else()
    target_compile_definitions(GLEW PUBLIC GLEW_STATIC)
endif()

if(WIN32)
    target_compile_definitions(GLEW PRIVATE UNICODE)
else()
    target_compile_definitions(GLEW PRIVATE HAVE_CONFIG_H)
endif()

ivw_folder(GLEW ext)

#--------------------------------------------------------------------
# Add to package
ivw_default_install_targets(GLEW)
ivw_register_package(GLEW GLEW)
