project(BaseBenchmarks)

set(SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/benchmain.cpp)
ivw_group("Source Files" ${SOURCE_FILES})

# Create application
add_executable(base-benchmark MACOSX_BUNDLE WIN32 ${SOURCE_FILES})
find_package(benchmark CONFIG REQUIRED)
target_link_libraries(base-benchmark 
    PUBLIC 
        benchmark::benchmark
        inviwo::module::base
)
set_target_properties(base-benchmark PROPERTIES FOLDER benchmarks)

# Define defintions and properties
ivw_define_standard_properties(base-benchmark)
ivw_define_standard_definitions(base-benchmark base-benchmark)
