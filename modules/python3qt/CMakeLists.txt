# Inviwo PythonQt Module
ivw_module(Python3Qt)

# https://doc-snapshots.qt.io/qt6-dev/cmake-qt5-and-qt6-compatibility.html#supporting-older-qt-5-versions   
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Gui Widgets Svg REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Widgets Svg REQUIRED)

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

if(QT_VERSION_MAJOR EQUAL 6)
    qt_wrap_cpp(MOCED_FILES ${MOC_FILES})
else()
    qt5_wrap_cpp(MOCED_FILES ${MOC_FILES})
endif()
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
ivw_create_module(NO_PCH ${SOURCE_FILES} ${MOCED_FILES} ${HEADER_FILES})
target_link_libraries(inviwo-module-python3qt PRIVATE 
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Svg
)
target_compile_definitions(inviwo-module-animationqt PUBLIC 
    "QT_DEPRECATED_WARNINGS_SINCE=QT_VERSION_CHECK(5, 15, 0)"
)
# Add templates directory to packaging
ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/templates)

ivw_make_package(InviwoPython3QtModule inviwo-module-python3qt)
