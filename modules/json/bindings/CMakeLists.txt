# Create python bindings for json module
find_package(pybind11 CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

set(HEADER_FILES
)
ivw_group("Header Files" BASE ${CMAKE_CURRENT_SOURCE_DIR}/include/modules/json/pythonbindings ${HEADER_FILES})

set(SOURCE_FILES
    src/jsonbindings.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

ivw_add_py_wrapper(ivwjson)
target_sources(ivwjson
    PUBLIC
    FILE_SET HEADERS
    TYPE HEADERS
    BASE_DIRS include
    FILES
        ${HEADER_FILES}
    PRIVATE
        ${SOURCE_FILES}
)
target_link_libraries(ivwjson PUBLIC
    inviwo::module::json
    inviwo::module::python3
    nlohmann_json::nlohmann_json
)

ivw_install_helper(TARGET ivwjson
    NAMESPACE inviwo
    DESTINATION inviwo
)
