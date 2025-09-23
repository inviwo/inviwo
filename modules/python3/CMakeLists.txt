# Inviwo Python3 Module
ivw_module(Python3)

find_package(Python3 COMPONENTS Interpreter Development NumPy REQUIRED)

set(HEADER_FILES
    include/modules/python3/layerpy.h
    include/modules/python3/opaquetypes.h
    include/modules/python3/polymorphictypehooks.h
    include/modules/python3/processortrampoline.h
    include/modules/python3/pybindflags.h
    include/modules/python3/pybindmodule.h
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
    include/modules/python3/pythonworkspacescripts.h
    include/modules/python3/pyutils.h
    include/modules/python3/volumepy.h
)
ivw_group("Header Files" ${HEADER_FILES})

set(SOURCE_FILES
    src/layerpy.cpp
    src/opaquetypes.cpp
    src/polymorphictypehooks.cpp
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
    src/pythonworkspacescripts.cpp
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

# Add script files
set(PYTHON_PROCESSORS_FILES
    processors/MandelbrotNumpy.py
    processors/MeshCreationTest.py
    processors/PythonExample.py
    processors/PythonMeshExample.py
    processors/PythonPickingExample.py
    processors/PythonVolumeExample.py
    processors/VolumeCreationTest.py
)
ivw_group("Python Processor Files" BASE processors ${PYTHON_PROCESSORS_FILES})

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
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SCRIPT_FILES} ${PYTHON_PROCESSORS_FILES})

find_package(pybind11 CONFIG REQUIRED)
target_compile_definitions(inviwo-module-python3 PUBLIC PYBIND11_DETAILED_ERROR_MESSAGES)
target_link_libraries(inviwo-module-python3 PUBLIC
    pybind11::embed
    Python3::Python
)

add_subdirectory(bindings)

if(TARGET inviwo-unittests-python3)
    add_dependencies(inviwo-unittests-python3 inviwopy)
    target_link_libraries(inviwo-unittests-python3 PUBLIC inviwo::python-helper)
endif()

# Add scripts directory to pack
ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/scripts)
ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/processors)

ivw_vcpkg_install(pybind11 MODULE Python3)
if("python" IN_LIST VCPKG_MANIFEST_FEATURES)
    ivw_vcpkg_install(python3 MODULE Python3 EXTRA_CODE [=[
        if(WIN32)
            set(pythondir python${Python3_VERSION_MAJOR})
            ivw_private_vcpkg_install_helper(
                FILES_VAR INFO_VCPKG_OWNED_FILES
                DIRNAME "tools/${pythondir}"
                EXTENSION "(py|dll)"
                DESTINATION ${IVW_INSTALL_PREFIX}tools/${pythondir}
                COMPONENT Application
            )
            install(DIRECTORY "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/tools/${pythondir}/Lib/site-packages"
                DESTINATION "${IVW_INSTALL_PREFIX}tools/${pythondir}/Lib"
                COMPONENT Application
            )
        else()
            set(pythondir python${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR})
            ivw_private_vcpkg_install_helper(
                FILES_VAR INFO_VCPKG_OWNED_FILES
                DIRNAME "lib/${pythondir}"
                EXTENSION "(py|dylib|so|dll)"
                DESTINATION ${IVW_INSTALL_PREFIX}lib/${pythondir}
                COMPONENT Application
            )
            install(DIRECTORY "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib/${pythondir}/site-packages"
                DESTINATION "${IVW_INSTALL_PREFIX}lib/${pythondir}"
                COMPONENT Application
            )
        endif()
    ]=])
endif()







