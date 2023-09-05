# Inviwo PythonQt Module
ivw_module(Python3Qt)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Svg)

# Add MOC files
set(MOC_FILES
)

# Add header files
set(HEADER_FILES
    include/modules/python3qt/properties/pythoneditordockwidget.h
    include/modules/python3qt/properties/pythonfilepropertywidgetqt.h
    include/modules/python3qt/properties/pythonpropertywidgetqt.h
    include/modules/python3qt/python3qtmodule.h
    include/modules/python3qt/python3qtmoduledefine.h
    include/modules/python3qt/pythoneditorwidget.h
    include/modules/python3qt/pythonmenu.h
    include/modules/python3qt/pythonsyntaxhighlight.h
)
ivw_group("Header Files" ${HEADER_FILES})

qt_wrap_cpp(MOCED_FILES ${MOC_FILES})
source_group("MOC Files" FILES ${MOCED_FILES})

# Add source files
set(SOURCE_FILES
    src/properties/pythoneditordockwidget.cpp
    src/properties/pythonfilepropertywidgetqt.cpp
    src/properties/pythonpropertywidgetqt.cpp
    src/python3qtmodule.cpp
    src/pythoneditorwidget.cpp
    src/pythonmenu.cpp
    src/pythonsyntaxhighlight.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

# Create module
ivw_create_module(NO_PCH QT ${SOURCE_FILES} ${MOCED_FILES} ${HEADER_FILES})
target_link_libraries(inviwo-module-python3qt PUBLIC
    inviwo::qtapplicationbase
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Svg
)

# Add templates directory to packaging
ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/templates)
