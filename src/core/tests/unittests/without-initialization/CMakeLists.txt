# Check if there are any tests
if(NOT IVW_TEST_UNIT_TESTS)
    return()
endif()

set(test_name "inviwo-unittests-core-without-initialization")
ivw_debug_message(STATUS "create unittests: core-without-initialization")

# Inviwo integration tests
project(${test_name})

# Add source files
set(SOURCE_FILES
    inviwo-core-unittest-without-initialization-main.cpp
    init-deinit-tests.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

set(enabled_modules
    InviwoCoreModule
)

# Create application
add_executable(${test_name} ${SOURCE_FILES})

find_package(GTest CONFIG REQUIRED)
target_link_libraries(${test_name} PRIVATE 
    inviwo::core
    inviwo::module::base
    inviwo::testutil
    GTest::gtest
)
set_target_properties(${test_name} PROPERTIES FOLDER unittests)

ivw_configure_application_module_dependencies(${test_name} ${enabled_modules})
ivw_define_standard_definitions(${test_name} ${test_name})
ivw_define_standard_properties(${test_name})
# Add run on build command
if(IVW_TEST_UNIT_TESTS_RUN_ON_BUILD)
    add_custom_command(TARGET "${test_name}" POST_BUILD COMMAND "${test_name}" 
                        COMMENT "Running unittest for: ${test_name}" VERBATIM)
endif()
