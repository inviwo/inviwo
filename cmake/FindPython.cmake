#
# Try to find PYTHON library and include path.
# Once done this will define
#
# For windows 
#
# PYTHON_FOUND
# PYTHON_INCLUDE_DIR
# PYTHON_LIBRARY_DIR
# PYTHON_VERSION
# PYTHON_NUMPY_FOUND
# PYTHON_NUMPY_INCLUDE_DIR
#
#
# FOR UNIX
#
# PYTHON_FOUND
# PYTHON_INCLUDE_DIR
# PYTHON_LIBRARY_DIR
# PYTHON_VERSION

INCLUDE(FindPackageHandleStandardArgs)

if(WIN32)
    set(MS_PATHS   
        [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\2.7\\InstallPath]
        [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\2.6\\InstallPath]
        [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\2.5\\InstallPath]
        [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\2.4\\InstallPath] )
          
    find_path(PYTHON_BASE_DIR python.exe ${PYTHON_BASE_DIR} ${MS_PATHS})

    set(PYTHON_LIBS python27.lib python26.lib python25.lib python24.lib python.lib)

    if(PYTHON_BASE_DIR)
        find_path(PYTHON_INCLUDE_DIR Python.h ${PYTHON_BASE_DIR}\\include)
        find_path(PYTHON_LIBRARY_DIR NAMES ${PYTHON_LIBS}  PATHS ${PYTHON_BASE_DIR}\\libs)
        find_library(PYTHON_LIBRARIES NAMES ${PYTHON_LIBS} PATHS ${PYTHON_LIBRARY_DIR})
        #get_filename_component(PYTHON_LIBRARIES ${PYTHON_LIBRARIES} NAME)
    endif(PYTHON_BASE_DIR)

    if(PYTHON_BASE_DIR AND IVW_MODULE_PYPACKAGES)
   
        #numpy
        execute_process(COMMAND "${PYTHON_BASE_DIR}/python.exe" "-c"
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

        execute_process(COMMAND "${PYTHON_BASE_DIR}/python.exe" "-c"
                        "try: import numpy; print numpy.get_include()\nexcept: pass\n"
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
            LIST(APPEND PYTHON_INCLUDE_DIR "${PYTHON_NUMPY_INCLUDE_DIR}")   
        else()  
            #message (FATAL_ERROR "Numpy not found")
            set(PYTHON_NUMPY_FOUND FALSE)
        endif()

        #Setuptools and EasyInstall
        execute_process(COMMAND "${PYTHON_BASE_DIR}/python.exe" "-c"
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
        execute_process(COMMAND "${PYTHON_BASE_DIR}/python.exe" "-c"
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
    
    FIND_PACKAGE_HANDLE_STANDARD_ARGS( Python DEFAULT_MSG PYTHON_LIBRARY_DIR PYTHON_INCLUDE_DIR )

    mark_as_advanced(PYTHON_FOUND)
    mark_as_advanced(PYTHON_VERSION)
    mark_as_advanced(PYTHON_INCLUDE_DIR)
    mark_as_advanced(PYTHON_LIBRARY_DIR)
    mark_as_advanced(PYTHON_LIBRARY)
    mark_as_advanced(PYTHON_LIBRARIES)
    mark_as_advanced(PYTHON_BASE_DIR)
    mark_as_advanced(PYTHON_NUMPY_FOUND)
    mark_as_advanced(PYTHON_NUMPY_INCLUDE_DIR)

elseif(APPLE)
    set(APPLE_INCLUDE_PATHS /System/Library/Frameworks/Python.framework/Headers)
    set(APPLE_LIB_PATHS /System/Library/Frameworks/Python.framework/Versions/Current/lib)
    find_path(PYTHON_INCLUDE_DIR Python.h ${APPLE_INCLUDE_PATHS})
    
    set(PYTHON_APPLE_LIBS libpython2.7.dylib libpython2.6.dylib libpython2.5.dylib libpython2.4.dylib)
    find_library(PYTHON_LIBRARIES NAMES ${PYTHON_APPLE_LIBS} PATHS ${APPLE_LIB_PATHS})
    
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(Python DEFAULT_MSG PYTHON_LIBRARIES PYTHON_INCLUDE_DIR)
    
    mark_as_advanced(PYTHON_FOUND)
    mark_as_advanced(PYTHON_INCLUDE_DIR)
    mark_as_advanced(PYTHON_LIBRARY_DIR)
    mark_as_advanced(PYTHON_LIBRARIES)

elseif(UNIX)
    set(UNIX_INCLUDE_PATHS /usr/local/pyenv/versions/2.7.6/include/python2.7 /usr/include/python2.7 /usr/include/python2.6 /usr/include/python2.5)
    set(UNIX_LIB_PATHS /usr/local/pyenv/versions/2.7.6/lib/ /usr/lib/i386-linux-gnu)
    find_path(PYTHON_INCLUDE_DIR Python.h ${UNIX_INCLUDE_PATHS})
    if(BUILD_SHARED_LIBS)
        set(PYTHON_UNIX_LIBS libpython2.7.so libpython2.6.so libpython2.5.so libpython2.4.so)
    else()
        set(PYTHON_UNIX_LIBS libpython2.7.a libpython2.6.a libpython2.5.a libpython2.4.a)
    endif()
    find_library(PYTHON_LIBRARIES NAMES ${PYTHON_UNIX_LIBS} PATHS ${UNIX_LIB_PATHS})
    
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(Python DEFAULT_MSG PYTHON_LIBRARIES PYTHON_INCLUDE_DIR)
    
    mark_as_advanced(PYTHON_FOUND)
    mark_as_advanced(PYTHON_INCLUDE_DIR)
    mark_as_advanced(PYTHON_LIBRARY_DIR)
    mark_as_advanced(PYTHON_LIBRARIES)
endif()

