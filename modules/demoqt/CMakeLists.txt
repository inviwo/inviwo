#--------------------------------------------------------------------
# Inviwo DemoQt Module
ivw_module(DemoQt)

find_package(Qt5Core QUIET REQUIRED)
find_package(Qt5Gui QUIET REQUIRED)
find_package(Qt5Widgets QUIET REQUIRED)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/demonavigatordockwidgetqt.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/demonavigatordockwidgetqt.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

# Add some qt specific things
source_group("Resource Files" FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/demo.qrc)
qt5_add_resources(QRC_FILE "${CMAKE_CURRENT_SOURCE_DIR}/resources/demo.qrc")

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${QRC_FILE})
target_link_libraries(inviwo-module-demoqt PRIVATE Qt5::Core Qt5::Gui Qt5::Widgets)
ivw_make_package(InviwoDemoQtModule inviwo-module-demoqt)