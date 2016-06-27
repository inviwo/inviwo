#--------------------------------------------------------------------
# Inviwo Python3 Module
ivw_module(Python3)

include(CMakeParseArguments)

# Try and find python 3 installation
string(FIND "${CMAKE_PREFIX_PATH}" python3 haspython) #Skip if python3 is already in CMAKE_PREFIX_PATH
if(NOT ${haspython} GREATER 0) 
   if(APPLE)
        set(pycellar "/usr/local/Cellar/python3" )
        if(EXISTS "${pycellar}")
            file(GLOB pythonversions RELATIVE "${pycellar}" "${pycellar}/?.?.?")
            list(LENGTH pythonversions len)
            if(${len} GREATER 0)
                list(GET pythonversions -1 latest)
                string(SUBSTRING "${latest}" 0 3 pyversion)
                set(pypath "${pycellar}/${latest}/Frameworks/Python.framework/Versions/${pyversion}") 
                if(EXISTS "${pypath}")
                    ivw_debug_message(STATUS "Found python at: ${pypath}")
                    list(APPEND CMAKE_PREFIX_PATH "${pypath}")
                    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} PARENT_SCOPE)
                endif()
             endif()
        endif()
    endif()    
endif()
 


set(Python_ADDITIONAL_VERSIONS 3.5)

# ONLY find_package for python if it has not been found, 
# OR if it has been found before, only call find_package if previous version was 2.x.x 
# https://gitlab.kitware.com/cmake/cmake/issues/14920
# https://gitlab.kitware.com/cmake/cmake/issues/16142 
if(NOT PYTHONLIBS_FOUND)
    find_package(PythonLibs 3)
elseif(${PYTHONLIBS_VERSION_STRING} VERSION_LESS 3)
    find_package(PythonLibs 3)
endif()

if (PYTHONLIBS_FOUND)
    ivw_add_definition(IVW_PYTHON_AVAILABLE)
endif()

ivw_include_directories(${PYTHON_INCLUDE_DIR})

if(PYTHON_DEBUG_LIBRARY)
    option(IVW_PYTHON_LINK_DEBUG "Link python debug libraries" OFF)
endif()

if(IVW_PYTHON_LINK_DEBUG)
    ivw_add_definition(IVW_PYTHON_LINK_DEBUG)
endif()

# Parse PYTHON_LIBRARY
set(options "")
set(oneValueArgs optimized debug)
set(multiValueArgs "")
cmake_parse_arguments(PYTHON_LIBS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${PYTHON_LIBRARY})
if(PYTHON_LIBS_optimized)
    set(PYTHON_LIB ${PYTHON_LIBS_optimized})
elseif(PYTHON_LIBS_UNPARSED_ARGUMENTS)
    set(PYTHON_LIB ${PYTHON_LIBS_UNPARSED_ARGUMENTS})
elseif(PYTHON_LIBS_debug)
     set(PYTHON_LIB ${PYTHON_LIBS_debug})
else()
     ivw_message(FATAL_ERROR "Python could not determain the python lib file")
endif()

ivw_library_bits(${PYTHON_LIB} lib_bits)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(sys_bits 64)
else()
    set(sys_bits 32)
endif()

if(NOT ${lib_bits} EQUAL ${sys_bits})
    ivw_message(FATAL_ERROR "Python (${PYTHON_LIB}) is built for ${lib_bits}bits "
                            "while target output is ${sys_bits}bits")
endif()
add_dependency_libs_to_module(${PYTHON_LIB})


#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/defaultinterface/pycamera.h
    ${CMAKE_CURRENT_SOURCE_DIR}/defaultinterface/pycanvas.h
    ${CMAKE_CURRENT_SOURCE_DIR}/defaultinterface/pylist.h
    ${CMAKE_CURRENT_SOURCE_DIR}/defaultinterface/pyproperties.h
    ${CMAKE_CURRENT_SOURCE_DIR}/defaultinterface/pyutil.h
    ${CMAKE_CURRENT_SOURCE_DIR}/defaultinterface/pyvolume.h
    ${CMAKE_CURRENT_SOURCE_DIR}/pyinviwo.h
    ${CMAKE_CURRENT_SOURCE_DIR}/pythonexecutionoutputobservable.h
    ${CMAKE_CURRENT_SOURCE_DIR}/pythonincluder.h
    ${CMAKE_CURRENT_SOURCE_DIR}/pythoninterface/pythonparameterparser.h
    ${CMAKE_CURRENT_SOURCE_DIR}/pythoninterface/pyvalueparser.h
    ${CMAKE_CURRENT_SOURCE_DIR}/pythonlogger.h
    ${CMAKE_CURRENT_SOURCE_DIR}/pythonscript.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/defaultinterface/pycamera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/defaultinterface/pycanvas.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/defaultinterface/pylist.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/defaultinterface/pyproperties.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/defaultinterface/pyutil.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/defaultinterface/pyvolume.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pyinviwo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pythonexecutionoutputobservable.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pythoninterface/pythonparameterparser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pythoninterface/pyvalueparser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pythonlogger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pythonscript.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})


#--------------------------------------------------------------------
# Add script files
set(SCRIPT_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/outputredirector.py
)
ivw_group("Script Files" ${SCRIPT_FILES})


#--------------------------------------------------------------------
# Add Unittests
#set(TEST_FILES
#    ${CMAKE_CURRENT_SOURCE_DIR}/tests/python3-test.cpp
#)
#ivw_add_unittest(${TEST_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SCRIPT_FILES})


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
