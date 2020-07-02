# Dockerfile

You'll find in that folder the dockerfile to create a docker image to build inviwo & a second docker image to run inviwo without the build related depedencies (FYI: more aggressive trimming down is needed).

The main goal was to avoid using the X server depedencies to help streamline the devops scaling, hence why the: IVW_TINY_GLFW_APPLICATION & IVW_INTEGRATION_TESTS are turned off. The Inviwo app & inviwo_qt_minimum are building fine. Only thing left to fix is the opengl initialization not working when running Inviwo -w workspace.inv inside the docker entry script. It could be that we are using the wrong nvidia base image, their documentation is on the dry side, or maybe something on the inviwo's side, we would appreciate a fresh set of eyes on that issue.


## Base image creation

Tthe "imagia/nvidia-opengl:1.0-glvnd-1.3-runtime-ubuntu20.04 AS builder" Docker image which isn't public, use the following Dockerfile to create it:

```
FROM ubuntu:20.04
RUN apt update && \
    apt-get install -y --no-install-recommends \
        libglvnd0=1.3.1-1 \
        libgl1 \
        libglx0 \
        libegl1 \
        libgles2 && \
    rm -rf /var/lib/apt/lists/*
COPY 10_nvidia.json /usr/share/glvnd/egl_vendor.d/10_nvidia.json
```

And related file:
```
10_nvidia.json:
{
    "file_format_version" : "1.0.0",
    "ICD" : {
        "library_path" : "libEGL_nvidia.so.0"
    }
}
```
