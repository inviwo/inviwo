#--------------------------------------------------------------------
# Dependencies for OpenglQt module
set(dependencies 
    InviwoOpenGLModule
    InviwoQtWidgets
)

if(DESIRED_QT_VERSION MATCHES 5)
    list(APPEND dependencies Qt5OpenGL)
else()
    list(APPEND dependencies Qt)
endif()



