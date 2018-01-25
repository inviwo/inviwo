option(IVW_BENCHMARKS "Generate benchmark targets" OFF)

if(IVW_BENCHMARKS)
    # Setup benchmark add_compile_options
    option(BENCHMARK_ENABLE_TESTING "Enable testing of the benchmark library." OFF)
    option(BENCHMARK_ENABLE_INSTALL "Enable installation of benchmark. (Projects embedding benchmark may want to turn this OFF.)" OFF)
    option(BENCHMARK_ENABLE_GTEST_TESTS "Enable building the unit tests which depend on gtest" OFF)
    set(BUILD_SHARED_LIBS OFF)
    add_subdirectory(benchmark)
    set_target_properties(benchmark PROPERTIES FOLDER ext)
endif()