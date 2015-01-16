
SOURCES += \
    $${IVW_MODULE_PATH}/opencl/inviwoopencl.cpp 

HEADERS += \
    $${IVW_MODULE_PATH}/opencl/inviwoopencl.h \
	$${IVW_MODULE_PATH}/opencl/ext/cl/cl.hpp 


win32 {
    isEmpty(NVIDIA_GPU_COMPUTING_SDK) {
        warning("OpenCL module: NVIDIA_GPU_COMPUTING_SDK not set, set environment variable to OpenCL path. Example NVIDIA_GPU_COMPUTING_SDK="C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v5.0"")
    }
    
    win32-msvc {
        LIBS += -L$${NVIDIA_GPU_COMPUTING_SDK}/lib/Win32
        LIBS += -l$${NVIDIA_GPU_COMPUTING_SDK}/lib/Win32/OpenCL
    }
    
    win64-msvc {
        LIBS += -L$${NVIDIA_GPU_COMPUTING_SDK}/lib/x64
        LIBS += -l$${NVIDIA_GPU_COMPUTING_SDK}/lib/x64/OpenCL
    }
}

unix : !macx {
    #QMAKE_LFLAGS += -lOpenCL
    LIBS += -lOpenCL
}

macx {
    LIBS += -framework OpenCL
}
