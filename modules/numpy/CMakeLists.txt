#--------------------------------------------------------------------
# Inviwo NumPy Module
ivw_module(NumPy)

if(PYTHON_EXECUTABLE)

    #Check if numpy exist 
    execute_process(COMMAND "${PYTHON_EXECUTABLE}" "-c"
                    "try: import numpy; print(numpy.__version__);\nexcept: print('failed')\n"
                    RESULT_VARIABLE NUMPY_STATUS
                    OUTPUT_VARIABLE NUMPY_OUTPUT_VERSION
                    ERROR_VARIABLE NUMPY_ERROR
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                   )

    if (NUMPY_OUTPUT_VERSION MATCHES "failed")
        ivw_message (FATAL_ERROR "Missing Python Package Numpy.")
    endif()


    #Get include include directories for numpy
    execute_process(COMMAND "${PYTHON_EXECUTABLE}" "-c"
                    "try: import numpy; print(numpy.get_include())\nexcept: pass\n"
                    RESULT_VARIABLE NUMPY_STATUS
                    OUTPUT_VARIABLE NUMPY_OUTPUT_INCLUDES
                    ERROR_VARIABLE NUMPY_ERROR
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                   )

    if (NUMPY_STATUS MATCHES 0)
        set(PYTHON_NUMPY_FOUND TRUE CACHE INTERNAL "Python numpy development package is available")     
        find_path(PYTHON_NUMPY_INCLUDE_DIR arrayobject.h
                  "${NUMPY_OUTPUT_INCLUDES}\\numpy"          
                  DOC "Directory where the arrayobject.h header file can be found. This file is part of the numpy package"
                 )
        LIST(APPEND PYTHON_INCLUDE_DIR ${PYTHON_NUMPY_INCLUDE_DIR})   
        set(PYTHON_NUMPY_FOUND TRUE)
    else()  
        ivw_message (FATAL_ERROR "Failed to find numpy incude directory.")
        set(PYTHON_NUMPY_FOUND FALSE)
    endif()

    mark_as_advanced(PYTHON_NUMPY_FOUND)
    mark_as_advanced(PYTHON_NUMPY_INCLUDE_DIR)
endif()



#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    #${CMAKE_CURRENT_SOURCE_DIR}/numpyprocessor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/numpyobjectwrapper.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/numpymandelbrot.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/numpymeshcreatetest.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/numpyvolume.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    #${CMAKE_CURRENT_SOURCE_DIR}/numpyprocessor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/numpyobjectwrapper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/numpymandelbrot.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/numpymeshcreatetest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/numpyvolume.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})


#--------------------------------------------------------------------
# Add shaders
set(SCRIPT_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/data/scripts/mandelbrot.py
    ${CMAKE_CURRENT_SOURCE_DIR}/data/scripts/numpyvolumeprocessor.py
    ${CMAKE_CURRENT_SOURCE_DIR}/data/scripts/numpymeshcreatetest.py
)
ivw_group("Shader Files" ${SHADER_FILES})


#--------------------------------------------------------------------
# Add Unittests
set(TEST_FILES
	#${CMAKE_CURRENT_SOURCE_DIR}/tests/numpy-test.cpp
)
ivw_add_unittest(${TEST_FILES})


ivw_include_directories(${PYTHON_NUMPY_INCLUDE_DIR})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SCRIPT_FILES})

#--------------------------------------------------------------------
# Add shader directory to pack
# ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/glsl)

#target_link_libraries(${${mod}_target} pybind11::module)