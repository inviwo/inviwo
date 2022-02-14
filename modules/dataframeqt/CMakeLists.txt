# Inviwo DataFrameQt Module
ivw_module(DataFrameQt)

# https://doc-snapshots.qt.io/qt6-dev/cmake-qt5-and-qt6-compatibility.html#supporting-older-qt-5-versions   
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Gui Widgets Svg REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Widgets Svg REQUIRED)

#-------------------------------------------------------------------- 
# Add MOC files
set(MOC_FILES
    include/inviwo/dataframeqt/dataframetableview.h
    include/inviwo/dataframeqt/dataframemodel.h
    include/inviwo/dataframeqt/dataframesortfilterproxy.h
    include/inviwo/dataframeqt/dataframetableprocessorwidget.h
)

# Add header files
set(HEADER_FILES
    ${MOC_FILES}
    include/inviwo/dataframeqt/dataframeqtmodule.h
    include/inviwo/dataframeqt/dataframeqtmoduledefine.h
    include/inviwo/dataframeqt/datavisualizer/dataframetablevisualizer.h
    include/inviwo/dataframeqt/processors/dataframetable.h
)
ivw_group("Header Files" ${HEADER_FILES})

# Add source files
set(SOURCE_FILES
    src/dataframemodel.cpp
    src/dataframeqtmodule.cpp
    src/dataframesortfilterproxy.cpp
    src/dataframetableprocessorwidget.cpp
    src/dataframetableview.cpp
    src/datavisualizer/dataframetablevisualizer.cpp
    src/processors/dataframetable.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

# Add moc files
if(QT_VERSION_MAJOR EQUAL 6)
    qt_wrap_cpp(MOCED_FILES ${MOC_FILES})
else()
    qt5_wrap_cpp(MOCED_FILES ${MOC_FILES})
endif()
source_group("MOC Files" FILES ${MOCED_FILES})

# Create module
ivw_create_module(NO_PCH QT ${SOURCE_FILES} ${MOCED_FILES} ${HEADER_FILES})
target_link_libraries(inviwo-module-dataframeqt PRIVATE 
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Svg
)
target_compile_definitions(inviwo-module-dataframeqt PUBLIC 
    "QT_DEPRECATED_WARNINGS_SINCE=QT_VERSION_CHECK(5, 15, 0)"
)

ivw_compile_optimize_on_target(inviwo-module-dataframeqt)

ivw_make_package(InviwoDataFrameQtModule inviwo-module-dataframeqt)
