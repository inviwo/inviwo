#--------------------------------------------------------------------
# Inviwo hdf Module
ivw_module(HDF5)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/hdf5handle.h
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/hdf5metadata.h
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/hdf5path.h
    ${CMAKE_CURRENT_SOURCE_DIR}/hdf5exception.h
    ${CMAKE_CURRENT_SOURCE_DIR}/hdf5types.h
    ${CMAKE_CURRENT_SOURCE_DIR}/hdf5utils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ports/hdf5port.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/hdf5pathselection.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/hdf5source.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/hdf5volumesource.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/hdf5handle.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/hdf5metadata.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/hdf5path.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/hdf5exception.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/hdf5types.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/hdf5utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/hdf5pathselection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/hdf5source.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/hdf5volumesource.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})


#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES})

#--------------------------------------------------------------------
# HDF5 Components
set(HDF5_EXTERNALLY_CONFIGURED 1)
set(HDF5_EXPORTED_TARGETS "inviwo-hdf5-targets")
set(HDF5_EXTERNAL_LIB_PREFIX "ivw")
set(HDF5_INSTALL_NO_DEVELOPMENT ON)

set(HDF5_ENABLE_Z_LIB_SUPPORT OFF)
#find_package(ZLIB REQUIRED)
#set(H5_ZLIB_HEADER "zlib.h")
#set(ZLIB_USE_EXTERNAL ON)
#set(ZLIB_INCLUDE_DIRS "${ZLIB_INCLUDE_DIR}")
#set(ZLIB_LIBRARIES "${ZLIB_LIBRARIES}")

# config
set(HDF5_BUILD_EXAMPLES OFF)
set(HDF5_BUILD_TOOLS OFF)
set(BUILD_TESTING OFF)

add_subdirectory(ext/hdf5)

target_link_libraries(inviwo-module-hdf5
    hdf5-shared
    hdf5_hl-shared
    hdf5_cpp-shared
    hdf5_hl_cpp-shared
)
# not sure why this dir is nto picked up automatically
target_include_directories(inviwo-module-hdf5 PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/ext/hdf5)