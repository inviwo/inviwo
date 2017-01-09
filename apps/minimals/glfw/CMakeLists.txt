#--------------------------------------------------------------------
# Inviwo GLFW Application
ivw_project(inviwo_glfwminimum)

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    glfwminimum.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})
        
#--------------------------------------------------------------------
# Define libraries that should be linked
ivw_retrieve_all_modules(package_list)

set(core_dependencies InviwoCore)
list(APPEND core_dependencies GLFW)
list(APPEND core_dependencies InviwoGLFWModule)
list(APPEND package_list ${core_dependencies})

#--------------------------------------------------------------------
# Remove Qt stuff from list
foreach(module ${package_list})
    string(TOUPPER ${module} u_module)
    if(u_module MATCHES "QT+")
        list(REMOVE_ITEM package_list ${module})
    endif()
endforeach()
# Remove other GL stuff from list
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
# Determine dendendencies
set(dependencies "")
if(IVW_RUNTIME_MODULE_LOADING)
	# Application does not need to depend on all modules
	# if they are loaded at runtime.
	set(dependencies ${core_dependencies})
	add_definitions(-DIVW_RUNTIME_MODULE_LOADING)
	# Note: GLWF has not implemented FileObserving 
	# These lines can be decommented when this has been done.
	#if (IVW_RUNTIME_MODULE_RELOADING)
	#	add_definitions(-DIVW_RUNTIME_MODULE_RELOADING)
	#endif()

	# Specify which modules to load at runtime (all will be loaded if not existing)
	ivw_create_enabled_modules_file("inviwo_glfwminimum" ${package_list})
else()
	set(dependencies ${package_list})
endif()

# Need to add dependent directories before creating application
ivw_add_dependency_directories(${dependencies})

#--------------------------------------------------------------------
# Create application
add_executable(inviwo_glfwminimum MACOSX_BUNDLE WIN32 ${SOURCE_FILES})

#--------------------------------------------------------------------
# Define defintions
ivw_define_standard_definitions(inviwo_glfwminimum inviwo_glfwminimum)

#--------------------------------------------------------------------
# Define standard properties
ivw_define_standard_properties(inviwo_glfwminimum)

#--------------------------------------------------------------------
# Move to folder
ivw_folder(inviwo_glfwminimum minimals)

#--------------------------------------------------------------------
# Add application to pack
if(IVW_PACKAGE_PROJECT)
if(WIN32)
   install(TARGETS inviwo_glfwminimum
            RUNTIME DESTINATION bin
            COMPONENT glfw_app)
else()
    install(TARGETS inviwo_glfwminimum
            RUNTIME DESTINATION bin
            BUNDLE DESTINATION bin
            ARCHIVE DESTINATION lib
            LIBRARY DESTINATION lib
            COMPONENT glfw_app)
endif()
endif()

#--------------------------------------------------------------------
# Add dependencies
ivw_add_dependencies(${dependencies})

#--------------------------------------------------------------------
# Optimize compilation with pre-compilied headers based on inviwo-core
ivw_compile_optimize_inviwo_core()
