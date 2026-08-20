project(inviwo-cloudfiles-unittests)

set(tests
    main-test.cpp
    cloudfiles-test.cpp
)
ivw_group("Test Files" BASE . ${tests})

add_executable(inviwo-cloudfiles-unittests ${tests})

find_package(GTest CONFIG REQUIRED)
target_link_libraries(inviwo-cloudfiles-unittests
    PUBLIC
        inviwo::cloudfiles
        GTest::gtest
)

set_target_properties(inviwo-cloudfiles-unittests PROPERTIES
    FOLDER unittests
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

if(MSVC)
    target_compile_options(inviwo-cloudfiles-unittests PRIVATE /W4 /utf-8 /permissive- /diagnostics:caret)
else()
    target_compile_options(inviwo-cloudfiles-unittests PRIVATE -pedantic -Wall -Wextra -Wshadow)
endif()
