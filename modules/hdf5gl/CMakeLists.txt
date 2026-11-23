ivw_module(HDF5GL)

set(HEADER_FILES
    include/inviwo/hdf5gl/hdf5glmodule.h
    include/inviwo/hdf5gl/hdf5glmoduledefine.h
    include/inviwo/hdf5gl/layerhdftoglconverter.h
    include/inviwo/hdf5gl/volumehdftoglconverter.h
)
ivw_group("Header Files" ${HEADER_FILES})

set(SOURCE_FILES
    src/hdf5glmodule.cpp
    src/layerhdftoglconverter.cpp
    src/volumehdftoglconverter.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

set(TEST_FILES
    tests/unittests/hdf5gl-unittest-main.cpp
)
ivw_add_unittest(${TEST_FILES})

ivw_create_module(${SOURCE_FILES} ${HEADER_FILES})
