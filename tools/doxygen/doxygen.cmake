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
 
function(make_template FILENAME DOXY_NAME BRIEF OUTPUT_DIR INPUT_LIST TAGFILE INPUT_TAG_LIST)
	set(PROJNAME ${DOXY_NAME})
	ivw_message("Make doxygen project " ${PROJNAME})
	
	string(REGEX REPLACE ";" " \\\\\n                         " INPUTS "${INPUT_LIST}")
	set(INPUTS ${INPUTS})
	
	string(REGEX REPLACE ";" " \\\\\n                         " INPUT_TAGS "${INPUT_TAG_LIST}")
	set(INPUT_TAGS ${INPUT_TAGS})

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
	
	set(IVW_DOXY_OUT ${IVW_ROOT_DIR}/tools/doxygen/doc)
	
	# Main
	make_template(
		"${IVW_DOXY_DIR}/main.doxy" 
		"Inviwo" 
		"Inviwo documentation" 
		"doc/" 
		"${IVW_SOURCE_DIR};${IVW_INCLUDE_DIR};${IVW_APPLICATION_DIR};${IVW_MODULE_DIR}"
		"" 
		""
	)
	
	# Help
	make_template(
		"${IVW_DOXY_DIR}/help.doxy" 
		"Inviwo help" 
		"Inviwo help" 
		"${IVW_DOXY_OUT}" 
		"${IVW_SOURCE_DIR};${IVW_INCLUDE_DIR};${IVW_APPLICATION_DIR};${IVW_MODULE_DIR}"
		"" 
		""
	)
	file(READ ${IVW_DOXY_DIR}/help.doxy.template HELP_SETTINGS)
	file(APPEND ${IVW_DOXY_DIR}/help.doxy ${HELP_SETTINGS})
	
	# Core
	make_template(
		"${IVW_DOXY_DIR}/core.doxy" 
		"Inviwo Core" 
		"Core functionality of Inviwo" 
		"${IVW_DOXY_OUT}/core" 
		"${IVW_SOURCE_DIR}/core;${IVW_CORE_INCLUDE_DIR}" 
		"${IVW_DOXY_OUT}/core/html/ivwcore.tag"
		""
	)
	
	# OT
	make_template(
		"${IVW_DOXY_DIR}/qt.doxy" 
		"Inviwo Qt" 
		"Main Qt elements of Inviwo" 
		"${IVW_DOXY_OUT}/qt" 
		"${IVW_SOURCE_DIR}/qt;${IVW_QT_INCLUDE_DIR}" 
		"${IVW_DOXY_OUT}/qt/html/ivwqt.tag"
		"${IVW_DOXY_OUT}/core/html/ivwcore.tag=../../core/html;qtcore.tags=http://qt-project.org/doc/qt-5/"
	)
	
	# Apps
	set(IVW_DOXY_APP_TAGS
		"${IVW_DOXY_OUT}/qt/html/ivwqt.tag=../../qt/html"
		"${IVW_DOXY_OUT}/core/html/ivwcore.tag=../../core/html"
		"${IVW_DOXY_OUT}/modules/html/ivwmodules.tag=../../modules/html"
		"qtcore.tags=http://qt-project.org/doc/qt-5/"
	)
	
	make_template(
		"${IVW_DOXY_DIR}/apps.doxy" 
		"Inviwo Apps" 
		"Applications using Inviwo Core and Modules" 
		"${IVW_DOXY_OUT}/apps" 
		"${IVW_SOURCE_DIR}/core;${IVW_CORE_INCLUDE_DIR}" 
		"${IVW_DOXY_OUT}/core/html/ivwapps.tag"
		"${IVW_DOXY_APP_TAGS}"
	)

	set(IVW_DOXY_MODULE_BASES "")
	foreach(module ${IVW_MODULE_PATHS})
        get_filename_component(folder_name ${module} NAME)
		get_filename_component(path_name ${module} PATH)
		list(APPEND IVW_DOXY_MODULE_BASES ${path_name})
		list(REMOVE_DUPLICATES IVW_DOXY_MODULE_BASES)
    endforeach()
	

	get_unique_names(unique_names "${IVW_DOXY_MODULE_BASES}")
	ivw_message("unique_names " ${unique_names})
	
	set(index 0)
	foreach(base ${IVW_DOXY_MODULE_BASES})
		list(GET unique_names ${index} name)
		
		ivw_message("Name " ${index} ": " ${name})
		ivw_message("Base " ${index} ": " ${base})
		
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
		
		make_template(
			"${IVW_DOXY_DIR}/${file_name}.doxy" 
			"${desc_name}" 
			"Modules for ${desc_name}" 
			"${IVW_DOXY_OUT}/${file_name}" 
			"${inc_dirs}" 
			"${IVW_DOXY_OUT}/core/html/${file_name}.tag"
			""
		)
		MATH(EXPR index "${index}+1")
	endforeach()
	
	
 endif()