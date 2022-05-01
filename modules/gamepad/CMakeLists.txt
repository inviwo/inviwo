ivw_module(Gamepad)

#~ find_package(Qt5Gamepad QUIET REQUIRED)
# https://doc-snapshots.qt.io/qt6-dev/cmake-qt5-and-qt6-compatibility.html#supporting-older-qt-5-versions   
find_package(QT NAMES Qt6 Qt5 COMPONENTS Gamepad REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Gamepad REQUIRED)

set(HEADER_FILES
    include/inviwo/gamepad/gamepadmodule.h
    include/inviwo/gamepad/gamepadmoduledefine.h
    include/inviwo/gamepad/processors/gamepadbuttonsswitches.h
    include/inviwo/gamepad/processors/gamepadcamera.h
    include/inviwo/gamepad/processors/gamepadinput.h
    include/inviwo/gamepad/processors/gamepadordinals.h
    include/inviwo/gamepad/properties/gamepadtextproperty.h
    include/inviwo/gamepad/properties/gamepadvaluesproperty.h
)
ivw_group("Header Files" ${HEADER_FILES})

set(SOURCE_FILES
    src/gamepadmodule.cpp
    src/processors/gamepadbuttonsswitches.cpp
    src/processors/gamepadcamera.cpp
    src/processors/gamepadinput.cpp
    src/processors/gamepadordinals.cpp
    src/properties/gamepadtextproperty.cpp
    src/properties/gamepadvaluesproperty.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

target_link_libraries(inviwo-module-gamepad PRIVATE
    Qt${QT_VERSION_MAJOR}::Gamepad
)

ivw_qt_add_to_install(gamepad Qt${QT_VERSION_MAJOR}Gamepad)

# Add shader directory to install package
#ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/glsl)
