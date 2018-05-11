#--------------------------------------------------------------------
# Inviwo PythonQt Module
ivw_module(Python3Qt)

find_package(Qt5Core QUIET REQUIRED)
find_package(Qt5Gui QUIET REQUIRED)
find_package(Qt5Widgets QUIET REQUIRED)

#--------------------------------------------------------------------
# Add MOC files
set(MOC_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/pythoneditorwidget.h
)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/pythonmenu.h
    ${CMAKE_CURRENT_SOURCE_DIR}/python3qtmodule.h
    ${CMAKE_CURRENT_SOURCE_DIR}/python3qtmoduledefine.h
    ${MOC_FILES}
)
ivw_group("Header Files" ${HEADER_FILES})

qt5_wrap_cpp(MOCED_FILES ${MOC_FILES})
source_group("MOC Files" FILES ${MOCED_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/pythoneditorwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pythonmenu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/python3qtmodule.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

# Create module
ivw_create_module(NO_PCH ${SOURCE_FILES} ${MOCED_FILES} ${HEADER_FILES})
target_link_libraries(inviwo-module-python3qt PRIVATE Qt5::Core Qt5::Gui Qt5::Widgets)

ivw_compile_optimize_on_target(inviwo-module-python3qt)

ivw_make_package(InviwoPython3QtModule inviwo-module-python3qt)
