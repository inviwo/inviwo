project(inviwo-python-helper)

add_library(inviwo-python-helper)
add_library(inviwo::python-helper ALIAS inviwo-python-helper)

get_property(pymodules GLOBAL PROPERTY IVW_PYMODULE_LIST)

set(FUNCTIONDECLS "")
set(FUNCTIONCALLS "")

# ensure inviwopy is first
# In the future we mihgt need to apply more fine grained sorting here if there are more dependencies
# For now this suffices.
if("inviwopy" IN_LIST pymodules)
    list(REMOVE_ITEM pymodules "inviwopy")
    list(PREPEND pymodules "inviwopy")
endif()

# enusre there are no dupliated.
list(REMOVE_DUPLICATES pymodules)

foreach(item IN LISTS pymodules)
    string(APPEND FUNCTIONDECLS "void inviwo_static_pybind_init_${item}();\n")
    string(APPEND FUNCTIONCALLS "    inviwo_static_pybind_init_${item}();\n")
endforeach()

configure_file(
    pythonhelper.cpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/python-helper.cpp
    @ONLY
)

target_sources(inviwo-python-helper
    PUBLIC
    FILE_SET HEADERS
    TYPE HEADERS
    BASE_DIRS ${IVW_INCLUDE_DIR}
    FILES
        ${IVW_INCLUDE_DIR}/inviwo/py/pythonhelper.h
    PRIVATE
        pythonhelper.cpp.in
        ${CMAKE_CURRENT_BINARY_DIR}/python-helper.cpp
)

ivw_define_standard_definitions(IVW_PYTHONHELPER inviwo-python-helper)
ivw_define_standard_properties(inviwo-python-helper)

ivw_install_helper(TARGET inviwo-python-helper
    NAMESPACE inviwo
    DESTINATION inviwo
    ALIAS python-helper
)

# Add build dependency towards any pybind11 targets and inviwo::python-helper,
# if python is not enabled, this pymodules list will be empty

message(STATUS "Modules: ${pymodules}")
if(pymodules)
    if(BUILD_SHARED_LIBS)
        add_dependencies(inviwo-python-helper ${pymodules})
    else()
        target_link_libraries(inviwo-python-helper PRIVATE ${pymodules})
    endif()
endif()
