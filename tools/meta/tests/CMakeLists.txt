project(inviwo-meta-unittests)

set(tests
    main-test.cpp
    grammar-test.cpp
    cmakefile-test.cpp
    filteriterator-test.cpp
    transformiterator-test.cpp
    range-test.cpp
)
ivw_group("Test Files" BASE . ${tests})

add_executable(inviwo-meta-unittests ${tests})

find_package(GTest CONFIG REQUIRED)
target_link_libraries(inviwo-meta-unittests 
    PUBLIC
        inviwo::meta
        GTest::gtest
)

if(MSVC)
    target_compile_options(inviwo-meta-unittests PRIVATE /W4 /utf-8 /permissive- /diagnostics:caret)
else()
    target_compile_options(inviwo-meta-unittests PRIVATE -pedantic -Wall -Wextra -Wshadow)
endif()