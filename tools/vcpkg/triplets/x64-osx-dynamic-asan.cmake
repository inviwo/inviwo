# Custom vcpkg triplet for x64 macOS (dynamic linking) with AddressSanitizer and
# UndefinedBehaviorSanitizer instrumentation.
#
# Use this triplet to build all vcpkg dependencies with the same sanitizer flags as
# the Inviwo codebase. This is the recommended best practice for comprehensive
# sanitizer coverage: without it, only Inviwo's own code is instrumented, which can
# produce false negatives or spurious errors at library boundaries.
#
# To activate this triplet, pass the following flags to cmake:
#   -DVCPKG_OVERLAY_TRIPLETS=<inviwo_src>/tools/vcpkg/triplets
#   -DVCPKG_TARGET_TRIPLET=x64-osx-dynamic-asan
#   -DVCPKG_HOST_TRIPLET=x64-osx-dynamic
#
# IMPORTANT: Always set VCPKG_HOST_TRIPLET to a non-ASAN triplet (e.g.
# x64-osx-dynamic).  Host tools such as pkgconf are executed during the build
# process by cmake's execute_process().  When compiled with sanitizer
# instrumentation those tools fail to start on macOS with the ASAN runtime dylib
# not available in that context.  Keeping the host triplet free of sanitizer flags
# ensures the build tools work correctly while target libraries are still fully
# instrumented.
#
# Note: Building dependencies with sanitizers creates binaries that are incompatible
# with the regular binary cache. vcpkg will rebuild all dependencies from source.

set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CMAKE_SYSTEM_NAME Darwin)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE dynamic)
set(VCPKG_OSX_ARCHITECTURES x86_64)

set(VCPKG_CXX_FLAGS "-fsanitize=address,undefined -fno-omit-frame-pointer")
set(VCPKG_C_FLAGS   "-fsanitize=address,undefined -fno-omit-frame-pointer")
set(VCPKG_LINKER_FLAGS "-fsanitize=address,undefined")

# Qt builds some translation units with -fno-rtti.  The UBSan vptr checker
# emits references to RTTI typeinfo symbols that are absent when RTTI is
# disabled, causing "Undefined symbols for architecture x86_64: typeinfo
# for ..." linker errors in libQt6Gui and other Qt libraries.  Disable the
# vptr sub-check only for Qt ports; all other UBSan diagnostics are kept.
if(PORT MATCHES "qt*")
    string(APPEND VCPKG_CXX_FLAGS " -fno-sanitize=vptr")
    string(APPEND VCPKG_C_FLAGS " -fno-sanitize=vptr")
    string(APPEND VCPKG_LINKER_FLAGS " -fno-sanitize=vptr")
endif()

# The python3 port installs an executable (tools/python3/python3) that is
# invoked at build time and at runtime.  When the binary is built with ASAN
# it gets an rpath entry for libclang_rt.asan_osx_dynamic.dylib which is not
# present in the vcpkg installed tree, causing a dyld load failure.  Skip
# sanitizer instrumentation for python3 to keep the interpreter functional.
if(PORT STREQUAL "python3")
    set(VCPKG_CXX_FLAGS "-fno-omit-frame-pointer")
    set(VCPKG_C_FLAGS   "-fno-omit-frame-pointer")
    set(VCPKG_LINKER_FLAGS "")
endif()
