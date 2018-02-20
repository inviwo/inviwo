# Building Inviwo from Source                         {#building}

[TOC]

## Requirements 

### Compilers

The folowing compilers are tested and supported

| Operating System | Compiler                | Minimum version | Recommended Version  |
|:-----------------|:------------------------|:----------------|:---------------------|
| Windows          | Microsoft Visual Studio | 2015            | [Visual Studio 2017] |
| Unix             | GCC                     | ????            | ?????                |
| Unix             | CLang                   | ????            | ?????                |
| Mac              | XCode/CLang             | ????            | ?????                |


### Third party software 

The following software/libraries are needed to get the source and build Inviwo. 

| What           | Required Version | Recommended Version |
|:---------------|:-----------------|:--------------------|
| [CMake]        | 3.2.0            | 3.9.1               |
| [Qt]           | 5.3              | 5.9.1               |
| Any git client |                  | [git]               |
| ^              | ^                | [SourceTree]        |
| ^              | ^                | [TortoiseGit]       |

The following applications are not required, but will enable additional features. 

| What      | Required Version |                                                              |
|:----------|:-----------------|:-------------------------------------------------------------|
| [Python]  | 3.X              | To enable embedded python scripting within Inviwo            |
| OpenCL    | ????             | Needed to use and develop features requiring OpenCL          |
| [Doxygen] | ????             | Used to generate API documentation and in App processor HELP |


## Getting the source  

### Using git

Use git to clone the source code from [github]
`git clone http://www.github.com/inviwo/inviwo.git`

Since we using the submodule feature of git for some of our external dependencies you need to make 
sure that submodules also has been cloned. Some applications, like [sourcetree], does this 
automatically. To clone/update all submodules run: 
`git submodule update --init`

### Downloading zip

Unfortunately, GitHub does not include submodules when downloading source code from their webpage. 
Because of that, the only way of getting the source code at the moment is using git. 

[Python]:  https://www.python.org/downloads/ "Python"
[Doxygen]: http://www.stack.nl/~dimitri/doxygen/download.html#srcbin "Doxygen"
[git]:      https://git-scm.com/downloads   "Git"
[github]:   http://www.github.com/inviwo/inviwo/    "GitHub"
[SourceTree]:   https://www.sourcetreeapp.com/    "SourceTree"
[CMake]:            https://cmake.org/   "CMake"
[Visual Studio 2017]:    https://www.visualstudio.com/downloads/ "Visual Studio 2017"
[Qt]:               http://download.qt.io/archive/qt/ "Qt"
[TortoiseGit]:      https://code.google.com/p/tortoisegit/ "TortoiseGit"


## Configure CMake
Start the CMake-gui app. Set ``Where is the source code`` to the folder where the Inviwo source code 
is. Set ``Where to build the binaries`` to a folder somewhere outside of the Inviwo source and 
press ``Configure``. You will be asked which generator you want to use. Select the compiler you are 
using.

__Note (windows):__ On windows this has to be  ``Visual Studio 15 2015 Win64`` or ``Visual Studio 15 2017 Win64``
depending on installed version of visual studio. It is important you select the Win64 version since 
inviwo does not support 32-bit builts. 
 
__Note (Unix):__ The rest of this guide will decribe actions as if you selected ``Unix makefiles`` 
and going to buld using a terminal window, but it should be possible to set up for you editor of choise. 
 
If no errors occurred, press ``Generate`` to generate the makefiles/solution.  

## Build and run the application
### Windows
While still in the CMake-gui application, when the ``Generate``-action has completed press  
``Open Project`` to open the Visual Studio solution. Before building, you might want to change from
Debug ot either RelWithDebInfo or Release to speed up the execution of the Inviwo application. 
The build the solution press F7 (if you have the default keyboard shortcuts) and then start inviwo 
by pressing F5 (or ctrl+F5 to not attached the bugger)

### Unix
If you selected ``Unix Makefiles`` as generator in the CMake-gui, open a terminal al change the 
directoy to the path you specified as ``Where to build the binaries`` in previous step. Start building
by typing ```make -jN``` where N is the number of threads to use, set that to the number of cores on
your machine or 1 less than the number of cores. 
 
When building is completed, type `./bin/inviwo` in the console to start Inviwo

### Mac OSX
TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO


## Common error and problems 

#### CMake gives error/warning when configuring
If you get the following message from CMake when configuring the Inviwo build 
```
CMake Warning at CMakeLists.txt:90 (find_package):
  By not providing "FindPythonLibsNew.cmake" in CMAKE_MODULE_PATH this
  project has asked CMake to find a package configuration file provided by
  "PythonLibsNew", but CMake did not find one.
```
This message indicates that the git submodule has not been initlized. Open a terminal a change direcyoty 
to the root if the inviwo source code and type: 
```
git submodule update --init --recursive 
``` 


