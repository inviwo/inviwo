# Inviwo hdf Module
ivw_module(HDF5)

set(HEADER_FILES
    include/modules/hdf5/datastructures/hdf5handle.h
    include/modules/hdf5/datastructures/hdf5metadata.h
    include/modules/hdf5/datastructures/hdf5path.h
    include/modules/hdf5/hdf5exception.h
    include/modules/hdf5/hdf5module.h
    include/modules/hdf5/hdf5moduledefine.h
    include/modules/hdf5/hdf5types.h
    include/modules/hdf5/hdf5utils.h
    include/modules/hdf5/ports/hdf5port.h
    include/modules/hdf5/processors/hdf5pathselection.h
    include/modules/hdf5/processors/hdf5source.h
    include/modules/hdf5/processors/hdf5volumesource.h
)
ivw_group("Header Files" ${HEADER_FILES})

set(SOURCE_FILES
    src/datastructures/hdf5handle.cpp
    src/datastructures/hdf5metadata.cpp
    src/datastructures/hdf5path.cpp
    src/hdf5exception.cpp
    src/hdf5module.cpp
    src/hdf5types.cpp
    src/hdf5utils.cpp
    src/processors/hdf5pathselection.cpp
    src/processors/hdf5source.cpp
    src/processors/hdf5volumesource.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES})

option (IVW_USE_EXTERNAL_HDF5 "Link with external HDF5 library instead of building it." OFF)
if (NOT IVW_USE_EXTERNAL_HDF5)
    # HDF5 Components
    set(HDF5_EXTERNALLY_CONFIGURED  1)
    set(HDF5_EXPORTED_TARGETS       "inviwo-hdf5-targets")
    set(HDF5_EXTERNAL_LIB_PREFIX    "ivw-")
    
    set(HDF5_INSTALL_NO_DEVELOPMENT ON)
    if(WIN32)
        set(HDF5_INSTALL_BIN_DIR bin)
        set(HDF5_INSTALL_LIB_DIR lib)
    elseif(APPLE)
        set(HDF5_INSTALL_BIN_DIR bin)
        set(HDF5_INSTALL_LIB_DIR Inviwo.app/Contents/MacOS)
    else()
        set(HDF5_INSTALL_BIN_DIR bin)
        set(HDF5_INSTALL_LIB_DIR lib)
    endif()
    
    find_package(ZLIB REQUIRED)
    set(ZLIB_SHARED_LIBRARY ${ZLIB_LIBRARIES}) # our zlib config is missing this one"
    set(HDF5_ENABLE_Z_LIB_SUPPORT ON)
   
    set(HDF5_BUILD_EXAMPLES OFF)
    set(HDF5_BUILD_TOOLS OFF)
    set(BUILD_TESTING OFF)
    set(HDF5_BUILD_CPP_LIB ON)
    set(HDF5_BUILD_HL_LIB ON)

    if(BUILD_SHARED_LIBS)
        set(BUILD_STATIC_LIBS OFF)
    endif()

    add_subdirectory(ext/hdf5)

    ivw_get_targets_in_dir_recursive(hdf5Targets ext/hdf5)
    ivw_remove_from_list(hdf5Targets hdf5Targets mirror_server mirror_server_stop)
    ivw_join(";" ", " hdf5list ${hdf5Targets})
    message(STATUS "HDF5 Targets: ${hdf5list}")

    set_target_properties(${hdf5Targets} PROPERTIES FOLDER ext/hdf5)
    ivw_suppress_compiler_warnings(${hdf5Targets})

    target_link_libraries(inviwo-module-hdf5 PUBLIC ${hdf5Targets})

    ivw_register_license_file(NAME hdf5 VERSION 1.14.3 MODULE HDF5
        URL http://www.hdfgroup.org/HDF5/
        TYPE "BSD-style"
        FILES ${CMAKE_CURRENT_SOURCE_DIR}/ext/hdf5/COPYING
    )
    ivw_default_install_targets(${h5-targets})

else()
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
endif()
