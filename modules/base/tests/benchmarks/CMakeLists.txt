project(BaseBenchmarks)

set(SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/marchingcubes.cpp)
ivw_group("Source Files" ${SOURCE_FILES})

# Create application
add_executable(bm-marchingcubes MACOSX_BUNDLE WIN32 ${SOURCE_FILES})
find_package(benchmark CONFIG REQUIRED)
target_link_libraries(bm-marchingcubes 
    PUBLIC 
        benchmark::benchmark
        inviwo::module::base
)
set_target_properties(bm-marchingcubes PROPERTIES FOLDER benchmarks)

# Define defintions and properties
ivw_define_standard_properties(bm-marchingcubes)
ivw_define_standard_definitions(bm-marchingcubes bm-marchingcubes)
