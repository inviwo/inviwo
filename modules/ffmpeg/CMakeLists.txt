ivw_module(FFmpeg)

set(HEADER_FILES
    include/inviwo/ffmpeg/ffmpeganimationrecorder.h
    include/inviwo/ffmpeg/ffmpegmodule.h
    include/inviwo/ffmpeg/ffmpegmoduledefine.h
    include/inviwo/ffmpeg/outputstream.h
    include/inviwo/ffmpeg/processors/movieexport.h
    include/inviwo/ffmpeg/recorder.h
    include/inviwo/ffmpeg/util.h
    include/inviwo/ffmpeg/wrap/codec.h
    include/inviwo/ffmpeg/wrap/codecid.h
    include/inviwo/ffmpeg/wrap/format.h
    include/inviwo/ffmpeg/wrap/frame.h
    include/inviwo/ffmpeg/wrap/outputformat.h
    include/inviwo/ffmpeg/wrap/packet.h
    include/inviwo/ffmpeg/wrap/swscale.h
)
ivw_group("Header Files" ${HEADER_FILES})

set(SOURCE_FILES
    src/ffmpeganimationrecorder.cpp
    src/ffmpegmodule.cpp
    src/outputstream.cpp
    src/processors/movieexport.cpp
    src/recorder.cpp
    src/util.cpp
    src/wrap/codec.cpp
    src/wrap/codecid.cpp
    src/wrap/format.cpp
    src/wrap/frame.cpp
    src/wrap/outputformat.cpp
    src/wrap/packet.cpp
    src/wrap/swscale.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

set(TEST_FILES
    tests/unittests/ffmpeg-unittest-main.cpp
)
ivw_add_unittest(${TEST_FILES})

ivw_create_module(${SOURCE_FILES} ${HEADER_FILES})

find_package(FFMPEG REQUIRED)
ivw_vcpkg_install(ffmpeg MODULE FFmpeg)
target_include_directories(inviwo-module-ffmpeg 
    PUBLIC 
        $<BUILD_INTERFACE:${FFMPEG_INCLUDE_DIRS}>
        $<BUILD_INTERFACE:${SWSCALE_INCLUDE_DIRS}>
)
target_link_directories(inviwo-module-ffmpeg 
    PUBLIC 
        $<BUILD_INTERFACE:${FFMPEG_LIBRARY_DIRS}>
        $<BUILD_INTERFACE:${SWSCALE_LIBRARY_DIRS}>
)
target_link_libraries(inviwo-module-ffmpeg 
    PUBLIC 
        ${FFMPEG_LIBRARIES}
        ${SWSCALE_LIBRARIES}
)

target_compile_definitions(inviwo-module-ffmpeg PUBLIC __STDC_CONSTANT_MACROS)
