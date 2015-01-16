#
# Try to find PYTHON library and include path.
# Once done this will define
#
# For windows 
#
# PYTHON3_FOUND
# PYTHON3_INCLUDE_DIR
# PYTHON3_LIBRARY_DIR
# PYTHON3_VERSION
# PYTHON3_NUMPY_FOUND
# PYTHON3_NUMPY_INCLUDE_DIR
#
#
# FOR UNIX
#
# PYTHON3_FOUND
# PYTHON3_INCLUDE_DIR
# PYTHON3_LIBRARY_DIR
# PYTHON3_VERSION

INCLUDE(FindPackageHandleStandardArgs)

if(WIN32)
    set(MS_PATHS   
        [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\3.4\\InstallPath] 
        [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\3.3\\InstallPath] 
        [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\3.2\\InstallPath] )
          
    find_path(PYTHON3_BASE_DIR python.exe ${PYTHON3_BASE_DIR} ${MS_PATHS})

    set(PYTHON3_LIBS python34.lib python33.lib python32.lib python.lib)

    if(PYTHON3_BASE_DIR)
        find_path(PYTHON3_INCLUDE_DIR Python.h ${PYTHON3_BASE_DIR}\\include)
        find_path(PYTHON3_LIBRARY_DIR NAMES ${PYTHON3_LIBS}  PATHS ${PYTHON3_BASE_DIR}\\libs)
        find_library(PYTHON3_LIBRARIES NAMES ${PYTHON3_LIBS} PATHS ${PYTHON3_LIBRARY_DIR})
        #get_filename_component(PYTHON3_LIBRARIES ${PYTHON3_LIBRARIES} NAME)
    endif(PYTHON3_BASE_DIR)

    if(PYTHON3_BASE_DIR AND IVW_MODULE_PYPACKAGES)
   
        #numpy
        execute_process(COMMAND "${PYTHON3_BASE_DIR}/python.exe" "-c"
                        "try: import numpy; print(numpy.__version__);\nexcept: print('failed')\n"
                        RESULT_VARIABLE NUMPY_STATUS
                        OUTPUT_VARIABLE NUMPY_OUTPUT_VERSION
                        ERROR_VARIABLE NUMPY_ERROR
                        OUTPUT_STRIP_TRAILING_WHITESPACE
                       )

        if (NUMPY_OUTPUT_VERSION MATCHES "failed")
            message (FATAL_ERROR "Missing Missing Python Package Numpy. Install from http://www.lfd.uci.edu/~gohlke/pythonlibs/#numpy ")
        endif()

        #message (FATAL_ERROR "Numpy Status " "${NUMPY_STATUS}" )
        #message (FATAL_ERROR "Numpy Version " "${NUMPY_OUTPUT_VERSION}" )
        #message (FATAL_ERROR "Numpy Error " "${NUMPY_ERROR}" )

        execute_process(COMMAND "${PYTHON3_BASE_DIR}/python.exe" "-c"
                        "try: import numpy; print numpy.get_include()\nexcept: pass\n"
                        RESULT_VARIABLE NUMPY_STATUS
                        OUTPUT_VARIABLE NUMPY_OUTPUT_INCLUDES
                        ERROR_VARIABLE NUMPY_ERROR
                        OUTPUT_STRIP_TRAILING_WHITESPACE
                       )

        if (NUMPY_STATUS MATCHES 0)
            set(PYTHON3_NUMPY_FOUND TRUE CACHE INTERNAL "Python numpy development package is available")     
            find_path(PYTHON3_NUMPY_INCLUDE_DIR arrayobject.h
                      "${NUMPY_OUTPUT_INCLUDES}\\numpy"          
                      DOC "Directory where the arrayobject.h header file can be found. This file is part of the numpy package"
                     )
            LIST(APPEND PYTHON3_INCLUDE_DIR "${PYTHON3_NUMPY_INCLUDE_DIR}")   
        else()  
            #message (FATAL_ERROR "Numpy not found")
            set(PYTHON3_NUMPY_FOUND FALSE)
        endif()

        #Setuptools and EasyInstall
        execute_process(COMMAND "${PYTHON3_BASE_DIR}/python.exe" "-c"
                        "try: from setuptools.command import easy_install; import setuptools; print(setuptools.__version__);\nexcept: print('failed')\n"
                        RESULT_VARIABLE SETUPTOOLS_STATUS
                        OUTPUT_VARIABLE SETUPTOOLS_OUTPUT_VERSION
                        ERROR_VARIABLE SETUPTOOLS_ERROR
                        OUTPUT_STRIP_TRAILING_WHITESPACE
                       )
        if (SETUPTOOLS_OUTPUT_VERSION MATCHES "failed")
            message (FATAL_ERROR "Missing Python Package Setuptools. Install from http://www.lfd.uci.edu/~gohlke/pythonlibs/#setuptools ")
        endif()     

        #pip
        execute_process(COMMAND "${PYTHON3_BASE_DIR}/python.exe" "-c"
                        "try: import pip; print(pip.__version__);\nexcept: print('failed')\n"
                        RESULT_VARIABLE PIP_STATUS
                        OUTPUT_VARIABLE PIP_OUTPUT_VERSION
                        ERROR_VARIABLE PIP_ERROR
                        OUTPUT_STRIP_TRAILING_WHITESPACE
                       )
        if (PIP_OUTPUT_VERSION MATCHES "failed")
            message (FATAL_ERROR "Missing Python Package Pip. Install from http://www.lfd.uci.edu/~gohlke/pythonlibs/#pip ")
        endif()

    endif()    
    
    FIND_PACKAGE_HANDLE_STANDARD_ARGS( Python DEFAULT_MSG PYTHON3_LIBRARY_DIR PYTHON3_INCLUDE_DIR )

    mark_as_advanced(PYTHON3_FOUND)
    mark_as_advanced(PYTHON3_VERSION)
    mark_as_advanced(CLEAR PYTHON3_INCLUDE_DIR)
    mark_as_advanced(CLEAR PYTHON3_LIBRARY_DIR)
    mark_as_advanced(PYTHON3_LIBRARY)
    mark_as_advanced(CLEAR PYTHON3_LIBRARIES)
    mark_as_advanced(CLEAR PYTHON3_BASE_DIR)
	mark_as_advanced(CLEAR PYTHON3_EXECUTABLE)
    mark_as_advanced(PYTHON3_NUMPY_FOUND)
    mark_as_advanced(PYTHON3_NUMPY_INCLUDE_DIR)

