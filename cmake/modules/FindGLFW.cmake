#=============================================================================
# InViWo Build
  
#--------------------------------------------------------------------
# Build GLFW lib
if(NOT GLFW_FOUND)
    set(BUILD_GLFW 1)
    set(GLFW_INCLUDE_DIR  ${IVW_MODULE_DIR}/glfw/ext/glfw/include)
    set(GLFW_LIBRARY glfw)
    set(GLFW_PROJECT glfw)
    set(GLFW_LIBRARY_DIR ${LIBRARY_OUTPUT_PATH})
    mark_as_advanced(FORCE  GLFW_INCLUDE_DIR )
    mark_as_advanced(FORCE  GLFW_LIBRARY )
    
    if(WIN32 AND BUILD_SHARED_LIBS)
        list(APPEND GLFW_LIBRARIES optimized ${IVW_LIBRARY_DIR}/Release/${GLFW_LIBRARY}3.lib debug ${IVW_LIBRARY_DIR}/Debug/${GLFW_LIBRARY}3${CMAKE_DEBUG_POSTFIX}.lib)
    elseif(WIN32)
        list(APPEND GLFW_LIBRARIES optimized ${GLFW_LIBRARY}3 debug ${GLFW_LIBRARY}3${CMAKE_DEBUG_POSTFIX})
    else()
        list(APPEND GLFW_LIBRARIES optimized ${GLFW_LIBRARY} debug ${GLFW_LIBRARY}${CMAKE_DEBUG_POSTFIX})
    endif()
    
endif(NOT GLFW_FOUND)
