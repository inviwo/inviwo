# Following
# http://stackoverflow.com/a/29214327/3214598

set(HEADER_FILES 
    ${IVW_EXTENSIONS_DIR}/tinydir/tinydir.h
)
ivw_group("Header Files" ${HEADER_FILES})
source_group("CMake Files" FILES ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt)

add_library(tinydir INTERFACE) # 'moduleA' is an INTERFACE pseudo target
add_library(inviwo::tinydir ALIAS tinydir)
target_include_directories(tinydir INTERFACE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>
    #$<INSTALL_INTERFACE:???>
)

# From here, the target 'moduleA' can be customised
#target_include_directories(moduleA ...) # Transitively forwarded
#install(TARGETS moduleA ...)

#  HACK: have the files showing in the IDE, under the name 'tinydircpp'
add_custom_target(tinydircpp SOURCES ${HEADER_FILES})

# Creates VS folder structure
ivw_folder(tinydircpp ext)

ivw_default_install_comp_targets(core tinydir)
ivw_make_package(tinydir tinydir)