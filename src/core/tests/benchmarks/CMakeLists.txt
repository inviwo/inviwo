project(BaseBenchmarks)

set(SOURCE_FILES safecstr.cpp)
ivw_group("Source Files" ${SOURCE_FILES})

# Create application
add_executable(bm-safecstr ${SOURCE_FILES})
find_package(benchmark CONFIG REQUIRED)
target_link_libraries(bm-safecstr 
    PUBLIC 
        benchmark::benchmark
        inviwo::core
)
set_target_properties(bm-safecstr PROPERTIES FOLDER benchmarks)

set_property(TARGET bm-safecstr APPEND_STRING PROPERTY LINK_FLAGS 
   " /SUBSYSTEM:CONSOLE /ENTRY:mainCRTStartup")

# Define defintions and properties
ivw_define_standard_properties(bm-safecstr)
ivw_define_standard_definitions(bm-safecstr bm-safecstr)
