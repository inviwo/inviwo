project(VolumeBenchmarks)

set(SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/regionmap.cpp)
ivw_group("Source Files" ${SOURCE_FILES})

# Create application
add_executable(bm-regionmap MACOSX_BUNDLE WIN32 ${SOURCE_FILES})
find_package(benchmark CONFIG REQUIRED)
target_link_libraries(bm-regionmap
    PUBLIC 
        benchmark::benchmark
        inviwo::module::volume
)
set_target_properties(bm-regionmap PROPERTIES FOLDER benchmarks)

# Define defintions and properties
ivw_define_standard_properties(bm-regionmap)
ivw_define_standard_definitions(bm-regionmap bm-regionmap)
