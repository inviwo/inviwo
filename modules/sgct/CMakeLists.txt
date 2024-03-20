ivw_module(SGCT)

set(HEADER_FILES
    include/inviwo/sgct/datastructures/sgctcamera.h
    include/inviwo/sgct/io/communication.h
    include/inviwo/sgct/networksyncmanager.h
    include/inviwo/sgct/sgctmodule.h
    include/inviwo/sgct/sgctmoduledefine.h
    include/inviwo/sgct/sgctutil.h
)
ivw_group("Header Files" ${HEADER_FILES})

set(SOURCE_FILES
    src/datastructures/sgctcamera.cpp
    src/io/communication.cpp
    src/networksyncmanager.cpp
    src/sgctmodule.cpp
    src/sgctutil.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

set(SHADER_FILES
)
ivw_group("Shader Files" ${SHADER_FILES})

set(TEST_FILES
    tests/unittests/sgct-unittest-main.cpp
)
ivw_add_unittest(${TEST_FILES})
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

find_package(sgct CONFIG REQUIRED)
ivw_vcpkg_install(sgct MODULE SGCT)

target_link_libraries(inviwo-module-sgct PRIVATE sgct::sgct)

# ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/glsl)
