cmake_minimum_required(VERSION 3.23...3.27 FATAL_ERROR)
project(inviwo-cloudfiles VERSION 0.0.1
    DESCRIPTION "Query and control OneDrive/iCloud on-demand files" LANGUAGES CXX)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(headers
    include/inviwo/cloudfiles/cloudfiles.hpp
    include/inviwo/cloudfiles/cloudfilesdefine.hpp
)
ivw_group("Header Files" BASE include/inviwo/cloudfiles ${headers})

set(sources
    src/cloudfiles.cpp
    src/cloudfilesdetail.hpp
)
# Exactly one platform backend is compiled.
if(WIN32)
    list(APPEND sources src/cloudfiles_win.cpp)
elseif(APPLE)
    enable_language(OBJCXX)
    list(APPEND sources src/cloudfiles_mac.mm)
else()
    list(APPEND sources src/cloudfiles_generic.cpp)
endif()
ivw_group("Source Files" BASE src ${sources})

add_library(inviwo-cloudfiles)
add_library(inviwo::cloudfiles ALIAS inviwo-cloudfiles)

target_sources(inviwo-cloudfiles
    PUBLIC
    FILE_SET HEADERS
    TYPE HEADERS
    BASE_DIRS include
    FILES
        ${headers}
    PRIVATE
        ${sources}
)
set_target_properties(inviwo-cloudfiles PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    FOLDER tools
    XCODE_GENERATE_SCHEME YES
)

ivw_define_standard_properties(inviwo-cloudfiles)
ivw_define_standard_definitions(${PROJECT_NAME} inviwo-cloudfiles)

find_package(fmt REQUIRED)
target_link_libraries(inviwo-cloudfiles PRIVATE fmt::fmt)

if(WIN32)
    # Cloud Filter API: CfHydratePlaceholder / CfDehydratePlaceholder
    target_link_libraries(inviwo-cloudfiles PRIVATE cldapi)
elseif(APPLE)
    target_link_libraries(inviwo-cloudfiles PRIVATE "-framework Foundation")
endif()

# Command line tool
option(IVW_APP_CLOUDFILES_CLI "Build cloudfiles cli tool" ON)
if(IVW_APP_CLOUDFILES_CLI)
    add_executable(inviwo-cloudfiles-cli src/main.cpp)
    target_link_libraries(inviwo-cloudfiles-cli PRIVATE
        inviwo::cloudfiles
        inviwo::tclap
        fmt::fmt
    )
    set_target_properties(inviwo-cloudfiles-cli PROPERTIES
        FOLDER tools
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
    )
    ivw_default_install_targets(inviwo-cloudfiles-cli)
endif()

# Tests
option(IVW_TEST_CLOUDFILES_BUILD_TESTS "Build cloudfiles tests" ON)
if(IVW_TEST_CLOUDFILES_BUILD_TESTS)
    add_subdirectory(tests)
endif()

ivw_install_helper(
    TARGET inviwo-cloudfiles
    NAMESPACE inviwo
    DESTINATION inviwo
    ALIAS cloudfiles
)

ivw_make_package(InviwoCloudFiles inviwo-cloudfiles)
