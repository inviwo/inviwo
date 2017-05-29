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
set(HDF5_EXTERNALLY_CONFIGURED  1)
set(HDF5_EXPORTED_TARGETS       "inviwo-hdf5-targets")
set(HDF5_EXTERNAL_LIB_PREFIX    "ivw-")

set(HDF5_INSTALL_NO_DEVELOPMENT ON)
if(WIN32)
    set(HDF5_INSTALL_BIN_DIR bin)
    set(HDF5_INSTALL_LIB_DIR lib)
else(APPLE)
    set(HDF5_INSTALL_BIN_DIR bin)
    set(HDF5_INSTALL_LIB_DIR Inviwo.app/Contents/MacOS)
else()
    set(HDF5_INSTALL_BIN_DIR bin)
    set(HDF5_INSTALL_LIB_DIR lib)
endif()

set(HDF5_ENABLE_Z_LIB_SUPPORT   ON)
find_package(ZLIB)
set(H5_ZLIB_HEADER "zlib.h")
set(ZLIB_INCLUDE_DIRS "${ZLIB_INCLUDE_DIR}")
if(${BUILD_SHARED_LIBS})
    set(ZLIB_SHARED_LIBRARY "${ZLIB_LIBRARIES}")
else()
    set(ZLIB_STATIC_LIBRARY "${ZLIB_LIBRARIES}")
endif()

set(HDF5_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(HDF5_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)

add_subdirectory(ext/hdf5)

set(h5-static hdf5-static hdf5_hl-static hdf5_cpp-static hdf5_hl_cpp-static)
set(h5-cmake H5detect H5make_libsettings)

set(h5-targets ${h5-static} ${h5-cmake})
if(BUILD_SHARED_LIBS)
    set(h5-shared hdf5-shared hdf5_hl-shared hdf5_cpp-shared hdf5_hl_cpp-shared)
    set(h5-targets ${h5-targets} ${h5-shared})
endif()

set_target_properties(${h5-targets} PROPERTIES FOLDER ext/hdf5)
ivw_suppress_compiler_warnings(${h5-targets})

# Configure some specific warnings.
if(WIN32 AND MSVC)
    foreach(target ${h5-targets})
        target_compile_options(${target} PRIVATE "/w24005")
    endforeach()
    set_property(TARGET ${h5-static} APPEND_STRING PROPERTY STATIC_LIBRARY_FLAGS " /IGNORE:4221")
endif()

if(${BUILD_SHARED_LIBS})
    target_link_libraries(inviwo-module-hdf5 ${h5-shared})
else()
    target_link_libraries(inviwo-module-hdf5 ${h5-static})
endif()

# not sure why this dir is nto picked up automatically
# Needed to pick up some generated headers.
target_include_directories(inviwo-module-hdf5 PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/ext/hdf5)