## Inviwo - Interactive Visualization Workshop

Inviwo is a software framework for rapid visualization prototyping.

Package releases and information is available at www.inviwo.org

Below follow some general information about the framework:

 - Freely available under the Simplified BSD License.
 - Cross-platform and runs on Windows, Linux and Mac OS X.
 - Easily extendable through inclusion of external modules and projects.

#### Core
 - The core is written in C/C++ only, with minor dependencies.
 
#### Modules
 - Modern graphics programming through OpenGL (>= 3.2 Core) related modules.
 - Parallel computing on multiple platforms (GPU/CPU) through OpenCL (>= 1.0) related modules.
 - Python (> 3.2) scripting and computation is supported through provided modules.

#### GUI
 - The primary GUI is based on Qt (Supported >= 5.3).
 - A minimal application is available, utilizing GLFW 3 for multiple window and context management.

#### Build system
 - The project and module configuration/generation is performed through CMake (>= 2.8.11).
 - Inviwo has been compiled in Visual Studio (>= 2013), XCode (>= 5), KDevelop (>= 4), Make.
 - C++11 Required