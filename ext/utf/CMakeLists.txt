# Following
# http://stackoverflow.com/a/29214327/3214598

add_subdirectory(utfcpp)

set(HEADER_FILES 
	${CMAKE_CURRENT_SOURCE_DIR}/utfcpp/source/utf8.h
    ${CMAKE_CURRENT_SOURCE_DIR}/utfcpp/source/utf8/checked.h
    ${CMAKE_CURRENT_SOURCE_DIR}/utfcpp/source/utf8/core.h
    ${CMAKE_CURRENT_SOURCE_DIR}/utfcpp/source/utf8/cpp11.h
    ${CMAKE_CURRENT_SOURCE_DIR}/utfcpp/source/utf8/cpp17.h
    ${CMAKE_CURRENT_SOURCE_DIR}/utfcpp/source/utf8/unchecked.h
)
set(DOC_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/LICENSE 
	${CMAKE_CURRENT_SOURCE_DIR}/README.md
)

ivw_group("Header Files" ${HEADER_FILES})
source_group("CMake Files" FILES ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt)
source_group("" FILES ${DOC_FILES})

#  HACK: have the files showing in the IDE, under the name 'utfcpp_dummy'
add_custom_target(utfcpp_dummy SOURCES ${HEADER_FILES} ${DOC_FILES})
ivw_folder(utfcpp_dummy ext)

ivw_default_install_targets(utf8cpp)
ivw_register_package(utf8cpp utf8cpp)
