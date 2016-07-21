# Following
# http://stackoverflow.com/a/29214327/3214598

set(HEADER_FILES 
    ${IVW_EXTENSIONS_DIR}/flags/allow_flags.h
    ${IVW_EXTENSIONS_DIR}/flags/flags.h
    ${IVW_EXTENSIONS_DIR}/flags/flagsfwd.h
    ${IVW_EXTENSIONS_DIR}/flags/iterator.h
)
ivw_group("Header Files" ${HEADER_FILES})
source_group("CMake Files" FILES ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt)

#add_library(moduleA INTERFACE) # 'moduleA' is an INTERFACE pseudo target

# From here, the target 'moduleA' can be customised
#target_include_directories(moduleA ...) # Transitively forwarded
#install(TARGETS moduleA ...)

#  HACK: have the files showing in the IDE, under the name 'flags'
add_custom_target(flags SOURCES ${HEADER_FILES})

# Creates VS folder structure
ivw_folder(flags ext)