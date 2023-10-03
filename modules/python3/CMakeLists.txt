# Inviwo Python3 Module
ivw_module(Python3)

find_package(Python3 COMPONENTS Interpreter Development NumPy REQUIRED)

set(HEADER_FILES
    include/modules/python3/layerpy.h
    include/modules/python3/processortrampoline.h
    include/modules/python3/pybindflags.h
    include/modules/python3/pybindutils.h
    include/modules/python3/pyportutils.h
    include/modules/python3/python3module.h
    include/modules/python3/python3moduledefine.h
    include/modules/python3/pythonexecutionoutputobservable.h
    include/modules/python3/pythoninport.h
    include/modules/python3/pythoninterpreter.h
    include/modules/python3/pythonlogger.h
    include/modules/python3/pythonoutport.h
    include/modules/python3/pythonprocessorfactoryobject.h
    include/modules/python3/pythonprocessorfolderobserver.h
    include/modules/python3/pythonscript.h
    include/modules/python3/pyutils.h
    include/modules/python3/volumepy.h
)
ivw_group("Header Files" ${HEADER_FILES})

set(SOURCE_FILES
    src/layerpy.cpp
    src/processortrampoline.cpp
    src/pybindflags.cpp
    src/pybindutils.cpp
    src/pyportutils.cpp
    src/python3module.cpp
    src/pythonexecutionoutputobservable.cpp
    src/pythoninport.cpp
    src/pythoninterpreter.cpp
    src/pythonlogger.cpp
    src/pythonoutport.cpp
    src/pythonprocessorfactoryobject.cpp
    src/pythonprocessorfolderobserver.cpp
    src/pythonscript.cpp
    src/pyutils.cpp
    src/volumepy.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

# Add script files
set(SCRIPT_FILES
    scripts/ivw/__init__.py
    scripts/ivw/animation.py
    scripts/ivw/camera.py
    scripts/ivw/regression.py
    scripts/ivw/utils.py
)
ivw_group("Script Files" ${SCRIPT_FILES})

# Add Unittests
set(TEST_FILES
    tests/unittests/python3-unittest-main.cpp
    tests/unittests/numpy-test.cpp
    tests/unittests/python-representations.cpp
    tests/unittests/scripts-test.cpp
    tests/unittests/scripts/glm.py
    tests/unittests/scripts/grabreturnvalue.py
    tests/unittests/scripts/option_property.py
    tests/unittests/scripts/passvalues.py
    tests/unittests/scripts/simple_buffer_test.py
)
ivw_add_unittest(${TEST_FILES})

# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SCRIPT_FILES})

find_package(pybind11 CONFIG REQUIRED)
target_link_libraries(inviwo-module-python3 PUBLIC
    pybind11::embed
    Python3::Python
)
target_compile_definitions(inviwo-module-python3 PUBLIC PYBIND11_USE_SMART_HOLDER_AS_DEFAULT)


add_subdirectory(bindings)

if(TARGET inviwo-unittests-python3)
    add_dependencies(inviwo-unittests-python3 inviwopy)
endif()

# Add Python to package
option(IVW_PACKAGE_EMBED_PYTHON "Embed a python dist in the installer.\ 

 Otherwise a existing python installation is needed to run inviwo" ON)
if(BUILD_SHARED_LIBS AND IVW_PACKAGE_EMBED_PYTHON)
    if(WIN32)
        if(IVW_CFG_CMAKE_DEBUG)
            set(show SHOW_PROGRESS)
        else()
            set(show "")
        endif()

        set(py_version "${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}.${Python3_VERSION_PATCH}")
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
        file(APPEND ${py_dir}/python/python${Python3_VERSION_MAJOR}${Python3_VERSION_MINOR}._pth 
            "import site")

        set(pip_file "${py_dir}/python/get-pip.py")
        if(NOT EXISTS "${pip_file}")
            set(pip_url "https://bootstrap.pypa.io/get-pip.py")
            ivw_debug_message(STATUS "Download ${pip_url}")
            file(DOWNLOAD "${pip_url}" "${pip_file}" 
                 STATUS pip_status ${show} LOG get_pip_log)
            if(NOT pip_status EQUAL 0)
                file(REMOVE "${pip_file}")
                message(FATAL_ERROR "Could not download '${pip_url}':\n ${get_pip_log}")
            endif()
        endif()

        if(NOT EXISTS ${py_dir}/python/Scripts)
            execute_process(COMMAND ${py_dir}/python/python.exe -I get-pip.py --no-warn-script-location 
                WORKING_DIRECTORY "${py_dir}/python")
            execute_process(COMMAND ${py_dir}/python/python.exe -I -m pip install numpy --isolated --no-warn-script-location 
                WORKING_DIRECTORY "${py_dir}/python")
        endif()

        install(DIRECTORY "${py_dir}/python/" 
            DESTINATION ${IVW_RUNTIME_INSTALL_DIR} 
            COMPONENT Application
            CONFIGURATIONS Release MinSizeRel RelWithDebInfo
        )
        install(DIRECTORY "${py_dir}/python/" 
            DESTINATION ${IVW_RUNTIME_DEBUG_INSTALL_DIR}
            COMPONENT Application
            CONFIGURATIONS Debug
        )
    endif()
endif()

# Add scripts directory to pack
ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/scripts)
ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/processors)
