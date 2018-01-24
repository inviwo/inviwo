    project(BaseBenchmarks)
    #--------------------------------------------------------------------
    # Add source files
    set(SOURCE_FILES 
        ${CMAKE_CURRENT_SOURCE_DIR}/benchmain.cpp 
    )
    ivw_group("Source Files" ${SOURCE_FILES})

    set(target "base-benchmark")
    #--------------------------------------------------------------------
    # Create application
    add_executable(${target} MACOSX_BUNDLE WIN32 ${SOURCE_FILES})
    target_link_libraries(${target} PUBLIC benchmark)
    target_link_libraries(${target} PUBLIC inviwo::module::base)
    set_target_properties(${target} PROPERTIES FOLDER benchmarks)

    #--------------------------------------------------------------------
    # Define defintions and properties
    ivw_define_standard_definitions(${target} ${target})
    ivw_define_standard_properties(${target})
