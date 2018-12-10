if(IVW_UNITTESTS OR IVW_INTEGRATION_TESTS)
    # Prevent overriding the parent project's compiler/linker
    # settings on Windows
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    option(BUILD_GMOCK "Builds the googlemock subproject" OFF)
    add_subdirectory(gtest)

    ivw_folder(gtest ext)
    ivw_folder(gtest_main ext)

    if(BUILD_SHARED_LIBS)
        target_compile_definitions(gtest PRIVATE -DGTEST_CREATE_SHARED_LIBRARY=1)
        target_compile_definitions(gtest INTERFACE -DGTEST_LINKED_AS_SHARED_LIBRARY=1)
        #target_compile_definitions(gtest_main PRIVATE -DGTEST_LINKED_AS_SHARED_LIBRARY=0)
    endif()
endif()