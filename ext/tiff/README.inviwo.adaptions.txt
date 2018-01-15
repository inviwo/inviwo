
Adapted Tiff image library version 4.0.9 taken from:
http://www.libtiff.org/ 
http://download.osgeo.org/libtiff/

Mainly changes to CMakeLists.txt to reduce information in CMake (logging and options). Removed options and set them to default values instead. Disabled searching for jpeg and zlib and set them to the ones included in inviwo instead.

Note: 
- Disabled search for jpeg12 since inviwo does not have it and we do not want to rely on system libraries.
- Disabled search for OpenGL and GLUT since they are only used in tiff/tools
- LibLZMA is still an option and set to ON by default. TODO: We need to include it if we want to distribute it when enabled ( https://git.tukaani.org/?p=lzma.git;a=summary ).
- Disabled c++ version tiffxx since it is not used yet.
- Disabled all subdirectories except libtiff
- Added alias inviwo::tiff in libtiff/CmakeLists.txt
- Suppress build warnings for MSVC builds (`_CRT_SECURE_NO_WARNINGS` and C4996)
- Temporarily disabled installing binary and header files:  `install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lib...` in `CMakeLists.txt` and `install(FILES ${tiff_HEADERS} ...` in `libtiff/CMakeLists.txt`

Examples of changes:

Disable option and set variable afterwards:
#option(extra-warnings "Enable extra compiler warnings" OFF)
set(extra-warnings OFF)

Disable option and find_package. Set package, set(ZLIB_LIBRARIES ZLIB::ZLIB), and always set ZLIB_SUPPORT to 1
#option(zlib "use zlib (required for Deflate compression)" ON)
#if (zlib)
  #find_package(ZLIB)
  set(ZLIB_LIBRARIES ZLIB::ZLIB)
#endif()
#set(ZLIB_SUPPORT 0)
#if(ZLIB_FOUND)
  set(ZLIB_SUPPORT 1)
#endif()
