set(test_name "inviwo-integrationtests-recycle-inviwo-application")
ivw_debug_message(STATUS "create integrationtests: recycle-inviwo-application")

# Inviwo integration tests
project(${test_name})

# Add source files
set(SOURCE_FILES
    recycle-inviwo-application-tests-main.cpp
    recycle-inviwo-application-tests.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

# Create application
add_executable(${test_name} ${SOURCE_FILES})

find_package(GTest CONFIG REQUIRED)
target_link_libraries(${test_name} PRIVATE 
    inviwo::core
    inviwo::module::base
    inviwo::testutil
    GTest::gtest
)
set_target_properties(${test_name} PROPERTIES FOLDER integrationtests)

ivw_define_standard_definitions(${test_name} ${test_name})
ivw_define_standard_properties(${test_name})

