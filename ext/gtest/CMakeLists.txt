if(IVW_UNITTESTS OR IVW_INTEGRATION_TESTS)
    # Prevent overriding the parent project's compiler/linker
    # settings on Windows
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    # Add googletest directly to our build. This defines
    # the gtest and gtest_main targets.
    add_subdirectory(gtest/googletest)

    ivw_folder(gtest ext)
    ivw_folder(gtest_main ext)

    if(BUILD_SHARED_LIBS)
        target_compile_definitions(gtest PRIVATE -DGTEST_CREATE_SHARED_LIBRARY=1)
        target_compile_definitions(gtest INTERFACE -DGTEST_LINKED_AS_SHARED_LIBRARY=1)
        #target_compile_definitions(gtest_main PRIVATE -DGTEST_LINKED_AS_SHARED_LIBRARY=0)
    endif()

    ivw_register_license_file(NAME "Google C++ Testing Framework" TARGET gtest MODULE Core
        URL "https://github.com/google/googletest"
        TYPE "BSD 3-clause"
        FILES ${IVW_EXTENSIONS_DIR}/gtest/gtest/LICENSE
    )

endif()