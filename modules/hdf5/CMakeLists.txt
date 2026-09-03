# Inviwo hdf Module
ivw_module(HDF5)

set(HEADER_FILES
    include/modules/hdf5/datastructures/hdf5handle.h
    include/modules/hdf5/datastructures/hdf5path.h
    include/modules/hdf5/datastructures/hdf5selection.h
    include/modules/hdf5/hdf5exception.h
    include/modules/hdf5/hdf5module.h
    include/modules/hdf5/hdf5moduledefine.h
    include/modules/hdf5/hdf5read.h
    include/modules/hdf5/hdf5types.h
    include/modules/hdf5/hdf5utils.h
    include/modules/hdf5/ports/hdf5port.h
    include/modules/hdf5/properties/dimselectionproperty.h
    include/modules/hdf5/properties/dimselectionsproperty.h
    include/modules/hdf5/processors/hdf5pathselection.h
    include/modules/hdf5/processors/hdf5source.h
    include/modules/hdf5/processors/hdf5volumesource.h
    include/modules/hdf5/processors/hdf5layersource.h
    include/modules/hdf5/processors/hdf5buffersource.h
    include/modules/hdf5/processors/hdf5dataframesource.h
)
ivw_group("Header Files" ${HEADER_FILES})

set(SOURCE_FILES
    src/datastructures/hdf5handle.cpp
    src/datastructures/hdf5path.cpp
    src/hdf5exception.cpp
    src/hdf5module.cpp
    src/hdf5read.cpp
    src/hdf5types.cpp
    src/hdf5utils.cpp
    src/properties/dimselectionproperty.cpp
    src/properties/dimselectionsproperty.cpp
    src/processors/hdf5pathselection.cpp
    src/processors/hdf5source.cpp
    src/processors/hdf5volumesource.cpp
    src/processors/hdf5layersource.cpp
    src/processors/hdf5buffersource.cpp
    src/processors/hdf5dataframesource.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

# Add Unittests
set(TEST_FILES
    tests/unittests/hdf5-unittest-main.cpp
    tests/unittests/dimselection-test.cpp
    tests/unittests/path-test.cpp
)
ivw_add_unittest(${TEST_FILES})

# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES})

find_package(hdf5 QUIET CONFIG COMPONENTS C CXX)
if(hdf5_FOUND)
    target_link_libraries(inviwo-module-hdf5
        PUBLIC
            hdf5::hdf5$<IF:$<BOOL:${BUILD_SHARED_LIBS}>,-shared,-static>
            hdf5::hdf5_cpp$<IF:$<BOOL:${BUILD_SHARED_LIBS}>,-shared,-static>
    )
    ivw_vcpkg_install(hdf5 MODULE HDF5)
else()
    if(NOT BUILD_SHARED_LIBS)
        set(HDF5_USE_STATIC_LIBRARIES ON)
    endif()
    set(HDF5_FIND_DEBUG ON)
    find_package(HDF5 REQUIRED COMPONENTS C CXX)
    target_link_libraries(inviwo-module-hdf5
        PUBLIC
            hdf5::hdf5$<IF:$<BOOL:${BUILD_SHARED_LIBS}>,,-static>
            hdf5::hdf5_cpp$<IF:$<BOOL:${BUILD_SHARED_LIBS}>,,-static>
    )
endif()

