#--------------------------------------------------------------------
# Inviwo Animation Module
ivw_module(Animation)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/animationcontroller.h
    ${CMAKE_CURRENT_SOURCE_DIR}/animationcontrollerobserver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/animationmanager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/animationmodule.h
    ${CMAKE_CURRENT_SOURCE_DIR}/animationsupplier.h
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/animation.h
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/animationobserver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/animationstate.h
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/animationtime.h
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/basekeyframe.h
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/basekeyframesequence.h
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/basetrack.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interpolation/constantinterpolation.h
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/controlkeyframe.h
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/controlkeyframesequence.h
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/controltrack.h
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/easing.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interpolation/interpolation.h
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/keyframe.h
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/keyframeobserver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/keyframesequence.h
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/keyframesequenceobserver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interpolation/linearinterpolation.h
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/propertytrack.h
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/track.h
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/trackobserver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/valuekeyframe.h
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/valuekeyframesequence.h
    ${CMAKE_CURRENT_SOURCE_DIR}/demo/democontroller.h
    ${CMAKE_CURRENT_SOURCE_DIR}/demo/democontrollerobserver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/factories/interpolationfactory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/factories/interpolationfactoryobject.h
    ${CMAKE_CURRENT_SOURCE_DIR}/factories/trackfactory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/factories/trackfactoryobject.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/animationcontroller.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/animationcontrollerobserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/animationmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/animationmodule.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/animationsupplier.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/animation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/animationobserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/animationstate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/animationtime.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/basekeyframe.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/basekeyframesequence.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/basetrack.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/controlkeyframe.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/controlkeyframesequence.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/controltrack.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/easing.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/interpolation/interpolation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/keyframe.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/keyframeobserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/keyframesequence.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/keyframesequenceobserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/propertytrack.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/track.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/trackobserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/valuekeyframe.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/valuekeyframesequence.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/demo/democontroller.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/demo/democontrollerobserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/factories/interpolationfactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/factories/interpolationfactoryobject.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/factories/trackfactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/factories/trackfactoryobject.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

#--------------------------------------------------------------------
# Add Unittests
set(TEST_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unittests/animation-unittest-main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unittests/track-test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unittests/easing-test.cpp
)
ivw_add_unittest(${TEST_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(NO_PCH ${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

#--------------------------------------------------------------------
# Add shader directory to pack
# ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/glsl)