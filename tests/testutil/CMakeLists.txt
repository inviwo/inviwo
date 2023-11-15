project(inviwo-testutil)

add_library(inviwo-testutil STATIC)
add_library(inviwo::testutil ALIAS inviwo-testutil)
target_sources(inviwo-testutil
    PUBLIC
    FILE_SET HEADERS
    TYPE HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        include/inviwo/testutil/configurablegtesteventlistener.h
        include/inviwo/testutil/zipmatcher.h
    PRIVATE
        src/configurablegtesteventlistener.cpp
        src/zipmatcher.cpp
)


find_package(GTest CONFIG REQUIRED)
find_package(warn CONFIG REQUIRED)
target_link_libraries(inviwo-testutil PUBLIC
    warn::warn 
    GTest::gtest
    GTest::gmock
)

ivw_define_standard_properties(inviwo-testutil)
ivw_define_standard_definitions(inviwo-testutil inviwo-testutil)

ivw_make_package(inviwo-testutil inviwo-testutil)
ivw_append_install_list(GLOBAL)
ivw_install_helper(
    TARGET inviwo-testutil
    ALIAS testutil
    NAMESPACE inviwo
    DESTINATION inviwo
    PACKAGES
        GTest
        warn
)
