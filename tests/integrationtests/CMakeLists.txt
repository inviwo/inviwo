# Inviwo integration tests
project(inviwo-integrationtests)

# Add source files
set(SOURCE_FILES
    inviwo-integrationtests.cpp
    image-test.cpp
    inviwoapplication-test.cpp
    network-test.cpp
    processorcreation-test.cpp
    propertycreation-test.cpp
    volume-test.cpp
    shader-test.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

# Create application
add_executable(inviwo-integrationtests ${SOURCE_FILES})

find_package(GTest CONFIG REQUIRED)
target_link_libraries(inviwo-integrationtests PRIVATE 
    inviwo::core
    inviwo::module-system
    inviwo::module::opengl
    inviwo::module::glfw
    inviwo::module::base
    inviwo::testutil
    GTest::gtest
)
set_target_properties(inviwo-integrationtests PROPERTIES FOLDER integrationtests)

ivw_define_standard_definitions(inviwo-integrationtests inviwo-integrationtests)
ivw_define_standard_properties(inviwo-integrationtests)

# Tests for initialization/deinitialization of InviwoApplication 
add_subdirectory("recycle-inviwo-application")
