#--------------------------------------------------------------------
# Inviwo Qt ApplicationBase
ivw_project(inviwo-qtapplicationbase)

#--------------------------------------------------------------------
# Add MOC files
set(MOC_FILES
    ${IVW_INCLUDE_DIR}/inviwo/qt/applicationbase/inviwoapplicationqt.h
)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${IVW_INCLUDE_DIR}/inviwo/qt/applicationbase/qtapplicationbasemoduledefine.h
    ${MOC_FILES}
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    inviwoapplicationqt.cpp
)

ivw_group("Source Files" ${SOURCE_FILES})

#--------------------------------------------------------------------
# Create library
find_package(Qt5Core QUIET REQUIRED)
find_package(Qt5Gui QUIET REQUIRED)
find_package(Qt5Widgets QUIET REQUIRED)

ivw_qt_wrap_cpp(MOC_FILES ${MOC_FILES})
source_group("MOC Files" FILES ${MOC_FILES})
add_library(inviwo-qtapplicationbase ${SOURCE_FILES} ${MOC_FILES} ${HEADER_FILES})
target_link_libraries(inviwo-qtapplicationbase PRIVATE Qt5::Core Qt5::Gui Qt5::Widgets)

#--------------------------------------------------------------------
# Define defintions
ivw_define_standard_definitions(IVW_QTAPPLICATIONBASE inviwo-qtapplicationbase)
ivw_define_standard_properties(inviwo-qtapplicationbase)
ivw_define_qt_definitions()

# Add core dependencies to this list
ivw_add_dependencies(InviwoCore)

#--------------------------------------------------------------------
# Make package (for other projects to find)
ivw_set_cpack_name(qt_applicationbase)
ivw_make_package(InviwoQtApplicationBase inviwo-qtapplicationbase)

ivw_default_install_comp_targets(qt_applicationbase inviwo-qtapplicationbase)
ivw_qt_add_to_install(inviwo-qtapplicationbase Qt5Core Qt5Gui Qt5Widgets)