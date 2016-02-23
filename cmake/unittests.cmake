 #################################################################################
 #
 # Inviwo - Interactive Visualization Workshop
 #
 # Copyright (c) 2013-2015 Inviwo Foundation
 # All rights reserved.
 # 
 # Redistribution and use in source and binary forms, with or without
 # modification, are permitted provided that the following conditions are met: 
 # 
 # 1. Redistributions of source code must retain the above copyright notice, this
 # list of conditions and the following disclaimer. 
 # 2. Redistributions in binary form must reproduce the above copyright notice,
 # this list of conditions and the following disclaimer in the documentation
 # and/or other materials provided with the distribution. 
 # 
 # THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 # ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 # WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 # DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 # ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 # (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 # LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 # ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 # (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 # SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 # 
 #################################################################################

 ### Generate unittests for modules. ###

#--------------------------------------------------------------------
# Options for unittests
option(IVW_UNITTESTS "Enable unittests" ON)
option(IVW_UNITTESTS_RUN_ON_BUILD "Enable running unittest when building" ON)

#--------------------------------------------------------------------
# Add unittests
function(ivw_add_unittest)
    if(IVW_UNITTESTS)
        foreach(item ${ARGN})
            set(unittest_files ${unittest_files};${item} CACHE INTERNAL "Unit test files")
        endforeach()
        list(REMOVE_DUPLICATES unittest_files)
        set(unittest_files ${unittest_files} CACHE INTERNAL "Unit test files")

        set(${_projectName}_UNITTEST_FILES "${ARGN}" PARENT_SCOPE)
    endif()
endfunction()

#--------------------------------------------------------------------
# Inviwo Unittest Application
function(ivw_make_unittest_target name dependencies)
	# Check if there are any tests
	if(NOT IVW_UNITTESTS OR NOT ${name}_UNITTEST_FILES)
		return()
	endif()

	set(test_name "inviwo-unittests-${name}")
	ivw_debug_message(STATUS "create unittests: ${test_name}")

	ivw_retrieve_all_modules(available_modules)
	ivw_mod_name_to_dir(module_dir_names ${available_modules})
	list(FIND module_dir_names ${name} index)
	if(NOT index EQUAL -1)
		list(GET available_modules ${index} module_name)
	endif()
	set(test_dependencies gtest InviwoCore ${module_name} ${dependencies})
	
	ivw_project(${test_name})

	add_definitions(-DGTEST_HAS_TR1_TUPLE=0)
	if(BUILD_SHARED_LIBS)
    	add_definitions(-DGTEST_LINKED_AS_SHARED_LIBRARY=1)
	endif()

	#--------------------------------------------------------------------
	# Add source files
	set(SOURCE_FILES ${${name}_UNITTEST_FILES} )
	ivw_group("Test Files" ${SOURCE_FILES})

	#--------------------------------------------------------------------
	# Need to add dependent directories before creating application
	ivw_add_dependency_directories(${test_dependencies})

	#--------------------------------------------------------------------
	# Register the use of inviwo modules
	list_intersection(use_inivwo_modules "${test_dependencies}" "${available_modules}")
	ivw_register_use_of_modules(${use_inivwo_modules})

	#--------------------------------------------------------------------
	# Include generation directory to register modules etc
	include_directories(${CMAKE_BINARY_DIR}/modules/_generated)
	include_directories(${CMAKE_SOURCE_DIR}/ext/gtest/include)
	ivw_link_directories(${LIBRARY_OUTPUT_PATH})

	#--------------------------------------------------------------------
	# Create application
	add_executable(${test_name} MACOSX_BUNDLE WIN32 ${SOURCE_FILES})

	#--------------------------------------------------------------------
	# Define defintions
	ivw_define_standard_definitions(${test_name} ${test_name})
	#--------------------------------------------------------------------
	# Define standard properties
	ivw_define_standard_properties(${test_name})

	#--------------------------------------------------------------------
	# Add dependencies
	ivw_add_dependency_libraries(${_preModuleDependencies})
	ivw_add_dependencies(${test_dependencies})

	#--------------------------------------------------------------------
	# Move to folder
	ivw_folder(${test_name} unittests)

	#--------------------------------------------------------------------
	# Optimize compilation with pre-compilied headers based on inviwo-core
	# ivw_compile_optimize_inviwo_core()

	if(IVW_UNITTESTS_RUN_ON_BUILD)
		add_custom_command(TARGET "${test_name}" POST_BUILD COMMAND "${test_name}" 
			               COMMENT "Running unittest for: ${test_name}")
	endif()

	ivw_memleak_setup(${test_name})

endfunction()