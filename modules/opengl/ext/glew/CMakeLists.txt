project(GLEW)

set(GLEW_MAJOR_VERSION 1)
set(GLEW_MINOR_VERSION 11)
set(GLEW_PATCH_VERSION 0)
set(GLEW_VERSION ${GLEW_MAJOR_VERSION}.${GLEW_MINOR_VERSION}.${GLEW_PATCH_VERSION})

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
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
# Set includes
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include 
    ${OPENGL_INCLUDE_DIR}
)

#--------------------------------------------------------------------
# Set the compiler flags
if(WIN32)
  add_definitions(-DGLEW_BUILD)
else(WIN32)
  add_definitions(-DHAVE_CONFIG_H)
endif(WIN32)

add_definitions(-DUNICODE)
source_group("CMake Files" FILES ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt)

#--------------------------------------------------------------------
# Create a target for the library
add_library(GLEW ${SOURCE_FILES} ${HEADER_FILES})

# InViWo Macros
ivw_vs_folder(GLEW ext)

target_link_libraries(GLEW
    ${OPENGL_LIBRARIES}
)

#--------------------------------------------------------------------
# Add to package
if(IVW_PACKAGE_PROJECT AND BUILD_SHARED_LIBS)  
if(WIN32)
   install(TARGETS GLEW
            RUNTIME DESTINATION bin
            COMPONENT modules)
else()
    install(TARGETS GLEW
            RUNTIME DESTINATION bin
            BUNDLE DESTINATION bin
            ARCHIVE DESTINATION lib
            LIBRARY DESTINATION lib
            COMPONENT modules)
endif()
endif()