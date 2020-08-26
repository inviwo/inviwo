cmake_minimum_required(VERSION 3.11..3.18)

project(tracy)

set(imgui_hdr 
    imgui/imconfig.h
    imgui/imgui.h
    imgui/imgui_internal.h
    imgui/imstb_rectpack.h
    imgui/imstb_textedit.h
    imgui/imstb_truetype.h
)
set(imgui_src
    imgui/imgui.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_widgets.cpp
)

set(gl3w_hdr 
    profiler/libs/gl3w/GL/gl3w.h
    profiler/libs/gl3w/GL/glcorearb.h
)
set(gl3w_src
    profiler/libs/gl3w/GL/gl3w.c
)

set(nfd_hdr 
    nfd/common.h
    nfd/nfd.h
    nfd/nfd_common.h
)
set(nfd_src
    nfd/nfd_common.c
    nfd/nfd_win.cpp
)

set(zstd_hdr 
    zstd/bitstream.h
    zstd/compiler.h
    zstd/cpu.h
    zstd/debug.h
    zstd/error_private.h
    zstd/fse.h
    zstd/hist.h
    zstd/huf.h
    zstd/mem.h
    zstd/pool.h
    zstd/threading.h
    zstd/xxhash.h
    zstd/zstd.h
    zstd/zstd_compress_internal.h
    zstd/zstd_compress_literals.h
    zstd/zstd_compress_sequences.h
    zstd/zstd_compress_superblock.h
    zstd/zstd_cwksp.h
    zstd/zstd_ddict.h
    zstd/zstd_decompress_block.h
    zstd/zstd_decompress_internal.h
    zstd/zstd_double_fast.h
    zstd/zstd_errors.h
    zstd/zstd_fast.h
    zstd/zstd_internal.h
    zstd/zstd_lazy.h
    zstd/zstd_ldm.h
    zstd/zstd_opt.h
    zstd/zstdmt_compress.h
)
set(zstd_src
    zstd/debug.c
    zstd/entropy_common.c
    zstd/error_private.c
    zstd/fse_compress.c
    zstd/fse_decompress.c
    zstd/hist.c
    zstd/huf_compress.c
    zstd/huf_decompress.c
    zstd/pool.c
    zstd/threading.c
    zstd/xxhash.c
    zstd/zstd_common.c
    zstd/zstd_compress.c
    zstd/zstd_compress_literals.c
    zstd/zstd_compress_sequences.c
    zstd/zstd_compress_superblock.c
    zstd/zstd_ddict.c
    zstd/zstd_decompress.c
    zstd/zstd_decompress_block.c
    zstd/zstd_double_fast.c
    zstd/zstd_fast.c
    zstd/zstd_lazy.c
    zstd/zstd_ldm.c
    zstd/zstd_opt.c
    zstd/zstdmt_compress.c
)

set(common_hdr 
    common/tracy_lz4.hpp
    common/tracy_lz4hc.hpp
    common/TracyAlign.hpp
    common/TracyForceInline.hpp
    common/TracyMutex.hpp
    common/TracyProtocol.hpp
    common/TracyQueue.hpp
    common/TracySocket.hpp
    common/TracySystem.hpp
)
set(common_src
    common/tracy_lz4.cpp
    common/tracy_lz4hc.cpp
    common/TracySocket.cpp
    common/TracySystem.cpp
)

set(server_hdr
    server/TracyCharUtil.hpp
    server/TracyEvent.hpp
    server/TracyFileRead.hpp
    server/TracyFileWrite.hpp
    server/TracyMemory.hpp
    server/TracyMmap.hpp
    server/TracyPopcnt.hpp
    server/TracyPrint.hpp
    server/TracySlab.hpp
    server/TracyStackFrames.hpp
    server/TracyTaskDispatch.hpp
    server/TracyTextureCompression.hpp
    server/TracyThreadCompress.hpp
    server/TracyVector.hpp
    server/TracyWorker.hpp
)
set(server_src
    server/TracyMemory.cpp
    server/TracyMmap.cpp
    server/TracyPrint.cpp
    server/TracyStackFrames.cpp
    server/TracyTaskDispatch.cpp
    server/TracyTextureCompression.cpp
    server/TracyThreadCompress.cpp
    server/TracyWorker.cpp
)

