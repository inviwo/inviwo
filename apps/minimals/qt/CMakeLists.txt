#--------------------------------------------------------------------
# Inviwo Qt Application
ivw_project(inviwo_qtminimum)

if(POLICY CMP0043)
    cmake_policy(SET CMP0043 OLD) # Ignore compile definitions.
endif()

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    qtminimum.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})
        
#--------------------------------------------------------------------
# Define defintions
ivw_define_standard_definitions(inviwo_qtminimum)

#--------------------------------------------------------------------
# Define libraries that should be linked
ivw_retrieve_all_modules(package_list)
list(APPEND package_list InviwoCore)

#--------------------------------------------------------------------
# Remove Qt stuff from list
foreach(module ${package_list})
    string(TOUPPER ${module} u_module)
    if(u_module MATCHES "QT+")
        list(REMOVE_ITEM package_list ${module})
    endif()
endforeach()

#--------------------------------------------------------------------
# Add Inviwo OpenGL Qt
list(APPEND package_list InviwoOpenGLQtModule)

#--------------------------------------------------------------------
# Only include one OpenGL context creation module
list(REMOVE_ITEM package_list InviwoGLFWModule)
list(REMOVE_ITEM package_list InviwoGLUTModule)
list(REMOVE_ITEM package_list InviwoSDLModule)

#--------------------------------------------------------------------
# Remove modules not used by this app
list(REMOVE_ITEM package_list InviwoPythonModule)
list(REMOVE_ITEM package_list InviwoUnitTestsModule)

#--------------------------------------------------------------------
# Register the use of modules
ivw_register_use_of_modules(${package_list})

#--------------------------------------------------------------------
# Need to add dependent directories before creating application
ivw_add_dependency_directories(${package_list})

#--------------------------------------------------------------------
# Create application
add_executable(inviwo_qtminimum MACOSX_BUNDLE WIN32 ${SOURCE_FILES})

#--------------------------------------------------------------------
# Move to folder
ivw_vs_folder(inviwo_qtminimum minimals)

#--------------------------------------------------------------------
# Add application to pack
if(IVW_PACKAGE_PROJECT)
if(WIN32)
   install(TARGETS inviwo_qtminimum
            RUNTIME DESTINATION bin
            COMPONENT qt_app)
else()
    install(TARGETS inviwo_qtminimum
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

#--------------------------------------------------------------------
# Set VS Executable properties
#ivw_vs_executable_setup(qtminimum)
