# Inviwo DataFrameQt Module
ivw_module(DataFrameQt)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Svg)

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
qt_wrap_cpp(MOCED_FILES ${MOC_FILES})
source_group("MOC Files" FILES ${MOCED_FILES})

# Create module
ivw_create_module(NO_PCH QT ${SOURCE_FILES} ${MOCED_FILES} ${HEADER_FILES})
target_link_libraries(inviwo-module-dataframeqt PUBLIC 
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Svg
)

ivw_compile_optimize_on_target(inviwo-module-dataframeqt)
