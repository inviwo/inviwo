#--------------------------------------------------------------------
# Create python module
set(HEADER_FILES
    include/ivwdiscretedata/ivwdiscretedata.h
    include/ivwdiscretedata/pychannels.h
    include/ivwdiscretedata/pyconnectivities.h
    include/ivwdiscretedata/pydataset.h
)
ivw_group("Header Files" BASE ${CMAKE_CURRENT_SOURCE_DIR}/include/ivwdiscretedata ${HEADER_FILES})

set(SOURCE_FILES
    src/ivwdiscretedata.cpp
    src/pychannels.cpp
    src/pyconnectivities.cpp
    src/pydataset.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

ivw_add_py_wrapper(ivwdiscretedata ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(ivwdiscretedata PUBLIC inviwo::module::discretedata inviwo::module::discretedatapython inviwo::module::python3)
target_include_directories(ivwdiscretedata PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

ivw_default_install_comp_targets(python ivwdiscretedata)
