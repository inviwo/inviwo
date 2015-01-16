# Interactive Visualization Workshop (Inviwo)
if(DESIRED_QT_VERSION MATCHES 5)
cmake_minimum_required(VERSION 2.8.11)
cmake_policy(VERSION 2.8.11)
else()
cmake_minimum_required(VERSION 2.8.4)
cmake_policy(VERSION 2.8.4)
endif()
if(POLICY CMP0046)
    cmake_policy(SET CMP0046 OLD) # Missing dependencies policy.
endif()

if(CMAKE_VERSION VERSION_GREATER "3.0.0")
	cmake_policy(SET CMP0042 OLD) # MacOSX rpath policy.
endif()

if (NOT CMAKE_VERSION VERSION_LESS "3.1.0")
	# include TARGET_OBJECTS expressions in a target's SOURCES property
	cmake_policy(SET CMP0051 NEW)
	# only interpret if() arguments as variables or keywords when unquoted
	cmake_policy(SET CMP0054 NEW)
endif()

project(inviwo-projects)

# Build
option(SHARED_LIBS "Build shared libs, else static libs" ON)

include(cmake/globalconfig.cmake)

#--------------------------------------------------------------------
# Default setup of options for the build

option(IVW_SIGAR "Use sigar to detect system capabilities" ON)

# Modules
build_module(Base)
build_module(BaseGL)
build_module(FontRendering)
build_module(FreeImage)
build_module(OpenGL)
build_module(PVM)

# OpenGL Context created with GLFW or Qt within Inviwo by default
if(IVW_USE_GLFW_NOT_OPENGLQT)
build_module(GLFW)
else()
build_module(OpenGLQt)
endif()

# Applications/Qt
option(IVW_QT_APPLICATION "Build Inviwo Qt Application" ON)
option(IVW_QT_EDITOR "Build Inviwo Qt Editor" ON)
option(IVW_QT_WIDGETS "Build Inviwo Qt Widgets" ON)

option(IVW_UNIT_TEST_APPLICATION "Build Inviwo Unit tests Application" OFF)

#--------------------------------------------------------------------
# Add externals
add_subdirectory(ext/ticpp)
if(IVW_SIGAR)
add_subdirectory(ext/sigar)
endif()
if(WIN32)
add_subdirectory(ext/stackwalker)
endif()

#--------------------------------------------------------------------
# Add Core
add_subdirectory(src/core)

#--------------------------------------------------------------------
# Add Qt
add_subdirectory(src/qt)

#--------------------------------------------------------------------
# Add modules
begin_add_modules()
add_internal_modules()
add_external_modules()
end_add_modules()

#--------------------------------------------------------------------
# Add applications
add_subdirectory(apps)

#--------------------------------------------------------------------
# Add external projects
add_external_projects()

#--------------------------------------------------------------------
# Package creation
include(cmake/packaging.cmake)