set(server_extra_hdr 
    server/IconsFontAwesome5.h
    server/tracy_pdqsort.h
    server/tracy_robin_hood.h
    server/tracy_xxh3.h
    server/TracyBadVersion.hpp
    server/TracyBuzzAnim.hpp
    server/TracyColor.hpp
    server/TracyDecayValue.hpp
    server/TracyFileHeader.hpp
    server/TracyFilesystem.hpp
    server/TracyImGui.hpp
    server/TracyMicroArchitecture.hpp
    server/TracyMouse.hpp
    server/TracyShortPtr.hpp
    server/TracySort.hpp
    server/TracySourceView.hpp
    server/TracyStorage.hpp
    server/TracyStringDiscovery.hpp
    server/TracyTexture.hpp
    server/TracyUserData.hpp
    server/TracyVarArray.hpp
    server/TracyVersion.hpp
    server/TracyView.hpp
    server/TracyViewData.hpp
    server/TracyYield.hpp
)
set(server_extra_src
    server/TracyBadVersion.cpp
    server/TracyColor.cpp
    server/TracyFilesystem.cpp
    server/TracyMicroArchitecture.cpp
    server/TracyMouse.cpp
    server/TracySourceView.cpp
    server/TracyStorage.cpp
    server/TracyTexture.cpp
    server/TracyUserData.cpp
    server/TracyView.cpp
)

set(profiler_hdr 
    profiler/src/Arimo.hpp
    profiler/src/Cousine.hpp
    profiler/src/FontAwesomeSolid.hpp
    profiler/src/icon.hpp
    profiler/src/imgui_freetype.h
    profiler/src/imgui_impl_glfw.h
    profiler/src/imgui_impl_opengl3.h
    profiler/src/HttpRequest.hpp
    profiler/src/NativeWindow.hpp
    profiler/src/ResolvService.hpp
    profiler/src/stb_image.h
)
set(profiler_src
    profiler/src/imgui_freetype.cpp
    profiler/src/imgui_impl_glfw.cpp
    profiler/src/imgui_impl_opengl3.cpp
    profiler/src/main.cpp
    profiler/src/HttpRequest.cpp
    profiler/src/NativeWindow.cpp
    profiler/src/ResolvService.cpp
    profiler/src/winmain.cpp
    profiler/src/winmainArchDiscovery.cpp
)

add_library(tracy_opts INTERFACE)
add_library(tracy::opts ALIAS tracy_opts)
target_compile_features(tracy_opts INTERFACE cxx_std_17)
target_compile_definitions(tracy_opts INTERFACE 
    INTERFACE 
        _CRT_SECURE_NO_DEPRECATE
        _CRT_NONSTDC_NO_DEPRECATE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        _USE_MATH_DEFINES
        _MBCS
)
target_compile_options(tracy_opts INTERFACE /permissive- /arch:AVX2 /fp:fast /Gm- /W3 /MP /sdl /Oi)
target_link_options(tracy_opts INTERFACE /OPT:ICF /OPT:REF)

