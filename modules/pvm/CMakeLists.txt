add_subdirectory(ext/tidds)

#--------------------------------------------------------------------
# Inviwo PVM Module
ivw_module(PVM)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/mpvmvolumereader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/pvmvolumereader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/pvmvolumewriter.h
)
ivw_group("Header Files" ${HEADER_FILES})


#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/mpvmvolumereader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pvmvolumereader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pvmvolumewriter.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${MOC_FILES} ${HEADER_FILES})
target_link_libraries(inviwo-module-pvm PRIVATE tidds)

ivw_register_license_file(NAME "Tiny DDS Package" MODULE PVM TYPE "LGPL"
    URL https://github.com/Eyescale/Equalizer
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/ext/tidds/README
          ${CMAKE_CURRENT_SOURCE_DIR}/ext/tidds/LGPL
)

ivw_make_package(InviwoPVMModule inviwo-module-pvm)
