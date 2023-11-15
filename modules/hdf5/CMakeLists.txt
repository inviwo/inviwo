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
    option(HDF5_ENABLE_Z_LIB_SUPPORT "Enable Zlib Filters" ON)
   
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

    if(BUILD_SHARED_LIBS)
        target_link_libraries(inviwo-module-hdf5 PUBLIC ${h5-shared})
    else()
        target_link_libraries(inviwo-module-hdf5 PUBLIC ${h5-static})
    endif()

    # not sure why this dir is nto picked up automatically
    # Needed to pick up some generated headers.
    set(hdf_inc_dirs "")
    foreach(target ${h5-targets})
        get_target_property(inc_dirs ${target} INCLUDE_DIRECTORIES)
        if(inc_dirs)
            list(APPEND hdf_inc_dirs ${inc_dirs})
        endif()
    endforeach()
    remove_duplicates(hdf_unique_inc_dirs ${hdf_inc_dirs})
    target_include_directories(inviwo-module-hdf5 PUBLIC ${hdf_unique_inc_dirs})

    ivw_register_license_file(NAME hdf5 VERSION 1.10.0.1 MODULE HDF5
        URL http://www.hdfgroup.org/HDF5/
        TYPE "BSD-style"
        FILES ${CMAKE_CURRENT_SOURCE_DIR}/ext/hdf5/COPYING
    )
    ivw_default_install_targets(${h5-targets})

else()
    find_package(hdf5 REQUIRED)
    if(BUILD_SHARED_LIBS)
        target_link_libraries(inviwo-module-hdf5 
            PUBLIC 
                hdf5::hdf5-shared 
                hdf5::hdf5_hl-shared 
                hdf5::hdf5_cpp-shared 
                hdf5::hdf5_hl_cpp-shared
        )
    else()
        target_link_libraries(inviwo-module-hdf5 
            PUBLIC 
                hdf5::hdf5-static 
                hdf5::hdf5_hl-static 
                hdf5::hdf5_cpp-static 
                hdf5::hdf5_hl_cpp-static
        )
    endif()
    ivw_vcpkg_install(hdf5 MODULE HDF5)
    set(version ${HDF5_VERSION_STRING})
endif()
