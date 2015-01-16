IVW_MODULE_CLASSES += OpenCLModule
IVW_MODULE_CLASS_HEADERS += opencl/openclmodule.h
IVW_MODULE_CLASS_SOURCES += opencl/openclmodule.cpp
win32 {
    isEmpty(NVIDIA_GPU_COMPUTING_SDK) {
        error("OpenCL module: NVIDIA_GPU_COMPUTING_SDK not set.")
    }
    INCLUDEPATH += $${NVIDIA_GPU_COMPUTING_SDK}/OpenCL/common/inc
    INCLUDEPATH += $${NVIDIA_GPU_COMPUTING_SDK}/include
}