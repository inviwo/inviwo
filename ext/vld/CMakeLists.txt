set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/vld.h
    ${CMAKE_CURRENT_SOURCE_DIR}/vld_def.h
)

ivw_group("Header Files"   ${HEADER_FILES})
ivw_group("Settings Files" ${CMAKE_CURRENT_SOURCE_DIR}/vld.ini)
ivw_group("CMake Files"    ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt)


if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
    link_directories(${IVW_EXTENSIONS_DIR}/vld/lib/Win64)
else ()
    link_directories(${IVW_EXTENSIONS_DIR}/vld/lib/Win32)
endif()

if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
    set(arch "Win64")
else()
    set(arch "Win32")
endif()

add_custom_target(vld
    ${CMAKE_COMMAND} -E copy_if_different 
        ${IVW_EXTENSIONS_DIR}/vld/vld.ini
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$(ConfigurationName)/vld.ini

    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
        ${IVW_EXTENSIONS_DIR}/vld/bin/${arch}/vld_x64.dll
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$(ConfigurationName)/vld_x64.dll 
    
    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
        ${IVW_EXTENSIONS_DIR}/vld/bin/${arch}/dbghelp.dll
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$(ConfigurationName)/dbghelp.dll
    
    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
        ${IVW_EXTENSIONS_DIR}/vld/bin/${arch}/Microsoft.DTfW.DHL.manifest
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$(ConfigurationName)/Microsoft.DTfW.DHL.manifest    

    DEPENDS 
        ${IVW_EXTENSIONS_DIR}/vld/vld.ini
        ${IVW_EXTENSIONS_DIR}/vld/bin/${arch}/vld_x64.dll
        ${IVW_EXTENSIONS_DIR}/vld/bin/${arch}/dbghelp.dll
        ${IVW_EXTENSIONS_DIR}/vld/bin/${arch}/Microsoft.DTfW.DHL.manifest

    COMMENT "Setup VLD memory leak checking"
    VERBATIM
    SOURCES ${HEADER_FILES} 
        ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
        ${CMAKE_CURRENT_SOURCE_DIR}/vld.ini
)
set_target_properties(vld PROPERTIES FOLDER "ext")