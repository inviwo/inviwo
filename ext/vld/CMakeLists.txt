set(HEADER_FILES
    vld.h
    vld_def.h
)

ivw_group("Header Files"   ${HEADER_FILES})
ivw_group("Settings Files" vld.ini)
ivw_group("CMake Files"    CMakeLists.txt)

set(arch "Win64")
set(dll "vld_x64.dll")

add_custom_target(vld
    ${CMAKE_COMMAND} -E copy_if_different 
        ${CMAKE_CURRENT_SOURCE_DIR}/vld.ini
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$(ConfigurationName)/vld.ini

    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
        ${CMAKE_CURRENT_SOURCE_DIR}/bin/${arch}/${dll}
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$(ConfigurationName)/${dll} 

    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
        ${CMAKE_CURRENT_SOURCE_DIR}/bin/${arch}/dbghelp.dll
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$(ConfigurationName)/dbghelp.dll

    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
        ${CMAKE_CURRENT_SOURCE_DIR}/bin/${arch}/Microsoft.DTfW.DHL.manifest
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$(ConfigurationName)/Microsoft.DTfW.DHL.manifest

    DEPENDS 
        ${CMAKE_CURRENT_SOURCE_DIR}/vld.ini
        ${CMAKE_CURRENT_SOURCE_DIR}/bin/${arch}/${dll}
        ${CMAKE_CURRENT_SOURCE_DIR}/bin/${arch}/dbghelp.dll
        ${CMAKE_CURRENT_SOURCE_DIR}/bin/${arch}/Microsoft.DTfW.DHL.manifest

    COMMENT "Setup VLD memory leak checking"
    VERBATIM
    SOURCES ${HEADER_FILES} 
        ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
        ${CMAKE_CURRENT_SOURCE_DIR}/vld.ini
)
set_target_properties(vld PROPERTIES FOLDER "ext")

# Create an interface target to link to.
add_library(vld-interface INTERFACE)
target_link_libraries(vld-interface INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/lib/${arch}/vld.lib)
target_compile_definitions(vld-interface INTERFACE IVW_ENABLE_MSVC_MEM_LEAK_TEST)
target_include_directories(vld-interface INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
add_dependencies(vld-interface vld)

# package
install(FILES 
        vld.ini
        bin/${arch}/${dll}
        bin/${arch}/dbghelp.dll
        bin/${arch}/Microsoft.DTfW.DHL.manifest
    DESTINATION bin 
    COMPONENT qt_app
)
