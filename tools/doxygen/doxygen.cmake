 #################################################################################
 #
 # Inviwo - Interactive Visualization Workshop
 #
 # Copyright (c) 2014-2015 Inviwo Foundation
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
 
function(make_template FILENAME DOXY_NAME BRIEF OUTPUT_DIR INPUT_LIST TAGFILE INPUT_TAG_LIST EXTRA_FILE_LIST)
	set(PROJNAME ${DOXY_NAME})
	ivw_message("Make doxygen project " ${PROJNAME})

	set(MAINPAGE "${IVW_ROOT_DIR}/README.md")

	list(APPEND INPUT_LIST ${MAINPAGE})
	string(REGEX REPLACE ";" " \\\\\n                         " INPUTS "${INPUT_LIST}")
	set(INPUTS ${INPUTS})
	
	string(REGEX REPLACE ";" " \\\\\n                         " INPUT_TAGS "${INPUT_TAG_LIST}")
	set(INPUT_TAGS ${INPUT_TAGS})

	string(REGEX REPLACE ";" " \\\\\n                         " EXTRA_FILES "${EXTRA_FILE_LIST}")
	set(EXTRA_FILES ${EXTRA_FILES})

	configure_file(${IVW_DOXY_DIR}/main.doxy.template ${FILENAME})
endfunction()
 
function(get_unique_names retval paths)
	list(LENGTH paths npaths)

	# Remove non-unique start of path
	set(ind 0)
	set(names ${paths})
	set(ret ${paths})
	list(LENGTH names n_names)
	while(n_names EQUAL npaths)
		set(ret ${names})
		set(names "")
		foreach(module ${paths})
			set(path "")
			set(i 0)
			string(REPLACE "/" ";" module_list ${module})
			foreach(dir ${module_list})
				if( i GREATER ind OR i EQUAL ind)
					list(APPEND path ${dir})
				endif()
				MATH(EXPR i "${i}+1")
			endforeach()
			string(REPLACE ";" "/" path_joined "${path}")
			list(APPEND names ${path_joined})
		endforeach()
		list(REMOVE_DUPLICATES names)
		list(LENGTH names n_names)
		MATH(EXPR ind "${ind}+1")
	endwhile()
	
	# Remove non-unique end of path
	set(ind 0)
	set(new_module_bases ${ret})
	set(names ${ret})
	list(LENGTH names n_names)
	while(n_names EQUAL npaths)
		set(ret ${names})
		set(names "")
		foreach(module ${new_module_bases})
			set(path "")
			set(i 0)
			string(REPLACE "/" ";" module_list ${module})
			list(REVERSE module_list)
			foreach(dir ${module_list})
				if( i GREATER ind OR i EQUAL ind)
					list(APPEND path ${dir})
				endif()
				MATH(EXPR i "${i}+1")
			endforeach()
			list(REVERSE path)
			string(REPLACE ";" "/" path_joined "${path}")
			list(APPEND names ${path_joined})
		endforeach()
		list(REMOVE_DUPLICATES names)
		list(LENGTH names n_names)
		MATH(EXPR ind "${ind}+1")
	endwhile()
	set(${retval} ${ret} PARENT_SCOPE)
