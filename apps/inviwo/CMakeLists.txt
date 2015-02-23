#--------------------------------------------------------------------
# Inviwo Qt Application
ivw_project(inviwo)

if(POLICY CMP0043)
    cmake_policy(SET CMP0043 OLD) # Ignore compile definitions.
endif()

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${IVW_APPLICATION_DIR}/inviwo/inviwomainwindow.h
    ${IVW_APPLICATION_DIR}/inviwo/inviwosplashscreen.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add header files
set(MOC_FILES
     ${IVW_APPLICATION_DIR}/inviwo/inviwomainwindow.h
     ${IVW_APPLICATION_DIR}/inviwo/inviwosplashscreen.h
)

if(DESIRED_QT_VERSION MATCHES 5)
    qt5_wrap_cpp(MOC_FILES ${MOC_FILES})    
else()
    qt4_wrap_cpp(MOC_FILES ${MOC_FILES})
endif()

source_group("MOC Files" FILES ${MOC_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${IVW_APPLICATION_DIR}/inviwo/inviwo.cpp
    ${IVW_APPLICATION_DIR}/inviwo/inviwomainwindow.cpp
    ${IVW_APPLICATION_DIR}/inviwo/inviwosplashscreen.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

source_group("Resource Files" FILES ${IVW_RESOURCES_DIR}/inviwo.qrc)

#--------------------------------------------------------------------
# Add resource file
if(DESIRED_QT_VERSION MATCHES 5)
    qt5_add_resources(QRC_FILE "${IVW_RESOURCES_DIR}/inviwo.qrc")
else()
    qt4_add_resources(QRC_FILE "${IVW_RESOURCES_DIR}/inviwo.qrc")
endif()
        
#--------------------------------------------------------------------
# Define defintions
ivw_define_standard_definitions(inviwo)

#--------------------------------------------------------------------
# Define libraries that should be linked
ivw_retrieve_all_modules(package_list)
list(APPEND package_list InviwoQtEditor)
list(APPEND package_list InviwoCore)
# Only include one OpenGL context creation module
if(IVW_USE_GLFW_NOT_OPENGLQT)
list(REMOVE_ITEM package_list InviwoOpenGLQtModule)
else()
list(REMOVE_ITEM package_list InviwoGLFWModule)
endif()

#--------------------------------------------------------------------
# Remove modules not used by this app
list(REMOVE_ITEM package_list InviwoGLUTModule)
list(REMOVE_ITEM package_list InviwoSDLModule)

if(IVW_MODULE_UNITTESTS)
	option(IVW_RUN_UNITTEST_ON_STARTUP "Run all registered unittest when Inviwo starts" OFF)
	if(IVW_RUN_UNITTEST_ON_STARTUP)
		add_definitions(-DIVW_RUN_UNITTEST_ON_STARTUP)
    else()
        list(REMOVE_ITEM package_list InviwoUnitTestsModule)
	endif()
endif()

#--------------------------------------------------------------------
# Register the use of modules
ivw_register_use_of_modules(${package_list})

#--------------------------------------------------------------------
# Need to add dependent directories before creating application
ivw_add_dependency_directories(${package_list})

set(RES_FILES "")
if(WIN32)
    set(RES_FILES "${IVW_RESOURCES_DIR}/inviwo.rc")
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(MACOSX_BUNDLE_BUNDLE_NAME Inviwo)
  set(MACOSX_BUNDLE_GUI_IDENTIFIER "Inviwo")
  set(MACOSX_BUNDLE_ICON_FILE ${IVW_ROOT_DIR}/resources/icons/inviwo_light.icns)
  set(MACOSX_BUNDLE_INFO_STRING "Inviwo ${IVW_VERSION}, Copyright (c) 2012-2015 Inviwo Foundation")
  set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${IVW_VERSION}")
  set(MACOSX_BUNDLE_LONG_VERSION_STRING "${IVW_VERSION}.")
  set(MACOSX_BUNDLE_BUNDLE_VERSION "${IVW_VERSION}")
  set(MACOSX_BUNDLE_COPYRIGHT "(C) 2012-2015 Inviwo Foundation")
endif()

#--------------------------------------------------------------------
# Create application
add_executable(inviwo MACOSX_BUNDLE WIN32 ${SOURCE_FILES} ${HEADER_FILES} ${MOC_FILES} ${QRC_FILE} ${RES_FILES})

#--------------------------------------------------------------------
# Define standard properties
ivw_define_standard_properties(inviwo)

#--------------------------------------------------------------------
# Add application to pack
if(IVW_PACKAGE_PROJECT)
if(WIN32)
   install(TARGETS inviwo
            RUNTIME DESTINATION bin
            COMPONENT qt_app)
else()
    install(TARGETS inviwo
            RUNTIME DESTINATION bin
            BUNDLE DESTINATION bin
	        ARCHIVE DESTINATION lib
            LIBRARY DESTINATION lib
            COMPONENT qt_app)
endif()
endif()

#--------------------------------------------------------------------
# Add dependencies
ivw_add_dependencies(${package_list})

#--------------------------------------------------------------------
# Optimize compilation with pre-compilied headers based on inviwo-core
ivw_compile_optimize_inviwo_core()

if(IVW_ENABLE_MSVC_MEMLEAK_TEST)
	IF(WIN32)
		IF(MSVC)
			if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
				add_custom_command(TARGET inviwo POST_BUILD 
									COMMAND ${CMAKE_COMMAND}  -E copy_if_different 
										${IVW_EXTENSIONS_DIR}/vld/bin/Win64/vld_x64.dll 
										${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$(ConfigurationName)/vld_x64.dll)	
										
				add_custom_command(TARGET inviwo POST_BUILD 
									COMMAND ${CMAKE_COMMAND}  -E copy_if_different 
										${IVW_EXTENSIONS_DIR}/vld/bin/Win64/dbghelp.dll 
										${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$(ConfigurationName)/dbghelp.dll)	
										
				add_custom_command(TARGET inviwo POST_BUILD 
									COMMAND ${CMAKE_COMMAND}  -E copy_if_different 
										${IVW_EXTENSIONS_DIR}/vld/bin/Win64/Microsoft.DTfW.DHL.manifest 
										${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$(ConfigurationName)/Microsoft.DTfW.DHL.manifest)	
			else ()
				add_custom_command(TARGET inviwo POST_BUILD 
									COMMAND ${CMAKE_COMMAND}  -E copy_if_different 
										${IVW_EXTENSIONS_DIR}/vld/bin/Win32/vld_x86.dll 
										${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$(ConfigurationName)/vld_x86.dll)	
										
				add_custom_command(TARGET inviwo POST_BUILD 
									COMMAND ${CMAKE_COMMAND}  -E copy_if_different 
										${IVW_EXTENSIONS_DIR}/vld/bin/Win32/dbghelp.dll 
										${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$(ConfigurationName)/dbghelp.dll)	
										
				add_custom_command(TARGET inviwo POST_BUILD 
									COMMAND ${CMAKE_COMMAND}  -E copy_if_different 
										${IVW_EXTENSIONS_DIR}/vld/bin/Win32/Microsoft.DTfW.DHL.manifest 
										${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$(ConfigurationName)/Microsoft.DTfW.DHL.manifest)	
			endif()
		ENDIF(MSVC)
	ENDIF(WIN32)
endif()