find_package(glfw3 CONFIG REQUIRED)
find_package(Freetype CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

add_library(tracy_common OBJECT ${common_src} ${common_hdr})
add_library(tracy::common ALIAS tracy_common)
target_link_libraries(tracy_common PUBLIC tracy::opts ws2_32)

add_library(tracy_gl3w OBJECT ${gl3w_src} ${gl3w_hdr})
add_library(tracy::gl3w ALIAS tracy_gl3w)
target_include_directories(tracy_gl3w PUBLIC profiler/libs/gl3w ${OPENGL_INCLUDE_DIR})
target_link_libraries(tracy_gl3w PUBLIC tracy::opts ${OPENGL_LIBRARIES})
target_compile_definitions(tracy_gl3w INTERFACE IMGUI_IMPL_OPENGL_LOADER_GL3W)

add_library(tracy_imgui OBJECT ${imgui_src} ${imgui_hdr})
add_library(tracy::imgui ALIAS tracy_imgui)
target_include_directories(tracy_imgui PUBLIC imgui)
target_link_libraries(tracy_imgui PUBLIC tracy::opts tracy::gl3w)

add_library(tracy_nfd OBJECT ${nfd_src} ${nfd_hdr})
add_library(tracy::nfd ALIAS tracy_nfd)
target_link_libraries(tracy_nfd PUBLIC tracy::opts)

add_library(tracy_zstd OBJECT ${zstd_src} ${zstd_hdr})
add_library(tracy::zstd ALIAS tracy_zstd)
target_link_libraries(tracy_zstd PUBLIC tracy::opts)

add_library(tracy_capstone INTERFACE)
add_library(tracy::capstone ALIAS tracy_capstone)
find_library(CAPSTONE_LIBRARY NAMES capstone_dll capstone)
find_path(CAPSTONE_INCLUDE_DIR NAMES capstone/capstone.h)
target_link_libraries(tracy_capstone INTERFACE ${CAPSTONE_LIBRARY})
target_include_directories(tracy_capstone INTERFACE ${CAPSTONE_INCLUDE_DIR})

add_library(tracy_server OBJECT ${server_hdr} ${server_src})
add_library(tracy::server ALIAS tracy_server)
target_link_libraries(tracy_server PUBLIC tracy::opts tracy::capstone)
target_compile_definitions(tracy_server PUBLIC TRACY_NO_STATISTICS)

if(NOT DEFINED TRACY_SKIP_TOOLS)
    add_executable(TracyProfiler WIN32 
        ${server_hdr} ${server_src}
        ${server_extra_hdr} ${server_extra_src}
        ${profiler_hdr} ${profiler_src}
        profiler/build/win32/DebugVis.natvis
        profiler/build/win32/Tracy.rc
    )
    target_link_libraries(TracyProfiler
        PRIVATE
            tracy::opts
            tracy::common
            tracy::imgui
            tracy::gl3w 
            tracy::nfd
            tracy::zstd
            tracy::capstone
            glfw
            freetype
    )
    
    add_executable(update update/src/update.cpp getopt/getopt.h getopt/getopt.c)
    target_link_libraries(update PRIVATE tracy::opts tracy::common tracy::zstd tracy::server)
    
    add_executable(capture capture/src/capture.cpp getopt/getopt.h getopt/getopt.c)
    target_link_libraries(capture PRIVATE tracy::opts tracy::common tracy::zstd tracy::server)
    
    add_executable(import-chrome import-chrome/src/import-chrome.cpp import-chrome/src/json.hpp)
    target_link_libraries(import-chrome PRIVATE tracy::opts tracy::common tracy::zstd tracy::server)

    install(TARGETS TracyProfiler update capture import-chrome RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

add_library(tracy TracyClient.cpp)
target_compile_definitions(tracy PRIVATE TRACY_EXPORTS TRACY_ENABLE)
target_include_directories(tracy PUBLIC $<INSTALL_INTERFACE:include>)
set_target_properties(tracy PROPERTIES POSITION_INDEPENDENT_CODE ON)

install(TARGETS tracy
    EXPORT unofficial-tracy-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(
    EXPORT unofficial-tracy-targets
    FILE unofficial-tracy-config.cmake
    NAMESPACE unofficial::tracy::
    DESTINATION share/unofficial-tracy
)

if(NOT DEFINED TRACY_SKIP_INSTALL_HEADERS)
    install(
        DIRECTORY 
            ${CMAKE_CURRENT_SOURCE_DIR}/common
            ${CMAKE_CURRENT_SOURCE_DIR}/client
        DESTINATION include/tracy
        FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h"
    )
    install(
        FILES  
            Tracy.hpp
            TracyC.h
            TracyD3D12.hpp
            TracyLua.hpp
            TracyOpenCL.hpp
            TracyOpenGL.hpp
            TracyVulkan.hpp
        DESTINATION include/tracy
    )
endif()
