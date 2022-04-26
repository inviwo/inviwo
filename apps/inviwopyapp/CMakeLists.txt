# Inviwo Qt Application
project(inviwopyapp)

# https://doc-snapshots.qt.io/qt6-dev/cmake-qt5-and-qt6-compatibility.html#supporting-older-qt-5-versions	
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Gui Widgets REQUIRED)   
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Widgets REQUIRED)


find_package(pybind11 CONFIG REQUIRED)

# Add source files
set(SOURCE_FILES
    inviwopyapp.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

# Add source files
set(PYTHON_FILES
    inviwo.py
)
ivw_group("Python Files" ${PYTHON_FILES})

set(CMAKE_FILES
    CMakeLists.txt
)
ivw_group("CMake Files" ${CMAKE_FILES})

ivw_retrieve_all_modules(enabled_modules)
# Only include one OpenGL context creation module
list(REMOVE_ITEM enabled_modules InviwoGLFWModule)

# Create application
pybind11_add_module(inviwopyapp ${SOURCE_FILES} ${PYTHON_FILES} ${CMAKE_FILES})

set_target_properties(inviwopyapp PROPERTIES DEBUG_POSTFIX "")
set_target_properties(inviwopyapp PROPERTIES PREFIX "")
set_target_properties(inviwopyapp PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

target_link_libraries(inviwopyapp PUBLIC 
    inviwo::core
    inviwo::qtapplicationbase
    inviwo::module::qtwidgets
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
)
ivw_configure_application_module_dependencies(inviwopyapp ${enabled_modules})
ivw_define_standard_definitions(inviwopyapp inviwopyapp) 
ivw_define_standard_properties(inviwopyapp)
target_compile_definitions(inviwopyapp PUBLIC 
    "QT_DEPRECATED_WARNINGS_SINCE=QT_VERSION_CHECK(5, 15, 0)"
)
# Move to folder
ivw_folder(inviwopyapp minimals)
# Add application to pack
ivw_default_install_targets(inviwopyapp)

set_property(GLOBAL APPEND PROPERTY IVW_PYAPP_LIST inviwopyapp)
