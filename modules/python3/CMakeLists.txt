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


set(Python_ADDITIONAL_VERSIONS 3.5)
find_package(PythonLibs 3)

if (PYTHONLIBS_FOUND)
    ivw_add_definition(IVW_PYTHON_AVAILABLE)
endif()

ivw_include_directories(${PYTHON_INCLUDE_DIR})
get_filename_component(PYTHON_LIBRARY_DIR "${PYTHON_LIBRARY}" PATH)
ivw_link_directories(${PYTHON_LIBRARY_DIR})

if(PYTHON_DEBUG_LIBRARY)
    ivw_add_definition(IVW_PYTHON_LINK_DEBUG)
endif()

add_dependency_libs_to_module(${PYTHON_LIBRARIES})




#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/defaultinterface/pycamera.h
    ${CMAKE_CURRENT_SOURCE_DIR}/defaultinterface/pycanvas.h
    ${CMAKE_CURRENT_SOURCE_DIR}/defaultinterface/pyinfo.h
    ${CMAKE_CURRENT_SOURCE_DIR}/defaultinterface/pylist.h
    ${CMAKE_CURRENT_SOURCE_DIR}/defaultinterface/pyproperties.h
    ${CMAKE_CURRENT_SOURCE_DIR}/defaultinterface/pyutil.h
    ${CMAKE_CURRENT_SOURCE_DIR}/defaultinterface/pyvolume.h
    ${CMAKE_CURRENT_SOURCE_DIR}/pyinviwo.h
    ${CMAKE_CURRENT_SOURCE_DIR}/pyinviwoobserver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/pythonexecutionoutputobservable.h
    ${CMAKE_CURRENT_SOURCE_DIR}/pythonincluder.h
    ${CMAKE_CURRENT_SOURCE_DIR}/pythoninterface/pymethod.h
    ${CMAKE_CURRENT_SOURCE_DIR}/pythoninterface/pymodule.h
    ${CMAKE_CURRENT_SOURCE_DIR}/pythoninterface/pyvalueparser.h
    ${CMAKE_CURRENT_SOURCE_DIR}/pythonscript.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/defaultinterface/pycamera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/defaultinterface/pycanvas.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/defaultinterface/pyinfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/defaultinterface/pylist.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/defaultinterface/pyproperties.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/defaultinterface/pyutil.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/defaultinterface/pyvolume.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pyinviwo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pyinviwoobserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pythonexecutionoutputobservable.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pythoninterface/pymethod.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pythoninterface/pymodule.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pythoninterface/pyvalueparser.cpp
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
