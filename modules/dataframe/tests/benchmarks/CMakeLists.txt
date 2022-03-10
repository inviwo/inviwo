project(DataFrameBenchmarks LANGUAGES CXX)

ivw_benchmark(NAME bm-dataframejoin LIBS inviwo::core inviwo::module::base inviwo::module::dataframe FILES join.cpp)
