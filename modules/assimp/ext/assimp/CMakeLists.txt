
set(BUILD_FRAMEWORK            OFF CACHE INTERNAL "Build package as Mac OS X Framework bundle.")
set(ASSIMP_DOUBLE_PRECISION    OFF CACHE INTERNAL "Set to ON to enable double precision processing")
set(ASSIMP_OPT_BUILD_PACKAGES  OFF CACHE INTERNAL "Set to ON to generate CPack configuration files and packaging targets")
set(ASSIMP_INSTALL_PDB         OFF CACHE INTERNAL "Install MSVC debug files.")
set(ASSIMP_ANDROID_JNIIOSYSTEM OFF CACHE INTERNAL "Android JNI IOSystem support is active")
set(ASSIMP_NO_EXPORT           OFF CACHE INTERNAL "Disable Assimp's export functionality.")
set(ASSIMP_BUILD_ZLIB          ON  CACHE INTERNAL "Build your own zlib")
set(ASSIMP_BUILD_ASSIMP_TOOLS  OFF CACHE INTERNAL "If the supplementary tools for Assimp are built in addition to the library.")
set(ASSIMP_BUILD_SAMPLES       OFF CACHE INTERNAL "If the official samples are built as well (needs Glut).")
set(ASSIMP_BUILD_TESTS         OFF CACHE INTERNAL "If the test suite for Assimp is built in addition to the library.")
set(ASSIMP_COVERALLS           OFF CACHE INTERNAL "Enable this to measure test coverage.")
set(ASSIMP_WERROR              OFF CACHE INTERNAL "Treat warnings as errors.")
set(ASSIMP_ASAN                OFF CACHE INTERNAL "Enable AddressSanitizer.")
set(ASSIMP_UBSAN               OFF CACHE INTERNAL "Enable Undefined Behavior sanitizer.")
set(SYSTEM_IRRXML              OFF CACHE INTERNAL "Use system installed Irrlicht/IrrXML library.")
set(BUILD_DOCS                 OFF CACHE INTERNAL "Build documentation using Doxygen.")
set(INJECT_DEBUG_POSTFIX       ON  CACHE INTERNAL "Inject debug postfix in .a/.so lib names")
set(IGNORE_GIT_HASH            ON  CACHE INTERNAL "Don't call git to get the hash.")

# OpenEXR::IlmImf needs zlib:
set(ZLIB_FOUND ON)
set(ZLIB_LIBRARIES ZLIB::ZLIB)
get_target_property(ZLIB_INCLUDE_DIR ZLIB::ZLIB INTERFACE_INCLUDE_DIRECTORIES)

add_subdirectory(assimp)

ivw_move_targets_in_dir_to_folder(${CMAKE_CURRENT_SOURCE_DIR} ext/assimp)
ivw_get_targets_in_dir_recursive(targets ${CMAKE_CURRENT_SOURCE_DIR})
ivw_suppress_compiler_warnings(${targets})
    
# debug
#ivw_print_targets_in_dir_recursive(${CMAKE_CURRENT_SOURCE_DIR})
