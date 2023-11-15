# Inviwo PVM Module
ivw_module(PVM)

set(HEADER_FILES
    include/modules/pvm/mpvmvolumereader.h
    include/modules/pvm/pvmmodule.h
    include/modules/pvm/pvmmoduledefine.h
    include/modules/pvm/pvmvolumereader.h
    include/modules/pvm/pvmvolumewriter.h
)
ivw_group("Header Files" ${HEADER_FILES})

set(SOURCE_FILES
    src/mpvmvolumereader.cpp
    src/pvmmodule.cpp
    src/pvmvolumereader.cpp
    src/pvmvolumewriter.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

# Create module
ivw_create_module(${SOURCE_FILES} ${MOC_FILES} ${HEADER_FILES})

add_subdirectory(ext/tidds)


find_package(tidds CONFIG REQUIRED)
target_link_libraries(inviwo-module-pvm PRIVATE tidds::tidds)

ivw_register_license_file(NAME "Tiny DDS Package" MODULE PVM TYPE "LGPL"
    URL https://github.com/Eyescale/Equalizer
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/ext/tidds/tidds/README
          ${CMAKE_CURRENT_SOURCE_DIR}/ext/tidds/tidds/LGPL
)
