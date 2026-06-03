project(CoreBenchmarks LANGUAGES CXX)

ivw_benchmark(NAME bm-safecstr LIBS inviwo::core FILES safecstr.cpp)
ivw_benchmark(NAME bm-optimaltransport LIBS inviwo::core FILES optimaltransport.cpp)
