# Adapted Tiff image library version 4.4.0

Downloaded from http://download.osgeo.org/libtiff/

Modifications and comment blocks in CMake files are indicated by `#[===[ Inviwo modification`.

Changes to 
* `CMakeLists.txt`
    - Disabled all subdirectories except libtiff
    - Reduced CMake logging
    - Added Inviwo-specific CMake macros for packaging, suppressing warnings, etc.
* `libtiff/CMakeLists.txt`
    - Removed export and install targets since not needed by Inviwo
    - Removed targets for mkg3states and faxtable. Both not being part of the libtiff library.
* `cmake/CXXLibrary.cmake`
    - Toggled option to OFF since `tiffxx` is not used 
* `cmake/DeflateCodec.cmake` and `cmake/JPEGCodec.cmake`:
    - Added `CONFIG` to `find_package()` to not default to external packages (5073a4b)
