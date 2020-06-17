# Following
# http://stackoverflow.com/a/29214327/3214598

add_subdirectory(utfcpp)

set(HEADER_FILES 
	${IVW_EXTENSIONS_DIR}/utf/utfcpp/source/utf8.h
    ${IVW_EXTENSIONS_DIR}/utf/utfcpp/source/utf8/checked.h
    ${IVW_EXTENSIONS_DIR}/utf/utfcpp/source/utf8/core.h
    ${IVW_EXTENSIONS_DIR}/utf/utfcpp/source/utf8/unchecked.h
)
set(DOC_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/LICENSE 
	${CMAKE_CURRENT_SOURCE_DIR}/README.md
)

ivw_group("Header Files" ${HEADER_FILES})
source_group("CMake Files" FILES ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt)
source_group("" FILES ${DOC_FILES})

#  HACK: have the files showing in the IDE, under the name 'utfcpp'
add_custom_target(utfcpp SOURCES ${HEADER_FILES} ${DOC_FILES})
set_target_properties(utfcpp PROPERTIES VERSION 3.1.0 SOVERSION 3.1)

# Creates VS folder structure
ivw_folder(utfcpp ext)

#ivw_default_install_targets(utf8cpp)
ivw_register_package(utfcpp utf8cpp)