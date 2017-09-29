project(GLEW)

set(GLEW_MAJOR_VERSION 2)
set(GLEW_MINOR_VERSION 0)
set(GLEW_PATCH_VERSION 0)
set(GLEW_VERSION ${GLEW_MAJOR_VERSION}.${GLEW_MINOR_VERSION}.${GLEW_PATCH_VERSION})

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/GL/eglew.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/GL/glew.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/GL/glxew.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/GL/wglew.h
)

source_group("Header Files" FILES ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/glew.c
)

source_group("Source Files" FILES ${SOURCE_FILES})

#--------------------------------------------------------------------
# Locate OpenGL
find_package(OpenGL QUIET REQUIRED)

#--------------------------------------------------------------------
# Set the compiler flags
if(WIN32)

else()
  add_definitions(-DHAVE_CONFIG_H)
endif()

add_definitions(-DUNICODE)
source_group("CMake Files" FILES ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt)

#--------------------------------------------------------------------
# Create a target for the library
add_library(GLEW ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(GLEW PUBLIC ${OPENGL_LIBRARIES})

#--------------------------------------------------------------------
# Set includes
target_include_directories(GLEW PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    #$<INSTALL_INTERFACE:???>
    ${OPENGL_INCLUDE_DIR}
)
if(BUILD_SHARED_LIBS)
    target_compile_definitions(GLEW PRIVATE GLEW_BUILD)
else()
    target_compile_definitions(GLEW PRIVATE GLEW_STATIC)
endif()

ivw_folder(GLEW ext)

#--------------------------------------------------------------------
# Add to package
ivw_default_install_targets(GLEW)
ivw_make_package(GLEW GLEW)
