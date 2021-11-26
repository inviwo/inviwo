project(CoreBenchmarks LANGUAGES CXX)

ivw_benchmark(NAME bm-safecstr LIBS inviwo::core FILES safecstr.cpp)
