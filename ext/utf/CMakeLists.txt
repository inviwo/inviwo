# Following
# http://stackoverflow.com/a/29214327/3214598

set(HEADER_FILES 
	${IVW_EXTENSIONS_DIR}/utf/utfcpp/source/utf8.h
    ${IVW_EXTENSIONS_DIR}/utf/utfcpp/source/utf8/checked.h
    ${IVW_EXTENSIONS_DIR}/utf/utfcpp/source/utf8/core.h
    ${IVW_EXTENSIONS_DIR}/utf/utfcpp/source/utf8/unchecked.h
)
set (DOC_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/LICENSE 
	${CMAKE_CURRENT_SOURCE_DIR}/README.md
)

ivw_include_directories(${IVW_EXTENSIONS_DIR}/utf/utfcpp/source/)

ivw_group("Header Files" ${HEADER_FILES})
source_group("CMake Files" FILES ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt)
source_group("" FILES ${DOC_FILES})

#add_library(moduleA INTERFACE) # 'moduleA' is an INTERFACE pseudo target
add_library(utf INTERFACE) # 'moduleA' is an INTERFACE pseudo target

# From here, the target 'moduleA' can be customised
target_include_directories(utf INTERFACE ${IVW_EXTENSIONS_DIR}/utf/utfcpp/source/)
target_sources(utf INTERFACE ${HEADER_FILES} ${DOC_FILES})
#target_include_directories(moduleA ...) # Transitively forwarded
#install(TARGETS moduleA ...)

#  HACK: have the files showing in the IDE, under the name 'utfcpp'
add_custom_target(utfcpp SOURCES ${HEADER_FILES} ${DOC_FILES})

# Creates VS folder structure
ivw_folder(utfcpp ext)