endfunction()
 
 if(IVW_DOXYGEN_PROJECT)
	ivw_message("Generate doxygen files")

	find_package(Perl)    # sets, PERL_FOUND, PERL_EXECUTABLE
	find_package(Doxygen) # sets, DOXYGEN_FOUND, DOXYGEN_EXECUTABLE, 
						  # DOXYGEN_DOT_FOUND, DOXYGEN_DOT_EXECUTABLE
	
	set(IVW_DOXY_DIR ${IVW_ROOT_DIR}/tools/doxygen)
	set(IVW_DOXY_OUT ${CMAKE_CURRENT_BINARY_DIR}/tools/doxygen)
	set(IVW_DOXY_TAG_FILES "")

	set(IVW_DOXY_EXTRA_FILES 
		"${IVW_DOXY_DIR}/style/img_downArrow.png"
	)

	# All In one.
	make_template(
		"${IVW_DOXY_OUT}/main.doxy" 
		"Inviwo" 
		"Inviwo documentation" 
		"${IVW_DOXY_OUT}/doc/main" 
		"${IVW_SOURCE_DIR};${IVW_INCLUDE_DIR};${IVW_APPLICATION_DIR};${IVW_MODULE_DIR}"
		"" 
		""
		"${IVW_DOXY_EXTRA_FILES}"
	)
	
	add_custom_target(inviwo-documentation
		COMMAND ${CMAKE_COMMAND} -E make_directory "${IVW_DOXY_OUT}/doc/main/html"
        COMMAND ${DOXYGEN_EXECUTABLE} "${IVW_DOXY_OUT}/main.doxy"
        COMMAND ln -sf "${IVW_DOXY_OUT}/doc/main/html/index.html" "${IVW_DOXY_OUT}/doc/main/index.html"
        WORKING_DIRECTORY ${IVW_DOXY_OUT}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
	
	# Help, used for the help inside invowo
	make_template(
		"${IVW_DOXY_OUT}/help.doxy" 
		"Inviwo help" 
		"Inviwo help" 
		"${IVW_DOXY_OUT}/doc/help"  
		"${IVW_SOURCE_DIR};${IVW_INCLUDE_DIR};${IVW_APPLICATION_DIR};${IVW_MODULE_DIR}"
		"" 
		""
		"${IVW_DOXY_EXTRA_FILES}"
	)
	file(READ ${IVW_DOXY_DIR}/help.doxy.template HELP_SETTINGS)
	file(APPEND "${IVW_DOXY_OUT}/help.doxy" ${HELP_SETTINGS})
	
	# Core
	set(IVW_DOXY_TAG_CORE "${IVW_DOXY_OUT}/doc/core/html/ivwcore.tag")
	make_template(
		"${IVW_DOXY_OUT}/core.doxy" 
		"Inviwo Core" 
		"Core functionality of Inviwo" 
		"${IVW_DOXY_OUT}/doc/core/" 
		"${IVW_SOURCE_DIR}/core;${IVW_CORE_INCLUDE_DIR}" 
		"${IVW_DOXY_TAG_CORE}"
		"${IVW_DOXY_TAG_FILES}"
		"${IVW_DOXY_EXTRA_FILES}"
	)
	list(APPEND IVW_DOXY_TAG_FILES "${IVW_DOXY_TAG_CORE}=${IVW_DOXY_OUT}/doc/core/html")
	
	# OT
	set(IVW_DOXY_TAG_QT "${IVW_DOXY_OUT}/doc/qt/html/ivwqt.tag")
	list(APPEND IVW_DOXY_TAG_FILES "qtcore.tags=http://qt-project.org/doc/qt-5/")
	make_template(
		"${IVW_DOXY_OUT}/qt.doxy" 
		"Inviwo Qt" 
		"Main Qt elements of Inviwo" 
		"${IVW_DOXY_OUT}/doc/qt/" 
		"${IVW_SOURCE_DIR}/qt;${IVW_QT_INCLUDE_DIR}" 
		"${IVW_DOXY_TAG_QT}"
		"${IVW_DOXY_TAG_FILES}"
		"${IVW_DOXY_EXTRA_FILES}"
	)
	list(APPEND IVW_DOXY_TAG_FILES "${IVW_DOXY_TAG_CORE}=${IVW_DOXY_OUT}/doc/qt/html")

	# Modules
	set(IVW_DOXY_MODULE_BASES "")
	foreach(module ${IVW_MODULE_PATHS})
        get_filename_component(folder_name ${module} NAME)
		get_filename_component(path_name ${module} PATH)
		list(APPEND IVW_DOXY_MODULE_BASES ${path_name})
		list(REMOVE_DUPLICATES IVW_DOXY_MODULE_BASES)
    endforeach()
	get_unique_names(unique_names "${IVW_DOXY_MODULE_BASES}")
	
	set(index 0)
	foreach(base ${IVW_DOXY_MODULE_BASES})
		list(GET unique_names ${index} name)
			
		string(REPLACE "/" "_" file_name ${name})
		string(TOLOWER ${file_name} file_name)
		string(REPLACE "/" " " desc_name ${name})
		
		set(inc_dirs "")
		foreach(module ${IVW_MODULE_PATHS})
			get_filename_component(path_name ${module} PATH)
			if ( path_name STREQUAL base )
				list(APPEND inc_dirs ${module})
			endif()
		endforeach()
		
		set(IVW_DOXY_TAG_MODULE "${IVW_DOXY_OUT}/doc/core/html/${file_name}.tag")
		make_template(
			"${IVW_DOXY_OUT}/${file_name}.doxy" 
			"${desc_name}" 
			"Modules for ${desc_name}" 
			"${IVW_DOXY_OUT}/doc/${file_name}/" 
			"${inc_dirs}" 
			"${IVW_DOXY_TAG_MODULE}"
			"${IVW_DOXY_TAG_FILES}"
			"${IVW_DOXY_EXTRA_FILES}"
		)
		list(APPEND IVW_DOXY_TAG_FILES "${IVW_DOXY_TAG_MODULE}=${IVW_DOXY_OUT}/doc/${file_name}/html")
		MATH(EXPR index "${index}+1")
	endforeach()
	
	
	# Apps
	set(IVW_DOXY_TAG_APPS "${IVW_DOXY_OUT}/doc/core/html/ivwapps.tag")
	make_template(
		"${IVW_DOXY_OUT}/apps.doxy" 
		"Inviwo Apps" 
		"Applications using Inviwo Core and Modules" 
		"${IVW_DOXY_OUT}/doc/apps/" 
		"${IVW_SOURCE_DIR}/core;${IVW_CORE_INCLUDE_DIR}" 
		"${IVW_DOXY_TAG_APPS}"
		"${IVW_DOXY_TAG_FILES}"
		"${IVW_DOXY_EXTRA_FILES}"
	)
	list(APPEND IVW_DOXY_TAG_FILES "${IVW_DOXY_TAG_APPS}={IVW_DOXY_OUT}/doc/apps/html")
	
 endif()