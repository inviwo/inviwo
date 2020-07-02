FROM imagia/nvidia-opengl:1.0-glvnd-1.3-runtime-ubuntu20.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive
ENV GCC_MAX_CONCURRENT_PROCESS=8
ENV IVW_BUILD_DIR=/usr/local/inviwo
ENV IVW_INSTALL_DIR=/inviwo
ENV IVW_SOURCE_DIR=/usr/src/inviwo

RUN mkdir -p ${IVW_SOURCE_DIR} ${IVW_INSTALL_DIR} ${IVW_BUILD_DIR}

RUN apt-get update && \
    apt-get install --no-install-recommends -y \
        build-essential \
        cmake \
        freeglut3-dev \
        gcc-8 g++-8 \
        libboost-all-dev libfreetype6 libfreetype6-dev libqt5opengl5-dev libqt5svg5-dev \
        ocl-icd-opencl-dev \
        python3 python3-distutils python3-numpy \
        qt5-default qttools5-dev && \
    rm -rf /var/lib/apt/lists/*

COPY . ${IVW_SOURCE_DIR}

WORKDIR ${IVW_BUILD_DIR}
RUN cmake -DCMAKE_INSTALL_PREFIX:PATH=${IVW_INSTALL_DIR} ${IVW_SOURCE_DIR}
RUN cmake --build ${IVW_BUILD_DIR} --target install -- -j ${GCC_MAX_CONCURRENT_PROCESS} 

FROM imagia/nvidia-opengl:1.0-glvnd-1.3-runtime-ubuntu20.04

ENV DEBIAN_FRONTEND=noninteractive
ENV IVW_BUILD_DIR=/usr/local/inviwo
ENV IVW_GROUP=inviwo
ENV IVW_INSTALL_DIR=/inviwo
ENV IVW_SOURCE_DIR=/usr/src/inviwo
ENV IVW_USER=inviwo

RUN apt-get update && \
    apt-get install --no-install-recommends -y \
        build-essential \
        cmake \
        freeglut3-dev \
        gcc-8 g++-8 \
        libboost-all-dev libfreetype6 libfreetype6-dev libqt5opengl5-dev libqt5svg5-dev \
        ocl-icd-opencl-dev \
        python3 python3-distutils python3-numpy \
        qt5-default qttools5-dev && \
    rm -rf /var/lib/apt/lists/*

COPY --from=builder ${IVW_SOURCE_DIR} ${IVW_SOURCE_DIR}
COPY --from=builder ${IVW_BUILD_DIR} ${IVW_BUILD_DIR}
COPY --from=builder ${IVW_INSTALL_DIR} ${IVW_INSTALL_DIR}

WORKDIR ${IVW_INSTALL_DIR}

