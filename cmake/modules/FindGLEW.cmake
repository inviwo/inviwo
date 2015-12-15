#
# Try to find GLEW library and include path.
# Once done this will define
#
# GLEW_FOUND
# GLEW_INCLUDE_DIR
# GLEW_LIBRARY
# GLEW_LIBRARIES
# 

# if(WIN32)
#     find_path( GLEW_INCLUDE_DIR GL/glew.h
#         $ENV{PROGRAMFILES}/GLEW/include
#         ${PROJECT_SOURCE_DIR}/src/nvgl/glew/include
#         DOC "The directory where GL/glew.h resides")
#     find_library( GLEW_LIBRARY
#         NAMES glew GLEW glew32 glew32s
#         PATHS
#         $ENV{PROGRAMFILES}/GLEW/lib
#         ${PROJECT_SOURCE_DIR}/src/nvgl/glew/bin
#         ${PROJECT_SOURCE_DIR}/src/nvgl/glew/lib
#         DOC "The GLEW library")
# else(WIN32)
#     find_path( GLEW_INCLUDE_DIR GL/glew.h
#         /usr/include
#         /usr/local/include
#         /sw/include
#         /opt/local/include
#         DOC "The directory where GL/glew.h resides")
#     find_library( GLEW_LIBRARY
#         NAMES glew GLEW
#         PATHS
#         /usr/lib64
#         /usr/lib
#         /usr/local/lib64
#         /usr/local/lib
#         /sw/lib
#         /opt/local/lib
#         DOC "The GLEW library")
# endif(WIN32)
# 
# get_filename_component( GLEW_LIBRARY_DIR ${GLEW_LIBRARY} PATH )
# 
# if(GLEW_INCLUDE_DIR)
#     set(GLEW_FOUND 1 CACHE STRING "Set to 1 if GLEW is found, 0 otherwise")
# else(GLEW_INCLUDE_DIR)
#     set(GLEW_FOUND 0 CACHE STRING "Set to 1 if GLEW is found, 0 otherwise")
# endif(GLEW_INCLUDE_DIR)
# 
# mark_as_advanced(GLEW_FOUND)

#=============================================================================
# InViWo Build

#--------------------------------------------------------------------
# Build GLEW lib
#if(NOT GLEW_FOUND)
    set(BUILD_GLEW 1)
    set(GLEW_INCLUDE_DIR ${IVW_MODULE_DIR}/opengl/ext/glew/include)
    set(GLEW_LIBRARY GLEW)
    set(GLEW_LIBRARY_DIR ${LIBRARY_OUTPUT_PATH})
    mark_as_advanced(FORCE  GLEW_INCLUDE_DIR)
    mark_as_advanced(FORCE  GLEW_LIBRARY)
#endif(NOT GLEW_FOUND)

if(WIN32 AND BUILD_SHARED_LIBS)
    set(GLEW_LIBRARIES optimized ${IVW_LIBRARY_DIR}/Release/${GLEW_LIBRARY}.lib debug ${IVW_LIBRARY_DIR}/Debug/${GLEW_LIBRARY}${CMAKE_DEBUG_POSTFIX}.lib)
else()
    set(GLEW_LIBRARIES optimized ${GLEW_LIBRARY} debug ${GLEW_LIBRARY}${CMAKE_DEBUG_POSTFIX})
endif()
