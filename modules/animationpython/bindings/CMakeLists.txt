# Create python module
set(HEADER_FILES
    include/ivwanimation/ivwanimation.h
    include/ivwanimation/pyanimation.h
)
ivw_group("Header Files" BASE ${CMAKE_CURRENT_SOURCE_DIR}/include/ivwanimation ${HEADER_FILES})

set(SOURCE_FILES
    src/ivwanimation.cpp
    src/pyanimation.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

ivw_add_py_wrapper(ivwanimation)
target_sources(ivwanimation
    PUBLIC
    FILE_SET HEADERS
    TYPE HEADERS
    BASE_DIRS include
    FILES
        ${HEADER_FILES}
    PRIVATE
        ${SOURCE_FILES}
)
target_link_libraries(ivwanimation PUBLIC inviwo::module::animation inviwo::module::python3)

ivw_install_helper(TARGET ivwanimation
    NAMESPACE inviwo
    DESTINATION inviwo
)
