# How to build Inviwo

## Windows

### Dependencies
You will need at least
- [CMake](https://cmake.org/download/) >= 3.12.0\
    Also add the cmake binary to your PATH.

- [Qt binaries](https://qt.io/download-open-source/) >= 5.12\
    Make sure you get the build for the 64 bit version for you Visual Studio version. Also add the Qt binary directory (something like `Qt/5.9.1/msvc2017_64/bin`) to your PATH.

### Building
1. `git clone --recurse-submodules https://github.com/inviwo/inviwo`\
The `--recurse-submodules` is necessary to pull dependencies.

2. Open CMake (we recommend using the GUI here), enter the source path and the preferred build directory (outside the inviwo directory!) and hit configure. You can then select the desired Inviwo modules (`IVW_MODULE_*`) and configure again. When selecting the compiler, make sure to select the correct Visual Studio version that you use on 64-bit. 32-bit is not supported.
3. (Optional) To add external Inviwo modules, add those in `IVW_EXTERNAL_MODULES` in the format of\
`C:/Inviwo/otherrepo/modules;C:/mysite/myrepo/mymodules;`\
 Use front slashes and no space between modules. Configure again. External modules are developed in the [inviwo modules repository](https://github.com/inviwo/modules).
4. Hit Generate and open the project in your IDE.

### Common Errors
#### Everything compiles but at runtime you get "failed to load QT symbols dll load errors"
Make sure that the same Qt version used for building is found when running the application. A common source of this error is that Anaconda is installed, which includes another Qt version and has added itself to the PATH environment variable. Make sure that the Qt version used for building is **before** the Anaconda path in the PATH. We have observed a similar problem with certain LaTeX distributions, so if the issue remains, try to move the LaTeX entry in your PATH behind your Qt version as well.

#### Everything compiles but at runtime you get "failed to load python.dll"
Add the path to the Python bin folder to your PATH environment variable.
You can find the path to the Python binary in Visual Studio by right clicking on the inviwo-module-python3 project and go to "Properties->Linker->Input->Additional dependencies".

#### Everything compiles but at runtime you get runtime error / Unhandled Exception in pybind11/embed.h
This may happen when the `PYTHONHOME` variable is not set or is incorrect. Check your system settings to see if it is correctly pointing to your python installation found by CMake. If you do not have the `PYTHONHOME` variable you should set it. It should point to the root folder of your python installation, e.g `C:/python37 or C:\Program Files (x86)\Microsoft Visual Studio\Shared\Anaconda3_64` (if you installed Anaconda with Visual Studio). To know which python installation inviwo uses you can check the output from the configuration pass in CMake, in the very beginning of the log it prints which python interpreter it found and will use.

## Linux

### Dependencies
You will need at least
- [CMake](https://cmake.org/download/) >= 3.12.0\
    **Ubuntu**: Make sure to add the [Kitware APT Repository](https://apt.kitware.com/) when you want to install cmake via `apt-get`, since the official Ubuntu repo offers an outdated CMake version.\
    **Other distors**: You can try your package manager, just make sure you get version 3.12.0 or higher from it.
- [Qt binaries](https://qt.io/download-open-source/) >= 5.12\
    Make sure you get the build for the 64 bit version of gcc or clang. Make sure to add the Qt folder to the `CMAKE_PREFIX_PATH` environment variable.\
    **Example**: `export CMAKE_PREFIX_PATH=/home/user/Qt/5.13.0/gcc_x64/`\
    **Note**: We highly recommend installing Qt with the official Qt installer instead of your package manager for Inviwo. While you can certainly get the versions from package managers to work, we experienced issues in the past with missing components and compiler incompatibilities.

For **Ubuntu** you can use the following commands:
```
wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | sudo apt-key add -

sudo apt-add-repository 'deb https://apt.kitware.com/ubuntu/ bionic main'

sudo apt-get update

sudo apt-get install build-essential cmake cmake-qt-gui git freeglut3-dev xorg-dev
```
The first two commands add the Kitware APT Repo and the appropriate signing key, the third and fourth update your package manager and download the dependencies.

### Building
1. `git clone --recurse-submodules https://github.com/inviwo/inviwo`\
The `--recurse-submodules` is necessary to pull dependencies.
2. Open CMake (we recommend using the GUI here), enter the source path and the preferred build directory (outside the inviwo directory!) and hit configure. You can then select the desired Inviwo modules (`IVW_MODULE_*`) and configure again.\
If CMake cannot find Qt, make sure you adjust your `CMAKE_PREFIX_PATH` as described above.
3. (Optional) To add external Inviwo modules, add those in `IVW_EXTERNAL_MODULES` in the format of\
`C:/Inviwo/otherrepo/modules;C:/mysite/myrepo/mymodules;`\
 Use front slashes and no space between modules. Configure again.
 External modules are developed in the [inviwo modules repository](https://github.com/inviwo/modules).
4. Hit Generate and open the project in your IDE.


## Mac
TODO: do

## Build Options
TODO: do



## Recommended Visual studio setup (Optional)

* Keep computer responsive during compiling.

Make sure that multiplying the two following options does not exceed the number of cores your computer has. Recommended on an 8-core machine: 1) 2-4,  2) 4.

**1)** Visual Studio: Tools->Options->Projects and Solutions->Build and Run->maximum number of parallel project builds

**2)** CMake: Set IVW_MULTIPROCESSOR_COUNT
