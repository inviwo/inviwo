# Create python module
find_package(pybind11 CONFIG REQUIRED)

set(HEADER_FILES
)
ivw_group("Header Files" BASE ${CMAKE_CURRENT_SOURCE_DIR}/include/modules/brushingandlinking/pythonbindings ${HEADER_FILES})

set(SOURCE_FILES
    src/brushingandlinkingbindings.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

ivw_add_py_wrapper(ivwbnl)
target_sources(ivwbnl
    PUBLIC
    FILE_SET HEADERS
    TYPE HEADERS
    BASE_DIRS include
    FILES
        ${HEADER_FILES}
    PRIVATE
        ${SOURCE_FILES}
)
target_link_libraries(ivwbnl PUBLIC inviwo::module::brushingandlinking inviwo::module::python3)

ivw_install_helper(TARGET ivwbnl
    NAMESPACE inviwo
    DESTINATION inviwo
)
