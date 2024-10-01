# Inviwo Dome Application
project(inviwo-dome)

add_executable(inviwo-dome)
target_sources(inviwo-dome
    PUBLIC
    FILE_SET HEADERS
    TYPE HEADERS
    BASE_DIRS include
    FILES
        include/inviwo/dome/sgctmanager.h
        include/inviwo/dome/dumpfiles.h
    PRIVATE
        src/inviwodome.cpp
        src/sgctmanager.cpp
        src/dumpfiles.cpp
        "${IVW_RESOURCES_DIR}/inviwo.manifest"
)

find_package(glfw3 REQUIRED)
find_package(sgct CONFIG REQUIRED)
find_package(tracywrap CONFIG REQUIRED)
target_link_libraries(inviwo-dome
    PUBLIC
        inviwo::core
        inviwo::module-system
        inviwo::python-helper
        inviwo::tracywrap
        inviwo::module::glfw
        inviwo::module::sgct
        sgct::sgct
        glfw
        Dbghelp.lib
)

option(IVW_CFG_TRACY_MENORY_PROFILING "Enable tracy memory profiling settings" OFF)
if(IVW_CFG_TRACY_MENORY_PROFILING)
    target_compile_definitions(inviwo-dome PUBLIC IVW_CFG_TRACY_MENORY_PROFILING)
endif()

set_target_properties(inviwo-dome PROPERTIES 
    VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:inviwo-dome>
    SGCT_CONFIG_DIR ${CMAKE_CURRENT_LIST_DIR}/scripts
    TRACY_EXE ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/tools/tracy/Tracy.exe
    XCODE_GENERATE_SCHEME YES
)

ivw_folder(inviwo-dome apps)

ivw_define_standard_definitions(inviwo-dome inviwo-dome)
ivw_define_standard_properties(inviwo-dome)

# Add application to pack
ivw_default_install_targets(inviwo-dome)

get_property(packageApps CACHE IVW_PACKAGE_SELECT_APP PROPERTY STRINGS)
list(APPEND packageApps ${PROJECT_NAME})
set_property(CACHE IVW_PACKAGE_SELECT_APP PROPERTY STRINGS ${packageApps})

if(IVW_PACKAGE_SELECT_APP STREQUAL PROJECT_NAME)
    include(packaging.cmake) # Package creation
    message(STATUS "Package inviwo dome")
endif()

# Create a helper script to start inviwo-dome with multiple node instances
file(GENERATE 
    OUTPUT ${CMAKE_BINARY_DIR}/run.$<CONFIG>.ps1
    CONTENT [=[
param ([int]$nodes=2, $config='???.json', [bool]$tracy=0)
$scriptdir = '$<TARGET_PROPERTY:inviwo-dome,SGCT_CONFIG_DIR>'
if ($config -eq '???.json') {
     $config = "$scriptdir/${nodes}_nodes.json"
}
if (!(Test-Path $config)) {
    if(Test-Path $scriptdir/$config) {
        $config = "$scriptdir/$config"
    }
}

$domeexe = '$<TARGET_FILE:inviwo-dome>'
$inviwoexe = '$<TARGET_FILE:inviwo>'
Write-Host "Nodes: $nodes, Config: $config, Tracy: $tracy"
Write-Host "Run: $domeexe --config $config"
Start-Process -FilePath $inviwoexe -ArgumentList "-c --config $config"
for ( $i = 1; $i -lt $nodes; $i++ ) {
    Write-Host "Run: $domeexe --config $config --local $i --client"
    Start-Process -FilePath $domeexe -ArgumentList "--config $config --local $i --client"
}
if ($tracy) {
    $tracyexe = '$<TARGET_PROPERTY:inviwo-dome,TRACY_EXE>'
    for( $i = 0; $i -lt $nodes; $i++ ) {
        $port = $i + 8086
        Write-Host "Run: $tracyexe -a localhost -p $port"
        Start-Process -FilePath $tracyexe -ArgumentList "-a localhost -p $port"
    }
}
]=]
)