elseif(APPLE)
    set(APPLE_INCLUDE_PATHS /System/Library/Frameworks/Python.framework/Headers)
    set(APPLE_LIB_PATHS /System/Library/Frameworks/Python.framework/Versions/Current/lib)
    find_path(PYTHON3_INCLUDE_DIR Python.h ${APPLE_INCLUDE_PATHS})
    
    set(PYTHON3_APPLE_LIBS libpython2.7.dylib libpython2.6.dylib libpython2.5.dylib libpython2.4.dylib)
    find_library(PYTHON3_LIBRARIES NAMES ${PYTHON3_APPLE_LIBS} PATHS ${APPLE_LIB_PATHS})
    
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(Python DEFAULT_MSG PYTHON3_LIBRARIES PYTHON3_INCLUDE_DIR)
    
    mark_as_advanced(PYTHON3_FOUND)
    mark_as_advanced(CLEAR PYTHON3_INCLUDE_DIR)
    mark_as_advanced(CLEAR PYTHON3_LIBRARY_DIR)
    mark_as_advanced(CLEAR PYTHON3_LIBRARIES)

elseif(UNIX)
    set(UNIX_INCLUDE_PATHS /usr/include/python3/ /usr/include/python3.2/ /usr/include/python3.3/ /usr/include/python3.4/)
    set(UNIX_LIB_PATHS  /usr/lib/x86_64-linux-gnu/libpy /usr/lib/i386-linux-gnu)
    find_path(PYTHON3_INCLUDE_DIR Python.h ${UNIX_INCLUDE_PATHS})
    if(BUILD_SHARED_LIBS)
        set(PYTHON3_UNIX_LIBS libpython3.so libpython3.2mu.so libpython3.3m.so libpython3.4m.so)
    else()
        set(PYTHON3_UNIX_LIBS libpython3.a libpython3.2mu.a libpython3.3m.so libpython3.4m.a)
    endif()
    find_library(PYTHON3_LIBRARIES NAMES ${PYTHON3_UNIX_LIBS} PATHS ${UNIX_LIB_PATHS})
    
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(Python DEFAULT_MSG PYTHON3_LIBRARIES PYTHON3_INCLUDE_DIR)
    
    mark_as_advanced(PYTHON3_FOUND)
    mark_as_advanced(CLEAR PYTHON3_INCLUDE_DIR)
    mark_as_advanced(CLEAR PYTHON3_LIBRARY_DIR)
    mark_as_advanced(CLEAR PYTHON3_LIBRARIES)
endif()

