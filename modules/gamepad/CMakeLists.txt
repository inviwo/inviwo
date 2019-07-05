#--------------------------------------------------------------------
# Inviwo gamepad Module
ivw_module(Gamepad)

find_package(Qt5Gamepad QUIET REQUIRED)


#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    boolcontrolledproperty.h
    cameracontrolledproperty.h
    gamepadcontrolledproperty.h
    gamepadcontroller.h
    gamepadmodule.h
    gamepadmoduledefine.h
    modeproperty.h
    ordinalcontrolledproperty.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    boolcontrolledproperty.cpp
    cameracontrolledproperty.cpp
    gamepadcontrolledproperty.cpp
    gamepadcontroller.cpp
    gamepadmodule.cpp
    modeproperty.cpp
    ordinalcontrolledproperty.cpp
)
ivw_group("Sources" ${SOURCE_FILES} ${HEADER_FILES})


#--------------------------------------------------------------------
# Add shaders
set(SHADER_FILES
    #${CMAKE_CURRENT_SOURCE_DIR}/glsl/gamepadprocessor.frag
)
ivw_group("Shader Files" ${SHADER_FILES})


#--------------------------------------------------------------------
# Add Unittests
set(TEST_FILES
	#${CMAKE_CURRENT_SOURCE_DIR}/tests/unittests/gamepad-test.cpp
)
ivw_add_unittest(${TEST_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

target_link_libraries(inviwo-module-gamepad PRIVATE Qt5::Gamepad)
ivw_qt_add_to_install(gamepad Qt5Gamepad)


#--------------------------------------------------------------------
# Add shader directory to pack
# ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/glsl)
