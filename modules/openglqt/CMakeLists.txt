#--------------------------------------------------------------------
# Inviwo OpenGLQt Module
ivw_module(OpenGLQt)

#--------------------------------------------------------------------
# Add MOC files
set(MOC_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/canvasprocessorwidgetqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/openglqtmenu.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shaderwidget.h
)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/canvasqglwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/canvasqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/canvasqwindow.h
    ${CMAKE_CURRENT_SOURCE_DIR}/hiddencanvasqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/openglqtcapabilities.h
    ${CMAKE_CURRENT_SOURCE_DIR}/openglqtmodule.h
    ${CMAKE_CURRENT_SOURCE_DIR}/openglqtmoduledefine.h
    ${MOC_FILES}
)

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/canvasprocessorwidgetqt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/canvasqglwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/canvasqwindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/openglqtcapabilities.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/openglqtmenu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/openglqtmodule.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shaderwidget.cpp
)

if(NOT ${Qt5Core_VERSION} VERSION_LESS "5.4.0")
    list(APPEND HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/canvasqopenglwidget.h)
    list(APPEND SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/canvasqopenglwidget.cpp)
endif()


ivw_qt_wrap_cpp(MOC_FILES ${MOC_FILES})
source_group("MOC Files" FILES ${MOC_FILES})
ivw_group("Header Files" ${HEADER_FILES})
ivw_group("Source Files" ${SOURCE_FILES})

ivw_define_qt_definitions()
ivw_add_definition(QT_OPENGL_LIB)

# Create module
ivw_create_module(${SOURCE_FILES} ${MOC_FILES} ${HEADER_FILES})

# Add Qt OpenGL to package
if(IVW_PACKAGE_PROJECT)
    if(WIN32)
        if(Qt5OpenGL_FOUND)
            set(QT_BINARY_DIR "${Qt5OpenGL_DIR}/../../../bin")
            install(FILES ${QT_BINARY_DIR}/Qt5OpenGL${CMAKE_DEBUG_POSTFIX}.dll DESTINATION bin COMPONENT qt_modules CONFIGURATIONS Debug)
            install(FILES ${QT_BINARY_DIR}/Qt5OpenGL.dll DESTINATION bin COMPONENT qt_modules CONFIGURATIONS Release)
        endif()
    endif()
endif()
