cmake_minimum_required(VERSION 3.16...3.22 FATAL_ERROR)
project(inviwo-meta VERSION 0.0.1 DESCRIPTION "CMakeList Parser" LANGUAGES CXX)

if(NOT cxx_std_17 IN_LIST CMAKE_CXX_COMPILE_FEATURES)
    message(STATUS "Will not build Inviwo meta since the used compiler does not have c++17 support")
    return()
endif()

find_package(Filesystem)
if(NOT Filesystem_FOUND) 
    message(STATUS "Will not build Inviwo meta since the used compiler does not have c++ filesystem support")
    return()
endif()

include(CheckIncludeFileCXX)
check_include_file_cxx(variant found_variant)
if(NOT found_variant)
    message(STATUS "Will not build Inviwo meta since the used compiler does not have c++ variant support")
    return()
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER cmake)

set(headers
    include/inviwo/meta/cmake/cmakefile.hpp
    include/inviwo/meta/cmake/grammar.hpp
    include/inviwo/meta/creator.hpp
    include/inviwo/meta/filetemplates.hpp
    include/inviwo/meta/inviwometadefine.hpp
    include/inviwo/meta/inviwomodule.hpp 
    include/inviwo/meta/inviwomoduleconf.hpp
    include/inviwo/meta/inviwomoduleregistration.hpp 
    include/inviwo/meta/iter/filteriterator.hpp
    include/inviwo/meta/iter/iterutil.hpp
    include/inviwo/meta/iter/range.hpp
    include/inviwo/meta/iter/transformiterator.hpp
    include/inviwo/meta/paths.hpp
    include/inviwo/meta/util.hpp
    include/inviwo/meta/includetools.hpp
)
ivw_group("Header Files" BASE include/inviwo/meta ${headers})

set(sources
    src/cmake/cmakefile.cpp
    src/creator.cpp
    src/filetemplates.cpp
    src/inviwomodule.cpp
    src/inviwomoduleconf.cpp
    src/inviwomoduleregistration.cpp
    src/paths.cpp
    src/util.cpp
    src/includetools.cpp
)
ivw_group("Source Files" BASE src ${sources})

add_library(inviwo-meta ${sources} ${headers})
add_library(inviwo::meta ALIAS inviwo-meta)
target_include_directories(inviwo-meta PUBLIC include)
target_compile_features(inviwo-meta PUBLIC cxx_std_20)
set_target_properties(inviwo-meta PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF FOLDER tools)
ivw_define_standard_properties(inviwo-meta)
ivw_define_standard_definitions(${PROJECT_NAME} inviwo-meta)

find_package(fmt REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

target_link_libraries(inviwo-meta PUBLIC
    std::filesystem 
    inviwo::warn
    fmt::fmt
    nlohmann_json::nlohmann_json
)

# Add pegtL 
option(IVW_USE_EXTERNAL_PEGTL "Pegtl is provided externaly" OFF)
if(NOT IVW_USE_EXTERNAL_PEGTL)
    option(PEGTL_BUILD_TESTS "Build test programs" OFF)
    option(PEGTL_BUILD_EXAMPLES "Build example programs" OFF)
    add_subdirectory(ext/pegtl)
    ivw_make_package(pegtl pegtl)
    ivw_move_targets_in_dir_to_folder(ext/pegtl ext/pegtl)
    ivw_register_license_file(NAME "PEGTL" VERSION 3.2.7 target taocpp::pegtl MODULE Core EXT
        URL "https://github.com/taocpp/PEGTL"
        TYPE "The MIT License"
        FILES ${IVW_TOOLS_DIR}/meta/ext/pegtl/LICENSE
    )
else()
    find_package(pegtl CONFIG REQUIRED)
    ivw_vcpkg_install(pegtl MODULE Core EXT)
endif()
target_link_libraries(inviwo-meta PUBLIC taocpp::pegtl)

# Add inja
option(IVW_USE_EXTERNAL_INJA "Inja is provided externaly" OFF)
if(NOT IVW_USE_EXTERNAL_INJA)
    option(INJA_BUILD_TESTS "" OFF)
    option(INJA_EXPORT "" OFF)
    option(INJA_INSTALL "" OFF)
    option(INJA_USE_EMBEDDED_JSON "" OFF)
    option(BUILD_BENCHMARK "" OFF)
    add_subdirectory(ext/inja)
    ivw_make_package(inja inja)
    ivw_register_license_file(NAME "inja" VERSION 3.3.0 target inja MODULE Core EXT
        URL "https://github.com/pantor/inja"
        TYPE "The MIT License"
        FILES ${IVW_TOOLS_DIR}/meta/ext/inja/LICENSE
    )
else()
    find_package(inja CONFIG REQUIRED)
    ivw_vcpkg_install(inja MODULE Core EXT)
endif()
target_link_libraries(inviwo-meta PUBLIC pantor::inja)

# Added cli app
option(IVW_APP_META_CLI "Build meta cli tool" ON)
if(IVW_APP_META_CLI)
    add_executable(inviwo-meta-cli src/main.cpp)
    target_link_libraries(inviwo-meta-cli 
        inviwo::meta
        inviwo::tclap
    )
    set_target_properties(inviwo-meta-cli PROPERTIES FOLDER tools)
endif()

# Add testing
option(IVW_TEST_META_BUILD_TESTS "Build meta tests" ON)
if(IVW_TEST_META_BUILD_TESTS) 
    add_subdirectory(tests)
endif()

ivw_default_install_targets(inviwo-meta)

ivw_make_package(InviwoMeta inviwo-meta)