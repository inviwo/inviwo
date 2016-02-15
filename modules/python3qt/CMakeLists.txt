#--------------------------------------------------------------------
# Inviwo PythonQt Module
ivw_module(Python3Qt)

#--------------------------------------------------------------------
# Add MOC files
set(MOC_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/pythoneditorwidget.h
)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/pythonmenu.h
    ${CMAKE_CURRENT_SOURCE_DIR}/pythonqtmethods/pythonqtmethods.h
    ${CMAKE_CURRENT_SOURCE_DIR}/python3qtmodule.h
    ${CMAKE_CURRENT_SOURCE_DIR}/python3qtmoduledefine.h
    ${MOC_FILES}
)
ivw_group("Header Files" ${HEADER_FILES})

ivw_qt_wrap_cpp(MOC_FILES ${MOC_FILES})
source_group("MOC Files" FILES ${MOC_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/pythoneditorwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pythonmenu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pythonqtmethods/pythonqtmethods.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/python3qtmodule.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

ivw_define_qt_definitions()
ivw_add_definition(IVW_PYTHON3_QT)

# Create module
ivw_create_module(${SOURCE_FILES} ${MOC_FILES} ${HEADER_FILES})
