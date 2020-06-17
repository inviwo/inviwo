set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK ON CACHE BOOL "Builds the googlemock subproject" FORCE)
option(INSTALL_GTEST "Enable installation of googletest. (Projects embedding googletest may want to turn this OFF.)" OFF)

add_subdirectory(gtest)

ivw_folder(gtest ext)
ivw_folder(gtest_main ext)
ivw_folder(gmock ext)
ivw_folder(gmock_main ext)

if(BUILD_SHARED_LIBS)
    target_compile_definitions(gtest PRIVATE -DGTEST_CREATE_SHARED_LIBRARY=1)
    target_compile_definitions(gtest INTERFACE -DGTEST_LINKED_AS_SHARED_LIBRARY=1)
endif()

ivw_make_package(GTest gtest gtest_main gmock gmock_main)
add_library(GTest::gtest ALIAS gtest)
add_library(GTest::gtest_main ALIAS gtest_main)
add_library(GTest::gmock ALIAS gmock)
add_library(GTest::gmock_main ALIAS gmock_main)
