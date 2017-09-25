option(IVW_TINY_GLFW_APPLICATION "Build Inviwo Tiny GLFW Application" OFF)
ivw_add_build_module_dependency(GLFW IVW_TINY_GLFW_APPLICATION)
if(IVW_TINY_GLFW_APPLICATION)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/minimals/glfw)
endif()

option(IVW_TINY_QT_APPLICATION "Build Inviwo Tiny QT Application" OFF)
if(IVW_TINY_QT_APPLICATION)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/minimals/qt)
endif()

if(IVW_QT_APPLICATION)
    add_subdirectory(inviwo)
endif()