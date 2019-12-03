if(IVW_UNITTESTS OR IVW_INTEGRATION_TESTS)
    # Prevent overriding the parent project's compiler/linker
    # settings on Windows
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    set(BUILD_GMOCK ON CACHE BOOL "Builds the googlemock subproject" FORCE)

    add_subdirectory(gtest)

    ivw_folder(gtest ext)
    ivw_folder(gtest_main ext)

    ivw_folder(gmock ext)
    ivw_folder(gmock_main ext)

    if(BUILD_SHARED_LIBS)
        target_compile_definitions(gtest PRIVATE -DGTEST_CREATE_SHARED_LIBRARY=1)
        target_compile_definitions(gtest INTERFACE -DGTEST_LINKED_AS_SHARED_LIBRARY=1)
    endif()
endif()