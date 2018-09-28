#--------------------------------------------------------------------
# Inviwo Python3 Module
ivw_module(Python3)

#Check if numpy exist 
execute_process(COMMAND "${PYTHON_EXECUTABLE}" "-c"
                "try: import numpy; print(numpy.__version__);\nexcept: print('failed')\n"
                RESULT_VARIABLE NUMPY_STATUS
                OUTPUT_VARIABLE NUMPY_OUTPUT_VERSION
                ERROR_VARIABLE NUMPY_ERROR
                OUTPUT_STRIP_TRAILING_WHITESPACE
               )

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/numpymandelbrot.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/numpymeshcreatetest.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/numpyvolume.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/pythonscriptprocessor.h
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
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/numpymandelbrot.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/numpymeshcreatetest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/numpyvolume.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/pythonscriptprocessor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pybindutils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pythonexecutionoutputobservable.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pythoninterpreter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pythonlogger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pythonscript.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

#--------------------------------------------------------------------
# Add script files
set(SCRIPT_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/documentgenerator.py
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/ivw/__init__.py
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/ivw/animation.py
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/ivw/camera.py
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/ivw/regression.py
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/ivw/utils.py

    ${CMAKE_CURRENT_SOURCE_DIR}/data/scripts/mandelbrot.py
    ${CMAKE_CURRENT_SOURCE_DIR}/data/scripts/numpymeshcreatetest.py
    ${CMAKE_CURRENT_SOURCE_DIR}/data/scripts/numpyvolumeprocessor.py
    ${CMAKE_CURRENT_SOURCE_DIR}/data/scripts/scriptprocessorexample.py
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
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unittests/scripts/glm.py
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unittests/scripts/option_property.py
)
if(NOT NUMPY_OUTPUT_VERSION MATCHES "failed")
    set(TEST_FILES
        ${TEST_FILES}
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/unittests/numpy-test.cpp
    )
endif()
ivw_add_unittest(${TEST_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SCRIPT_FILES})
target_link_libraries(inviwo-module-python3 PUBLIC pybind11::module $<BUILD_INTERFACE:${PYTHON_LIBRARIES}>)
target_include_directories(inviwo-module-python3 PUBLIC
    ${PYTHON_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/ext/pybind11/include
)

#--------------------------------------------------------------------
# Create python module
set(INTERFACE_HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/inviwopy.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/pybuffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/pydataformat.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/pydatamapper.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/pyglmtypes.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/pyimage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/pyinviwoapplication.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/pyinviwomodule.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/pymesh.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/pynetwork.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/pypickingmapper.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/pyport.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/pyprocessors.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/pyproperties.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/pypropertyowner.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/pytfprimitiveset.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/pyvolume.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/vectoridentifierwrapper.h
)
set(INTERFACE_SRC_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/inviwopy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/pybuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/pydataformat.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/pydatamapper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/pyglmtypes.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/pyimage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/pyinviwoapplication.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/pyinviwomodule.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/pymesh.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/pynetwork.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/pypickingmapper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/pyport.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/pyprocessors.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/pyproperties.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/pypropertyowner.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/pytfprimitiveset.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/pyvolume.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/vectoridentifierwrapper.cpp
)
ivw_add_py_wrapper(inviwopy ${INTERFACE_SRC_FILES} ${INTERFACE_HEADER_FILES})

if(IVW_UNITTESTS_RUN_ON_BUILD)
    add_dependencies(inviwo-unittests-python3 inviwopy)
endif()

#--------------------------------------------------------------------
# Add Python to package
if(IVW_PACKAGE_PROJECT AND BUILD_SHARED_LIBS)
    if(WIN32)
        set(py_version "${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}.${PYTHON_VERSION_PATCH}")
        set(py_url "https://www.python.org/ftp/python/${py_version}/python-${py_version}-embed-amd64.zip")
        set(py_dir "${CMAKE_BINARY_DIR}/py_embedd/")
        set(py_file "${py_dir}/py_embedd.${py_version}.zip")
        if(NOT EXISTS "${py_file}")
            file(DOWNLOAD ${py_url} ${py_file} STATUS py_status SHOW_PROGRESS)
            list(GET py_status 0 py_success)
            if(NOT py_success EQUAL 0)
                message(FATAL_ERROR "Could not download python embedd: ${py_url}")
            endif()
        endif()
       
        file(MAKE_DIRECTORY "${py_dir}/python")
        execute_process(COMMAND ${CMAKE_COMMAND} -E tar "xf" ${py_file}
                        WORKING_DIRECTORY "${py_dir}/python")
        install(DIRECTORY "${py_dir}/python/" DESTINATION bin COMPONENT python)
    endif()
endif()

ivw_default_install_comp_targets(python inviwopy)
# Add scripts directory to pack
ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/scripts)
ivw_make_package(InviwoPython3Module inviwo-module-python3)


#--------------------------------------------------------------------
# register license files
# Cannot get pybind11::module version property since it is an INTERFACE target
ivw_register_license_file(NAME "PyBind11" VERSION 2.2.2 MODULE Python3
    URL "https://github.com/pybind/pybind11"
    TYPE "BSD-style"
    FILES ${IVW_EXTENSIONS_DIR}/pybind11/LICENSE
)
