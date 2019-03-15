#--------------------------------------------------------------------
# Inviwo OpenGLQt Module
ivw_module(OpenGLQt)

find_package(Qt5OpenGL QUIET REQUIRED)
find_package(Qt5Svg QUIET REQUIRED)

#--------------------------------------------------------------------
# Add MOC files
set(MOC_FILES
    include/modules/openglqt/canvasprocessorwidgetqt.h
    include/modules/openglqt/openglqtmenu.h
)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    include/modules/openglqt/canvasqglwidget.h
    include/modules/openglqt/canvasqt.h
    #include/modules/openglqt/canvasqwindow.h
    include/modules/openglqt/hiddencanvasqt.h
    include/modules/openglqt/openglqtcapabilities.h
    include/modules/openglqt/openglqtmodule.h
    include/modules/openglqt/openglqtmoduledefine.h
    include/modules/openglqt/shaderwidget.h
    ${MOC_FILES}
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    src/canvasprocessorwidgetqt.cpp
    src/canvasqglwidget.cpp
    #src/canvasqwindow.cpp
    src/openglqtcapabilities.cpp
    src/openglqtmenu.cpp
    src/openglqtmodule.cpp
    src/shaderwidget.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

qt5_wrap_cpp(MOCED_FILES ${MOC_FILES})
source_group("MOC Files" FILES ${MOCED_FILES})

# Create module
ivw_create_module(NO_PCH ${SOURCE_FILES} ${MOCED_FILES} ${HEADER_FILES})
target_link_libraries(inviwo-module-openglqt PRIVATE Qt5::OpenGL Qt5::Svg)

ivw_compile_optimize_on_target(inviwo-module-openglqt)

ivw_make_package(InviwoOpenGLQtModule inviwo-module-openglqt)
ivw_qt_add_to_install(qt_modules Qt5OpenGL)
