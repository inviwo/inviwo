set(HEADER_FILES 
    include/half/half.hpp
)
ivw_group("Header Files" ${HEADER_FILES})
source_group("CMake Files" FILES ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt)

add_library(half INTERFACE)
add_library(inviwo::half ALIAS half)
target_include_directories(half INTERFACE include)

#  HACK: have the files showing in the IDE, under the name 'half-dummy'
add_custom_target(half_dummy SOURCES ${HEADER_FILES})
ivw_folder(half_dummy ext)
