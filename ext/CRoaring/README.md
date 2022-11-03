# CRoaring - Portable Roaring bitmaps in C (and C++)

As the original git repository [1] of CRoaring contains >250 MiB of benchmark data, the CRoaring subfolder is a git clone of commit [b3d5ecc](https://github.com/RoaringBitmap/CRoaring/commit/b3d5ecc480fee03a2f3dc36283d0f1587d211b4e) with the folder `benchmark` being removed. Hence, the roaring tests will not work (CMake option `ENABLE_ROARING_TESTS`).

Minor modifications to `CMakeLists.txt` were necessary:
1. set minimum CMake version `cmake_minimum_required(VERSION 2.9...3.18)`
2. enclose setting `TEST_DATA_DIR`, `BENCHMARK_DATA_DIR`, and `configure_file` for the tests with an if statement `if(ENABLE_ROARING_TESTS)`
3. copy cpp/* into include/ as is done in the installer, to make "#include <roaring/roaring.hh>" work

[1] https://github.com/RoaringBitmap/CRoaring
