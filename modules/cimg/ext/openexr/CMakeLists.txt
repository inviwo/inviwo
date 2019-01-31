#Adapted settings from /openexr/cmake/OpenEXRSettings.cmake

set(OPENEXR_BUILD_SHARED         ${BUILD_SHARED_LIBS}  CACHE INTERNAL "Build Shared Libraries")

set(OPENEXR_BUILD_ILMBASE        ON  CACHE INTERNAL "Build IlmBase")
set(OPENEXR_BUILD_OPENEXR        ON  CACHE INTERNAL "Build OpenEXR")
set(OPENEXR_BUILD_PYTHON_LIBS    OFF CACHE INTERNAL "Build the Python bindings")
set(OPENEXR_BUILD_VIEWERS        OFF CACHE INTERNAL "Build the viewers")
set(OPENEXR_BUILD_TESTS          OFF CACHE INTERNAL "Enable the tests")
set(OPENEXR_RUN_FUZZ_TESTS       OFF CACHE INTERNAL "Run damaged-input tests")
set(OPENEXR_BUILD_UTILS          OFF CACHE INTERNAL "Build the utility programs")
set(OPENEXR_BUILD_STATIC         OFF CACHE INTERNAL "Build Static Libraries")
set(OPENEXR_NAMESPACE_VERSIONING ON  CACHE INTERNAL "Use Namespace Versioning")
set(OPENEXR_FORCE_CXX03          OFF CACHE INTERNAL "Force CXX03")
set(OPENEXR_PYTHON_MAJOR         "2" CACHE INTERNAL "Python major version")
set(OPENEXR_PYTHON_MINOR         "7" CACHE INTERNAL "Python minor version")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/openexr/cmake)
include(OpenEXRVersion)
project(OpenEXR VERSION ${OPENEXR_VERSION})
include(OpenEXRSettings)

# OpenEXR::IlmImf needs zlib:
set(ZLIB_LIBRARIES ZLIB::ZLIB)

add_subdirectory(openexr/IlmBase)
add_subdirectory(openexr/OpenEXR)

ivw_move_targets_in_dir_to_folder(${CMAKE_CURRENT_SOURCE_DIR} ext/openexr)

ivw_get_targets_in_dir_recursive(targets ${CMAKE_CURRENT_SOURCE_DIR})
ivw_suppress_compiler_warnings(${targets})

# debug
#ivw_print_targets_in_dir_recursive(${CMAKE_CURRENT_SOURCE_DIR})
