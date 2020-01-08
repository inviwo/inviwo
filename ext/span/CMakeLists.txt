cmake_minimum_required(VERSION 3.8)
project(span LANGUAGES CXX)

add_library(span INTERFACE)
add_library(inviwo::span ALIAS span)
target_include_directories(span INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/span/include)
target_compile_definitions(span INTERFACE TCB_SPAN_NAMESPACE_NAME=inviwo::util )
target_compile_features(span INTERFACE cxx_std_17)

#  HACK: have the files showing in the IDE
add_custom_target(span_dummy SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/span/include/tcb/span.hpp)
# Creates VS folder structure
ivw_folder(span_dummy ext)