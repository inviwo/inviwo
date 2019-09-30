#--------------------------------------------------------------------
# Create python module
set(HEADER_FILES
    include/ivwdataframe/ivwdataframe.h
    include/ivwdataframe/pydataframe.h
)
ivw_group("Header Files" BASE ${CMAKE_CURRENT_SOURCE_DIR}/include/ivwdataframe ${HEADER_FILES})

set(SOURCE_FILES
    src/ivwdataframe.cpp
    src/pydataframe.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

ivw_add_py_wrapper(ivwdataframe ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(ivwdataframe PUBLIC inviwo::module::dataframe inviwo::module::python3)
target_include_directories(ivwdataframe PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

ivw_default_install_comp_targets(python ivwdataframe)
