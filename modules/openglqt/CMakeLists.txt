#--------------------------------------------------------------------
# Inviwo OpenGLQt Module
ivw_module(OpenGLQt)

find_package(Qt5OpenGL QUIET REQUIRED)

#--------------------------------------------------------------------
# Add MOC files
set(MOC_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/canvasprocessorwidgetqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/openglqtmenu.h
)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/canvasqglwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/canvasqt.h
    #${CMAKE_CURRENT_SOURCE_DIR}/canvasqwindow.h
    ${CMAKE_CURRENT_SOURCE_DIR}/hiddencanvasqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/openglqtcapabilities.h
    ${CMAKE_CURRENT_SOURCE_DIR}/openglqtmodule.h
    ${CMAKE_CURRENT_SOURCE_DIR}/openglqtmoduledefine.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shaderwidget.h
    ${MOC_FILES}
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/canvasprocessorwidgetqt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/canvasqglwidget.cpp
    #${CMAKE_CURRENT_SOURCE_DIR}/canvasqwindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/openglqtcapabilities.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/openglqtmenu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/openglqtmodule.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shaderwidget.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

#if(NOT ${Qt5Core_VERSION} VERSION_LESS "5.4.0")
#    list(APPEND HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/canvasqopenglwidget.h)
#    list(APPEND SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/canvasqopenglwidget.cpp)
#endif()

qt5_wrap_cpp(MOCED_FILES ${MOC_FILES})
source_group("MOC Files" FILES ${MOCED_FILES})

# Create module
ivw_create_module(NO_PCH ${SOURCE_FILES} ${MOCED_FILES} ${HEADER_FILES})
target_link_libraries(inviwo-module-openglqt PRIVATE Qt5::OpenGL)

ivw_compile_optimize_on_target(inviwo-module-openglqt)

ivw_make_package(InviwoOpenGLQtModule inviwo-module-openglqt)
ivw_qt_add_to_install(qt_modules Qt5OpenGL)

