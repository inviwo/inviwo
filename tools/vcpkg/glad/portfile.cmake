vcpkg_check_linkage(ONLY_STATIC_LIBRARY)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO Dav1dde/glad
    REF de6c39e3040c987323b8ed078c36442f4fb681b3
    SHA512 a24523186d59de5c0895791c639c62573eaacf1d3843d3bf81eba848b4a33a9a8d17f9b6f791202dac77692bf147e25b3650989731d5ddb7a22e7d023b66885e
    HEAD_REF master
    PATCHES encoding.patch
)

vcpkg_find_acquire_program(PYTHON3)

file(COPY
    ${CURRENT_INSTALLED_DIR}/include/KHR/khrplatform.h
    ${CURRENT_INSTALLED_DIR}/share/egl-registry/egl.xml
    ${CURRENT_INSTALLED_DIR}/share/opengl-registry/gl.xml
    ${CURRENT_INSTALLED_DIR}/share/opengl-registry/glx.xml
    ${CURRENT_INSTALLED_DIR}/share/opengl-registry/wgl.xml
    DESTINATION ${SOURCE_PATH}/glad/files
)

set(profile "core")
if("compatibility" IN_LIST FEATURES)
    set(profile "compatibility")
endif()

set(generator "c")
if(VCPKG_BUILD_TYPE STREQUAL "debug")
    set(generator "c-debug")
endif()

set(loader "--no-loader")
if("loader" IN_LIST FEATURES)
    set(loader "")
endif()

file(REMOVE_RECURSE ${SOURCE_PATH}/src ${SOURCE_PATH}/include)
foreach(spec IN ITEMS "gl" "wgl" "glx" "egl")
    if(NOT spec IN_LIST FEATURES)
        continue()
    endif()
    vcpkg_execute_required_process(
        COMMAND ${PYTHON3} -m glad
            --profile=${profile}
            --out-path=${SOURCE_PATH}
            --generator=${generator}
            --spec=${spec}
            ${loader}
            --reproducible
        WORKING_DIRECTORY ${SOURCE_PATH}
        LOGNAME generate-${TARGET_TRIPLET}-${VCPKG_BUILD_TYPE}
    )
endforeach()

file(COPY ${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt DESTINATION ${SOURCE_PATH})

vcpkg_configure_cmake(
    SOURCE_PATH ${SOURCE_PATH}
    PREFER_NINJA
    OPTIONS
        -DGLAD_INSTALL=ON
    OPTIONS_RELEASE
    OPTIONS_DEBUG
)

vcpkg_install_cmake()
vcpkg_copy_pdbs()
vcpkg_fixup_cmake_targets(CONFIG_PATH lib/cmake/glad)

file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/include ${CURRENT_PACKAGES_DIR}/include/KHR)
file(INSTALL ${SOURCE_PATH}/LICENSE DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT} RENAME copyright)
