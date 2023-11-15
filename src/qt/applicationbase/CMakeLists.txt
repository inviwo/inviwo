# Inviwo Qt ApplicationBase
project(inviwo-qtapplicationbase)

# Add MOC files
set(MOC_FILES
    ${IVW_INCLUDE_DIR}/inviwo/qt/applicationbase/qtapptools.h
)

# Add header files
set(HEADER_FILES
    ${IVW_INCLUDE_DIR}/inviwo/qt/applicationbase/qtapplicationbasemoduledefine.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/applicationbase/qtlocale.h
    ${MOC_FILES}
)
ivw_group("Header Files" BASE ${IVW_INCLUDE_DIR}/inviwo/qt/applicationbase/ ${HEADER_FILES})

# Add source files
set(SOURCE_FILES
    qtapptools.cpp
    qtlocale.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
qt_wrap_cpp(MOCED_FILES ${MOC_FILES})
source_group("MOC Files" FILES ${MOCED_FILES})

add_library(inviwo-qtapplicationbase ${SOURCE_FILES} ${MOCED_FILES} ${HEADER_FILES})
add_library(inviwo::qtapplicationbase ALIAS inviwo-qtapplicationbase)
target_link_libraries(inviwo-qtapplicationbase PUBLIC 
	inviwo-core
	Qt6::Core
	Qt6::Gui
	Qt6::Widgets
)

# Define defintions
# Use ivw-qtapplicationbase instead of inviwo-qtapplicationbase
# to make standard definition naming consisten with other code, e.g., IVW_XXX
ivw_define_standard_definitions(ivw-qtapplicationbase inviwo-qtapplicationbase)
ivw_define_standard_properties(inviwo-qtapplicationbase)

# Make package (for other projects to find)
ivw_install_helper(TARGET inviwo-qtapplicationbase NAMESPACE inviwo DESTINATION inviwo)