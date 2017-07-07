#--------------------------------------------------------------------
# Inviwo Python3 Module
ivw_module(Python3)


ivw_include_directories(${PYTHON_INCLUDE_DIR})
ivw_include_directories(${CMAKE_SOURCE_DIR}/ext/pybind11/include)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/numpymeshcreatetest.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/numpymandelbrot.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/numpyvolume.h
    ${CMAKE_CURRENT_SOURCE_DIR}/pybindutils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/pythonexecutionoutputobservable.h
    ${CMAKE_CURRENT_SOURCE_DIR}/pythoninterpreter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/pythonlogger.h
    ${CMAKE_CURRENT_SOURCE_DIR}/pythonscript.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/numpymeshcreatetest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/numpymandelbrot.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/numpyvolume.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pythonexecutionoutputobservable.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pythoninterpreter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pybindutils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pythonlogger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pythonscript.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})


#--------------------------------------------------------------------
# Add script files
set(SCRIPT_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/documentgenerator.py
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/outputredirector.py
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/ivw/__init__.py
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/ivw/animation.py
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/ivw/camera.py
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/ivw/regression.py
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/ivw/utils.py

    ${CMAKE_CURRENT_SOURCE_DIR}/data/scripts/mandelbrot.py
    ${CMAKE_CURRENT_SOURCE_DIR}/data/scripts/numpymeshcreatetest.py
    ${CMAKE_CURRENT_SOURCE_DIR}/data/scripts/numpyvolumeprocessor.py
)
ivw_group("Script Files" ${SCRIPT_FILES})


#--------------------------------------------------------------------
# Add Unittests
set(TEST_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unittests/python3-unittest-main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unittests/scripts-test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unittests/scripts/grabreturnvalue.py
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unittests/scripts/passvalues.py
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unittests/scripts/simple_buffer_test.py
)
ivw_add_unittest(${TEST_FILES})

set(INTERFACE_HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/inviwopy.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/pyglmtypes.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/pyvolume.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/pybuffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/pymesh.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/pyimage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/pynetwork.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/pyprocessors.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/pyproperties.h
)
#ivw_group("Header Files" ${INTERFACE_HEADER_FILES})

set(INTERFACE_SRC_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/inviwopy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/pyglmtypes.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/pyimage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/pyvolume.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/pybuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/pymesh.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/pynetwork.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/pyprocessors.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/pyproperties.cpp
)
#ivw_group("Source Files" ${INTERFACE_SRC_FILES})



add_dependency_libs_to_module(pybind11::module)
#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SCRIPT_FILES})
target_link_libraries(${${mod}_target} $<BUILD_INTERFACE:${PYTHON_LIBRARIES}>)

#target_link_libraries(${${mod}_target} pybind11::module)


ivw_add_py_wrapper(inviwopy ${INTERFACE_SRC_FILES} ${INTERFACE_HEADER_FILES})

if(IVW_UNITTESTS_RUN_ON_BUILD)
    add_dependencies(inviwo-unittests-python3 inviwopy)
endif()

#--------------------------------------------------------------------
# Add Python to package
if(IVW_PACKAGE_PROJECT AND BUILD_SHARED_LIBS)
    get_filename_component(python_lib_name "${PYTHON_LIB}" NAME_WE)
    get_filename_component(python_lib_dir "${PYTHON_LIB}" DIRECTORY)
    get_filename_component(python_dir "${python_lib_dir}" DIRECTORY)
    if(WIN32)
        find_file(PYTHON_DLL NAMES ${python_lib_name}.dll PATHS ${python_dir})
        install(FILES ${PYTHON_DLL} DESTINATION bin COMPONENT modules)
    endif()
endif()

# Add scripts directory to pack
ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/scripts)
