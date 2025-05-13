if(NOT DEFINED VCPKG_TARGET_TRIPLET)
    if(CMAKE_HOST_APPLE)
        # cmake flags for CMAKE_SYSTEM_PROCESSOR etc, are usually not
        # around before the first call to project, which is to late.
        execute_process(
            COMMAND uname -m
            OUTPUT_VARIABLE HOST_ARCH
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        if(HOST_ARCH STREQUAL "arm64")
            if (BUILD_SHARED_LIBS)
                set(VCPKG_TARGET_TRIPLET arm64-osx-dynamic)
            else()
                set(VCPKG_TARGET_TRIPLET arm64-osx)
            endif()
        else()
            if (BUILD_SHARED_LIBS)
                set(VCPKG_TARGET_TRIPLET x64-osx-dynamic)
            else()
                set(VCPKG_TARGET_TRIPLET x64-osx)
            endif()
        endif()
    elseif(CMAKE_HOST_LINUX)
        if (BUILD_SHARED_LIBS)
            set(VCPKG_TARGET_TRIPLET x64-linux-dynamic)
        else()
            set(VCPKG_TARGET_TRIPLET x64-linux)
        endif()
    elseif(CMAKE_HOST_WIN32)
        if (BUILD_SHARED_LIBS)
            set(VCPKG_TARGET_TRIPLET x64-windows)
        elseif(IVW_CFG_MSVC_FORCE_SHARED_CRT)
            set(VCPKG_TARGET_TRIPLET x64-windows-static-md)
        else()
            set(VCPKG_TARGET_TRIPLET x64-windows-static)
        endif()
    endif()
endif()
if(NOT DEFINED VCPKG_HOST_TRIPLET)
    set(VCPKG_HOST_TRIPLET ${VCPKG_TARGET_TRIPLET})
endif()

message(STATUS "BUILD_SHARED_LIBS: ${BUILD_SHARED_LIBS}")
message(STATUS "VCPKG_TARGET_TRIPLET: ${VCPKG_TARGET_TRIPLET}")
message(STATUS "VCPKG_HOST_TRIPLET: ${VCPKG_HOST_TRIPLET}")

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/../../vcpkg")
    find_package(Git QUIET)
    if(GIT_FOUND)
        execute_process(COMMAND "${GIT_EXECUTABLE}" rev-parse HEAD
            WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/../../vcpkg"
            RESULT_VARIABLE result
            OUTPUT_VARIABLE vcpkg_current_sha
            ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        file(READ "${CMAKE_CURRENT_LIST_DIR}/../vcpkg.json" vcpkg_json)
        string(JSON vcpkg_configuration GET "${vcpkg_json}" "vcpkg-configuration")
        string(JSON default_registry GET "${vcpkg_configuration}" "default-registry")
        string(JSON baseline GET "${default_registry}" "baseline")

        message(STATUS "inviwo baseline: ${baseline}, current vcpkg ${vcpkg_current_sha}")
        if(NOT baseline STREQUAL vcpkg_current_sha)
            execute_process(
                COMMAND "${GIT_EXECUTABLE}" merge-base --is-ancestor ${vcpkg_current_sha} ${baseline}
                WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/../../vcpkg"
                RESULT_VARIABLE IS_ANCESTOR_RESULT
            )
            if(IS_ANCESTOR_RESULT EQUAL 0)
                message(FATAL_ERROR "Your vcpkg repo is outdated.
You need to pull the VCPKG repo to commit: ${baseline}
git pull
git reset --hard ${baseline}
./bootstrap-vcpkg.[bat|sh]
")

            endif()

            execute_process(
                COMMAND "${GIT_EXECUTABLE}" merge-base --is-ancestor ${baseline} ${vcpkg_current_sha}
                WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/../../vcpkg"
                RESULT_VARIABLE IS_ANCESTOR_RESULT
            )
            if(IS_ANCESTOR_RESULT EQUAL 0)
                message(WARNING "Your vcpkg repo is newer than the inviwo baseline.
This can result in cache misses if your vcpkg tool has a different version.
To ensure consistant versions run:
git pull
git reset --hard ${baseline}
./bootstrap-vcpkg.[bat|sh]
")
            else()
                message(WARNING "Vcpkg baseline missmatch")
            endif()
        endif()
    endif()
endif()
