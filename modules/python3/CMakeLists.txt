#--------------------------------------------------------------------
# Inviwo Python3 Module
ivw_module(Python3)

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
                    set(PYTHON_INCLUDE_DIR "${pypath}/Headers")         
                endif()
             endif()
        endif()
    endif()    
endif()

if(NOT PYTHONLIBS_FOUND)
    set(Python_ADDITIONAL_VERSIONS 3.5)
    find_package(PythonLibs 3)
endif()

if (PYTHONLIBS_FOUND)
    ivw_add_definition(IVW_PYTHON_AVAILABLE)
endif()

ivw_include_directories(${PYTHON_INCLUDE_DIR})
get_filename_component(PYTHON_LIBRARY_DIR "${PYTHON_LIBRARY}" PATH)

if(PYTHON_DEBUG_LIBRARY)
    option(IVW_PYTHON_LINK_DEBUG "Link python debug libraries" OFF)
endif()

if(IVW_PYTHON_LINK_DEBUG)
    ivw_add_definition(IVW_PYTHON_LINK_DEBUG)
    add_dependency_libs_to_module(${PYTHON_LIBRARIES})
    ivw_link_directories(${PYTHON_LIBRARY_DIR})
else()
    list(LENGTH PYTHON_LIBRARIES NUM_DIRS)
    if(${NUM_DIRS} EQUAL 4)
        list(GET PYTHON_LIBRARIES 0 LIB1_TYPE)
        list(GET PYTHON_LIBRARIES 1 LIB1_PATH)
        list(GET PYTHON_LIBRARIES 2 LIB2_TYPE)
        list(GET PYTHON_LIBRARIES 3 LIB2_PATH)

        list(GET PYTHON_LIBRARY_DIR 0 LIB_DIR1_TYPE)
        list(GET PYTHON_LIBRARY_DIR 1 LIB_DIR1_PATH)
        list(GET PYTHON_LIBRARY_DIR 2 LIB_DIR2_TYPE)
        list(GET PYTHON_LIBRARY_DIR 3 LIB_DIR2_PATH)

        if(${LIB1_TYPE} STREQUAL "optimized")
            add_dependency_libs_to_module(${LIB1_PATH})
        elseif(${LIB2_TYPE} STREQUAL "optimized")
            add_dependency_libs_to_module(${LIB2_PATH})
        endif()

        if(${LIB_DIR1_TYPE} STREQUAL "optimized")
            ivw_link_directories(${LIB_DIR1_PATH})
        elseif(${LIB_DIR2_TYPE} STREQUAL "optimized")
            ivw_link_directories(${LIB_DIR2_PATH})
        endif()
    else()
        add_dependency_libs_to_module(${PYTHON_LIBRARIES})
        ivw_link_directories(${PYTHON_LIBRARY_DIR})
    endif()
endif()

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
# if(IVW_PACKAGE_PROJECT AND BUILD_SHARED_LIBS)
   # get_filename_component(FILENAME "${PYTHON3_LIBRARIES}" NAME_WE)
   # if(WIN32)
        # find_file(PYTHON3_DLL ${FILENAME}.dll)
        # mark_as_advanced(PYTHON3_DLL)
        # install(FILES ${PYTHON3_DLL} DESTINATION bin COMPONENT modules)
        # find_file(PYTHON3_MSVCR_DLL msvcr100.dll)
        # install(FILES ${PYTHON3_MSVCR_DLL} DESTINATION bin COMPONENT modules)
        # find_file(PYTHON3_MSVCP_DLL msvcp100.dll)
        # install(FILES ${PYTHON3_MSVCP_DLL} DESTINATION bin COMPONENT modules)
   # endif()
# endif()
# Add scripts directory to pack
ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/scripts)

#--------------------------------------------------------------------
# Check that python is built with the right arch.
find_package(PythonInterp 3)
if(PYTHON_EXECUTABLE AND (${PYTHON_VERSION_MAJOR} EQUAL 3))
    execute_process(COMMAND ${PYTHON_EXECUTABLE} -c "import struct;print(8*struct.calcsize('P'), end='')"
        OUTPUT_VARIABLE PYTHON_EXECUTABLE_BITS )
    
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(SYS_BITS 64)
    else()
        set(SYS_BITS 32)
    endif()

    if(NOT SYS_BITS EQUAL PYTHON_EXECUTABLE_BITS)
        ivw_message(FATAL_ERROR "Python is built for ${PYTHON_EXECUTABLE_BITS}bits"
                    " while target output is ${SYS_BITS}bits")
    endif()
endif()