#--------------------------------------------------------------------
# Inviwo Python3 Module
ivw_module(Python3)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    #${CMAKE_CURRENT_SOURCE_DIR}/python3processor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/defaultinterface/pycamera.h
    ${CMAKE_CURRENT_SOURCE_DIR}/defaultinterface/pycanvas.h    
    ${CMAKE_CURRENT_SOURCE_DIR}/defaultinterface/pylist.h
    ${CMAKE_CURRENT_SOURCE_DIR}/defaultinterface/pyproperties.h
    ${CMAKE_CURRENT_SOURCE_DIR}/defaultinterface/pyutil.h
    ${CMAKE_CURRENT_SOURCE_DIR}/defaultinterface/pyvolume.h
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
    #${CMAKE_CURRENT_SOURCE_DIR}/python3processor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/defaultinterface/pycamera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/defaultinterface/pycanvas.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/defaultinterface/pylist.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/defaultinterface/pyproperties.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/defaultinterface/pyutil.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/defaultinterface/pyvolume.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/defaultinterface/pyinfo.cpp
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
# Add shaders
#set(SHADER_FILES
    #${CMAKE_CURRENT_SOURCE_DIR}/glsl/python3processor.frag
#)
#ivw_group("Shader Files" ${SHADER_FILES})


#--------------------------------------------------------------------
# Add Unittests
#set(TEST_FILES
#	${CMAKE_CURRENT_SOURCE_DIR}/tests/python3-test.cpp
#)
#ivw_add_unittest(${TEST_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES} ${SCRIPT_FILES})

#--------------------------------------------------------------------
# Add shader directory to pack
# ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/glsl)

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
