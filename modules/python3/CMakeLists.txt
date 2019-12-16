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
    include/modules/python3/processors/numpymandelbrot.h
    include/modules/python3/processors/numpymeshcreatetest.h
    include/modules/python3/processors/numpyvolume.h
    include/modules/python3/processors/pythonscriptprocessor.h
    include/modules/python3/pybindutils.h
    include/modules/python3/pyportutils.h
    include/modules/python3/python3module.h
    include/modules/python3/python3moduledefine.h
    include/modules/python3/pythonexecutionoutputobservable.h
    include/modules/python3/pythoninterpreter.h
    include/modules/python3/pythonlogger.h
    include/modules/python3/pythonprocessorfactoryobject.h
    include/modules/python3/pythonprocessorfolderobserver.h
    include/modules/python3/pythonscript.h
    include/modules/python3/pyutils.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    src/processors/numpymandelbrot.cpp
    src/processors/numpymeshcreatetest.cpp
    src/processors/numpyvolume.cpp
    src/processors/pythonscriptprocessor.cpp
    src/pybindutils.cpp
    src/pyportutils.cpp
    src/python3module.cpp
    src/pythonexecutionoutputobservable.cpp
    src/pythoninterpreter.cpp
    src/pythonlogger.cpp
    src/pythonprocessorfactoryobject.cpp
    src/pythonprocessorfolderobserver.cpp
    src/pythonscript.cpp
    src/pyutils.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

#--------------------------------------------------------------------
# Add script files
set(SCRIPT_FILES
    scripts/documentgenerator.py
    scripts/ivw/__init__.py
    scripts/ivw/animation.py
    scripts/ivw/camera.py
    scripts/ivw/regression.py
    scripts/ivw/utils.py

    data/scripts/mandelbrot.py
    data/scripts/numpymeshcreatetest.py
    data/scripts/numpyvolumeprocessor.py
    data/scripts/scriptprocessorexample.py
)
ivw_group("Script Files" ${SCRIPT_FILES})

#--------------------------------------------------------------------
# Add Unittests
set(TEST_FILES
    tests/unittests/python3-unittest-main.cpp
    tests/unittests/scripts-test.cpp
    tests/unittests/scripts/grabreturnvalue.py
    tests/unittests/scripts/passvalues.py
    tests/unittests/scripts/simple_buffer_test.py
    tests/unittests/scripts/glm.py
    tests/unittests/scripts/option_property.py
)
if(NOT NUMPY_OUTPUT_VERSION MATCHES "failed")
    list(APPEND TEST_FILES tests/unittests/numpy-test.cpp)
endif()
ivw_add_unittest(${TEST_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SCRIPT_FILES})
target_link_libraries(inviwo-module-python3 PUBLIC 
    pybind11::embed 
    $<BUILD_INTERFACE:${PYTHON_LIBRARIES}>
)
target_include_directories(inviwo-module-python3 PUBLIC
    ${PYTHON_INCLUDE_DIR}
    ${IVW_EXTENSIONS_DIR}/pybind11/include
)

add_subdirectory(bindings)

if(IVW_UNITTESTS)
    add_dependencies(inviwo-unittests-python3 inviwopy)
endif()

#--------------------------------------------------------------------
# Add Python to package
option(IVW_PACKAGE_EMBED_PYTHON "Embed a python dist in the installer.\ 
 Otherwise a existing python installation is needed to run inviwo" ON)
if(BUILD_SHARED_LIBS AND IVW_PACKAGE_EMBED_PYTHON)
    if(WIN32)
        if(IVW_CMAKE_DEBUG)
            set(show SHOW_PROGRESS)
        else()
            set(show "")
        endif()

        set(py_version "${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}.${PYTHON_VERSION_PATCH}")
        set(py_url "https://www.python.org/ftp/python/${py_version}/python-${py_version}-embed-amd64.zip")
        set(py_dir "${CMAKE_BINARY_DIR}/py_embedd")
        set(py_file "${py_dir}/py_embedd.${py_version}.zip")
        if(NOT EXISTS "${py_file}")
            ivw_debug_message(STATUS "Download ${py_url}")
            file(DOWNLOAD ${py_url} ${py_file} STATUS py_status ${show} LOG get_py_log)
            list(GET py_status 0 py_success)
            if(NOT py_success EQUAL 0)
                message(FATAL_ERROR "Could not download python embedd: '${py_url}':\n${get_py_log}")
            endif()
        endif()

        file(MAKE_DIRECTORY "${py_dir}/python")
        execute_process(COMMAND ${CMAKE_COMMAND} -E tar "xf" ${py_file}
                        WORKING_DIRECTORY "${py_dir}/python")
        file(APPEND ${py_dir}/python/python37._pth "import site")

        set(pip_file "${py_dir}/python/get-pip.py")
        if(NOT EXISTS "${pip_file}")
            set(pip_urll "https://bootstrap.pypa.io/get-pip.py")
            set(pip_url2 "https://github.com/pypa/get-pip/raw/master/get-pip.py")
            ivw_debug_message(STATUS "Download ${pip_url2}")
            file(DOWNLOAD "${pip_url2}" "${pip_file}" 
                 STATUS pip_status ${show} LOG get_pip_log)
            if(NOT pip_status EQUAL 0)
                file(REMOVE "${pip_file}")
                message(FATAL_ERROR "Could not download '${pip_url2}':\n ${get_pip_log}")
            endif()
        endif()

        if(NOT EXISTS ${py_dir}/python/Scripts)
            execute_process(COMMAND ./python.exe get-pip.py --no-warn-script-location 
                WORKING_DIRECTORY "${py_dir}/python")    
            execute_process(COMMAND ./python.exe -m pip install numpy --no-warn-script-location 
                WORKING_DIRECTORY "${py_dir}/python")
        endif()

        install(DIRECTORY "${py_dir}/python/" DESTINATION bin COMPONENT python)
    endif()
endif()

# Add scripts directory to pack
ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/scripts)
ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/processors)
ivw_make_package(InviwoPython3Module inviwo-module-python3)

#--------------------------------------------------------------------
# register license files
# Cannot get pybind11::module version property since it is an INTERFACE target
ivw_register_license_file(NAME "PyBind11" VERSION 2.2.4 MODULE Python3
    URL "https://github.com/pybind/pybind11"
    TYPE "BSD-style"
    FILES ${IVW_EXTENSIONS_DIR}/pybind11/LICENSE
)
