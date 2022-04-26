# Inviwo Qt ApplicationBase
project(inviwo-qtapplicationbase)

# Add MOC files
set(MOC_FILES
    ${IVW_INCLUDE_DIR}/inviwo/qt/applicationbase/inviwoapplicationqt.h
)

# Add header files
set(HEADER_FILES
    ${IVW_INCLUDE_DIR}/inviwo/qt/applicationbase/qtapplicationbasemoduledefine.h
    ${MOC_FILES}
)
ivw_group("Header Files" BASE ${IVW_INCLUDE_DIR}/inviwo/qt/applicationbase/ ${HEADER_FILES})

# Add source files
set(SOURCE_FILES
    inviwoapplicationqt.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

# https://doc-snapshots.qt.io/qt6-dev/cmake-qt5-and-qt6-compatibility.html#supporting-older-qt-5-versions	
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Gui Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Widgets REQUIRED)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_wrap_cpp(MOCED_FILES ${MOC_FILES})
else()
    qt5_wrap_cpp(MOCED_FILES ${MOC_FILES})
endif()
source_group("MOC Files" FILES ${MOCED_FILES})

add_library(inviwo-qtapplicationbase ${SOURCE_FILES} ${MOCED_FILES} ${HEADER_FILES})
add_library(inviwo::qtapplicationbase ALIAS inviwo-qtapplicationbase)
target_link_libraries(inviwo-qtapplicationbase PUBLIC 
	inviwo-core
	Qt${QT_VERSION_MAJOR}::Core
	Qt${QT_VERSION_MAJOR}::Gui
	Qt${QT_VERSION_MAJOR}::Widgets
)
target_compile_definitions(inviwo-qtapplicationbase PUBLIC 
    "QT_DEPRECATED_WARNINGS_SINCE=QT_VERSION_CHECK(5, 15, 0)"
)

# Define defintions
ivw_define_standard_definitions(IVW_QTAPPLICATIONBASE inviwo-qtapplicationbase)
ivw_define_standard_properties(inviwo-qtapplicationbase)

# Make package (for other projects to find)
ivw_default_install_targets(inviwo-qtapplicationbase)
ivw_make_package(InviwoQtApplicationBase inviwo-qtapplicationbase)

