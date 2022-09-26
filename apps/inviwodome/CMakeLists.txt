#--------------------------------------------------------------------
# Inviwo Dome Application
project(inviwo-dome)

set(HEADER_FILES
    include/inviwo/dome/sgctmanager.h
    include/inviwo/dome/workspacewatcher.h
    include/inviwo/dome/dumpfiles.h
)
ivw_group("Header Files" BASE include/inviwo/dome ${HEADER_FILES})

set(SOURCE_FILES
    src/inviwodome.cpp
    src/sgctmanager.cpp
    src/workspacewatcher.cpp
    src/dumpfiles.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

ivw_retrieve_all_modules(enabled_modules)
# Remove Qt stuff from list
foreach(module ${enabled_modules})
    string(TOUPPER ${module} u_module)
    if(u_module MATCHES "QT+")
        list(REMOVE_ITEM enabled_modules ${module})
    endif()
endforeach()

add_executable(inviwo-dome ${HEADER_FILES} ${SOURCE_FILES})
target_include_directories(inviwo-dome PUBLIC include)

find_package(glfw3 REQUIRED)
find_package(sgct CONFIG REQUIRED)
find_package(Tracy CONFIG REQUIRED)
    
target_link_libraries(inviwo-dome 
    PRIVATE
        Tracy::TracyClient
        sgct::sgct
        glfw
        inviwo::module::glfw
        inviwo::module::sgct
        Dbghelp.lib
)

set_target_properties(inviwo-dome PROPERTIES 
    VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:inviwo-dome>
    SGCT_CONFIG_DIR ${CMAKE_CURRENT_LIST_DIR}/scripts
    TRACY_EXE ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/tools/tracy/Tracy.exe
)

ivw_configure_application_module_dependencies(inviwo-dome ${enabled_modules})
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


file(GENERATE 
    OUTPUT ${IVW_BINARY_DIR}/run.$<CONFIG>.ps1
    CONTENT [=[
param ([int]$nodes=2, $config='???.xml', [bool]$tracy=0)
$scriptdir = '$<TARGET_PROPERTY:inviwo-dome,SGCT_CONFIG_DIR>'
if ($config -eq '???.xml') {
     $config = "$scriptdir/${nodes}_nodes.xml"
}
if (!(Test-Path $config)) {
    if(Test-Path $scriptdir/$config) {
        $config = "$scriptdir/$config"
    }
}

Write-Host "Nodes: $nodes, Config: $config, Tracy: $tracy"
Write-Host "Run: $<TARGET_FILE:inviwo-dome> --config $config &"
$<TARGET_FILE:inviwo-dome> --config $config &
for ( $i = 1; $i -lt $nodes; $i++ ) {
    Write-Host "Run: $<TARGET_FILE:inviwo-dome> --config $config --local $i --client &"
    $<TARGET_FILE:inviwo-dome> --config $config --local $i --client &
}
if ($tracy) {
    for( $i = 0; $i -lt $nodes; $i++ ) {
        $port = $i + 8086
        Write-Host "Run: $<TARGET_PROPERTY:inviwo-dome,TRACY_EXE> -a localhost -p $port"
        $<TARGET_PROPERTY:inviwo-dome,TRACY_EXE> -a localhost -p $port &
    }
}
]=]
)

