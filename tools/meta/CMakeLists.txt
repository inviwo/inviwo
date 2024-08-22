cmake_minimum_required(VERSION 3.23...3.27 FATAL_ERROR)
project(inviwo-meta VERSION 0.0.1 DESCRIPTION "CMakeList Parser" LANGUAGES CXX)

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

add_library(inviwo-meta)
add_library(inviwo::meta ALIAS inviwo-meta)

target_sources(inviwo-meta
    PUBLIC
    FILE_SET HEADERS
    TYPE HEADERS
    BASE_DIRS include
    FILES
        ${headers}
    PRIVATE
        ${sources}
)
set_target_properties(inviwo-meta PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    FOLDER tools
)
ivw_define_standard_properties(inviwo-meta)
ivw_define_standard_definitions(${PROJECT_NAME} inviwo-meta)

find_package(fmt REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(warn CONFIG REQUIRED)

# Add pegtL
find_package(pegtl CONFIG REQUIRED)
ivw_vcpkg_install(pegtl MODULE Core EXT)

# Add inja
find_package(inja CONFIG REQUIRED)
ivw_vcpkg_install(inja MODULE Core EXT)

target_link_libraries(inviwo-meta PUBLIC
    warn::warn
    fmt::fmt
    nlohmann_json::nlohmann_json
    pantor::inja
    taocpp::pegtl
)

# Added cli app
option(IVW_APP_META_CLI "Build meta cli tool" ON)
if(IVW_APP_META_CLI)
    add_executable(inviwo-meta-cli src/main.cpp)
    target_link_libraries(inviwo-meta-cli
        inviwo::meta
        inviwo::tclap
    )
    set_target_properties(inviwo-meta-cli PROPERTIES FOLDER tools)

    ivw_default_install_targets(inviwo-meta-cli)
endif()

# Add testing
option(IVW_TEST_META_BUILD_TESTS "Build meta tests" ON)
if(IVW_TEST_META_BUILD_TESTS)
    add_subdirectory(tests)
endif()

ivw_install_helper(
    TARGET inviwo-meta
    NAMESPACE inviwo
    DESTINATION inviwo
    ALIAS meta
    PACKAGES
        inja
        pegtl
)
install(
    DIRECTORY templates
    DESTINATION ${IVW_RESOURCE_INSTALL_PREFIX}tools/meta/
    COMPONENT Development
    OPTIONAL
)

ivw_make_package(InviwoMeta inviwo-meta)
