# Following
# http://stackoverflow.com/a/29214327/3214598

file(GLOB_RECURSE ignore_files ./ignore/*)

set(HEADER_FILES 
    ${ignore_files}
    ${IVW_EXTENSIONS_DIR}/warn/pop
    ${IVW_EXTENSIONS_DIR}/warn/push
)
set(TEXT_FILES 
    ${IVW_EXTENSIONS_DIR}/warn/warnings.md
)

ivw_group("Header Files" ${HEADER_FILES})
ivw_group("Text Files" ${TEXT_FILES})

source_group("CMake Files" FILES ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt)

#add_library(moduleA INTERFACE) # 'moduleA' is an INTERFACE pseudo target

# From here, the target 'moduleA' can be customised
#target_include_directories(moduleA ...) # Transitively forwarded
#install(TARGETS moduleA ...)

#  HACK: have the files showing in the IDE, under the name 'flags'
add_custom_target(warn SOURCES ${HEADER_FILES} ${TEXT_FILES})

# Creates VS folder structure
ivw_folder(warn ext)