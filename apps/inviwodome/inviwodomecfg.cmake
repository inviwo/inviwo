# set BUILD_SHARED_LIBS OFF
# set VCPKG_TARGET_TRIPLET x64-windows-static
# Set IVW_CFG_INCLUDE to this file
# C:/Users/petst55.AD/Documents/Inviwo/inviwo/apps/inviwodome/inviwodomecfg.cmake

message(STATUS "Loading ${CMAKE_CURRENT_LIST_DIR}/inviwodomecfg.cmake")

set(BUILD_SHARED_LIBS                   OFF)
set(IVW_CFG_MSVC_FORCE_SHARED_CRT       ON)
set(VCPKG_TARGET_TRIPLET x64-windows-static-md CACHE STRING "")
set(CMAKE_TOOLCHAIN_FILE 
	"C:/Users/petst55.AD/Documents/vcpkg/vcpkg/scripts/buildsystems/vcpkg.cmake" 
	CACHE STRING "")
set(IVW_CFG_VCPKG_OVERLAYS "${CMAKE_SOURCE_DIR}/tools/vcpkg")

# Config
set(IVW_CFG_CMAKE_DEBUG              	ON)
set(IVW_CFG_PROFILING                	ON)
set(IVW_CFG_PRECOMPILED_HEADERS      	OFF)
set(IVW_CFG_FORCE_ASSERTIONS         	ON)
set(IVW_CFG_BUILD_CHANGELOG             OFF)
set(IVW_CFG_PROJECT_NAME				inviwodome)

# Testing
set(IVW_TEST_UNIT_TESTS_RUN_ON_BUILD 	OFF)
set(IVW_TEST_UNIT_TESTS                 OFF)
set(IVW_TEST_BENCHMARKS                 OFF)
set(IVW_TEST_INTEGRATION_TESTS          OFF)

# Packaging
set(IVW_PACKAGE_INSTALLER            	ON)

# Enable inviwo dome
set(IVW_PACKAGE_SELECT_APP				inviwodome)
set(IVW_APP_INVIWO_DOME                 ON)

# Apps we don't need
set(IVW_APP_INVIWO                      OFF)
set(IVW_APP_MINIMAL_GLFW             	OFF)
set(IVW_APP_MINIMAL_QT               	OFF)
set(IVW_APP_QTBASE                      OFF)
set(IVW_APP_META_CLI                    OFF)
# Qt modules we don't need
set(IVW_MODULE_QTWIDGETS                OFF)
set(IVW_MODULE_OPENGLQT					OFF)
set(IVW_MODULE_DATAFRAMEQT	     		OFF)
set(IVW_MODULE_ANIMATIONQT              OFF)

set(IVW_MODULE_SGCT                     ON)
set(IVW_MODULE_HDF5                     ON)
set(IVW_MODULE_ASSIMP                   ON)
set(IVW_MODULE_USERINTERFACEGL          ON)
set(IVW_MODULE_PLOTTING                 ON)
set(IVW_MODULE_PLOTTINGGL               ON)
set(IVW_MODULE_FONTRENDERING            ON)

# can't use static python
set(IVW_APP_PYTHON                      OFF)
set(IVW_MODULE_PYTHON3                  OFF)
set(IVW_MODULE_PYTHON3QT                OFF)
set(IVW_MODULE_DATAFRAMEPYTHON          OFF)
set(IVW_MODULE_PYTHONTOOLS              OFF)

set(IVW_MODULE_PVM						OFF)

# Externald deps
set(IVW_USE_TRACY						ON)
set(IVW_USE_OPENEXR                  	ON)
set(IVW_USE_OPENMP                   	OFF)

set(IVW_USE_EXTERNAL_ASSIMP          	ON)
set(IVW_USE_EXTERNAL_BENCHMARK          ON)
set(IVW_USE_EXTERNAL_EIGEN           	ON)
set(IVW_USE_EXTERNAL_FMT             	ON)
set(IVW_USE_EXTERNAL_FREETYPE        	ON)
set(IVW_USE_EXTERNAL_GLFW            	ON)
set(IVW_USE_EXTERNAL_GLM             	ON)
set(IVW_USE_EXTERNAL_GTEST              ON)
set(IVW_USE_EXTERNAL_HDF5            	ON)
set(IVW_USE_EXTERNAL_IMG             	ON)
set(IVW_USE_EXTERNAL_JSON             	ON)
set(IVW_USE_EXTERNAL_CIMG				ON)
set(IVW_USE_EXTERNAL_GLEW				ON)
set(IVW_USE_EXTERNAL_TCLAP				ON)
set(IVW_USE_EXTERNAL_UTFCPP				ON)
set(IVW_USE_EXTERNAL_TINYDIR		    ON)
set(IVW_USE_EXTERNAL_PYBIND11		    ON)
set(IVW_USE_EXTERNAL_OPENEXR         	ON)
set(IVW_USE_EXTERNAL_SGCT            	ON)
set(IVW_USE_EXTERNAL_NIFTI            	ON)