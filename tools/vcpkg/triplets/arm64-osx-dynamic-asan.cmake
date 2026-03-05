# Custom vcpkg triplet for arm64 macOS (dynamic linking) with AddressSanitizer and
# UndefinedBehaviorSanitizer instrumentation.
#
# Use this triplet to build all vcpkg dependencies with the same sanitizer flags as
# the Inviwo codebase. This is the recommended best practice for comprehensive
# sanitizer coverage: without it, only Inviwo's own code is instrumented, which can
# produce false negatives or spurious errors at library boundaries.
#
# To activate this triplet, pass the following flags to cmake:
#   -DVCPKG_OVERLAY_TRIPLETS=<inviwo_src>/tools/vcpkg/triplets
#   -DVCPKG_TARGET_TRIPLET=arm64-osx-dynamic-asan
#
# Note: Building dependencies with sanitizers creates binaries that are incompatible
# with the regular binary cache. vcpkg will rebuild all dependencies from source.

set(VCPKG_TARGET_ARCHITECTURE arm64)
set(VCPKG_CMAKE_SYSTEM_NAME Darwin)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE dynamic)
set(VCPKG_OSX_ARCHITECTURES arm64)

set(VCPKG_CXX_FLAGS "-fsanitize=address,undefined -fno-omit-frame-pointer")
set(VCPKG_C_FLAGS   "-fsanitize=address,undefined -fno-omit-frame-pointer")
set(VCPKG_LINKER_FLAGS "-fsanitize=address,undefined")
