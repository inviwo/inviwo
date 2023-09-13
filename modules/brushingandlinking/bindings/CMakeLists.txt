# Create python module
if(NOT IVW_USE_PYTHON)
    message(FATAL_ERROR "Python support is not enabled for Inviwo (see IVW_USE_PYTHON)")
endif()

find_package(pybind11 CONFIG REQUIRED)

set(HEADER_FILES
)
ivw_group("Header Files" BASE ${CMAKE_CURRENT_SOURCE_DIR}/include/modules/brushingandlinking/pythonbindings ${HEADER_FILES})

set(SOURCE_FILES
    src/brushingandlinkingbindings.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

ivw_add_py_wrapper(ivwbnl ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(ivwbnl PUBLIC inviwo::module::brushingandlinking inviwo::module::python3)
target_include_directories(ivwbnl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

ivw_default_install_targets(ivwbnl)
