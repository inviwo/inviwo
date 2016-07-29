 #################################################################################
 #
 # Inviwo - Interactive Visualization Workshop
 #
 # Copyright (c) 2014-2016 Inviwo Foundation
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
 
if(${MSVC})
    option(IVW_DOXYGEN_PROJECT "Create Inviwo doxygen files" OFF)
    if(${IVW_DOXYGEN_PROJECT})
        ivw_message(STATUS 
            "In Visual Studio the doxygen project will rerun every time you run\n"
            "\"ALL_BUILD\" even if it is up to date. Hence, you propbably only\n"
            "want to enable \"IVW_DOXYGEN_PROJECT\" once to generate the\n"
            "documentation and then then disable it.")
    endif()
else()
    option(IVW_DOXYGEN_PROJECT "Create Inviwo doxygen files" ON)
endif()

function(ivw_format_doxy_arg retval )
    string(REGEX REPLACE ";" " \\\\\n                         " result "${ARGN}")
    set(${retval} ${result} PARENT_SCOPE)
endfunction()    

function(ivw_make_documentation outdir doxy_name BRIEF input_list TAGFILE input_tag_list 
         extra_file_list, image_path_list aliases_list aditional_flags_list)
    string(TOLOWER ${doxy_name} name_lower)

    set(PROJNAME ${doxy_name})
    set(MAINPAGE "${IVW_ROOT_DIR}/README.md")
    set(OUTPUT_DIR "${outdir}/${name_lower}")

    list(APPEND input_list ${MAINPAGE})
    ivw_format_doxy_arg(INPUTS ${input_list})
    ivw_format_doxy_arg(INPUT_TAGS ${input_tag_list})
    ivw_format_doxy_arg(EXTRA_FILES ${extra_file_list})
    ivw_format_doxy_arg(IMAGE_PATH ${image_path_list})
    ivw_format_doxy_arg(ALIASES ${aliases_list})
    string(REGEX REPLACE ";" " \n " ADITIONAL_FLAGS "${aditional_flags_list}")
    set(ADITIONAL_FLAGS ${ADITIONAL_FLAGS}) # ADITIONAL_FLAGS is appended to Doxygen.in

    configure_file(${ivw_doxy_dir}/Doxygen.in ${outdir}/${name_lower}.doxy)

    add_custom_target("DOXY-${doxy_name}"
        COMMAND ${CMAKE_COMMAND} -E echo "Building doxygen ${doxy_name}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${outdir}/${name_lower}/html"
        COMMAND ${DOXYGEN_EXECUTABLE} "${outdir}/${name_lower}.doxy"
        COMMAND ${CMAKE_COMMAND} -DDEST_PATH="${outdir}/${name_lower}/html" -P "${outdir}/copy_img.cmake"
        WORKING_DIRECTORY ${outdir}
        COMMENT "Generating ${doxy_name} API documentation with Doxygen"
        VERBATIM
    )
    set_target_properties("DOXY-${doxy_name}" PROPERTIES FOLDER "doc" EXCLUDE_FROM_ALL TRUE)
endfunction()

function(ivw_make_help INPUT_DIR SOURCE_LIST image_path_list)
    # Help, used for the help inside invowo
    set(GENERATE_QHP "YES")
    set(GENERATE_IMG "NO")
    set(HTML_LAYOUT_FILE "${INPUT_DIR}/layout.xml")
    set(HTML_STYLESHEET "${INPUT_DIR}/stylesheet.css")
    set(HTML_HEADER "${INPUT_DIR}/header.html")
    set(HTML_FOOTER "${INPUT_DIR}/footer.html")
    set(aliases_list
        "docpage{1}=\"\\page docpage-\\1 \\1\""
        "docpage{2}=\"\\page docpage-\\1 \\2\""
    )

    set(aditional_flags_list 
        "AUTOLINK_SUPPORT       = NO"
        "HIDE_SCOPE_NAMES       = YES"
        "SHOW_INCLUDE_FILES     = NO"
        "GENERATE_TODOLIST      = NO"
        "GENERATE_TESTLIST      = NO"
        "GENERATE_BUGLIST       = NO"
        "GENERATE_DEPRECATEDLIST= NO"
        "SHOW_USED_FILES        = NO"
        "SHOW_FILES             = NO"
        "SHOW_NAMESPACES        = NO"
        "WARN_IF_UNDOCUMENTED   = NO"
        "SOURCE_BROWSER         = NO"
        "ALPHABETICAL_INDEX     = NO"
        "HTML_DYNAMIC_SECTIONS  = NO"
        "DISABLE_INDEX          = YES"
        "SEARCHENGINE           = NO"
        "GENERATE_AUTOGEN_DEF   = NO"
        "CLASS_DIAGRAMS         = NO"
    )

    ivw_make_documentation(
        "${ivw_doxy_out}" 
        "Help" 
        "Inviwo help"  
        "${SOURCE_LIST}"
        "" 
        ""
        "${extra_files}"
        "${image_path_list}"
        "${aliases_list}"
        "${aditional_flags_list}"
    )

    get_filename_component(QT_BIN_PATH ${QT_QMAKE_EXECUTABLE} PATH)
    find_program(IVW_DOXY_QCOLLECTIONGENERATOR "qcollectiongenerator" ${QT_BIN_PATH})
    find_program(IVW_DOXY_QHELPGENERATOR "qhelpgenerator" ${QT_BIN_PATH})

    set(INV_QHCP 
        "<?xml version=\"1.0\" encoding=\"utf-8\" ?>"
        "<QHelpCollectionProject version=\"1.0\">"
        "    <docFiles>"
        "        <register>"
        "            <file>inviwo.qch</file>"
        "        </register>"
        "    </docFiles>"
        "</QHelpCollectionProject>"
    )
    string(REPLACE ";" "\n" INV_QHCP ${INV_QHCP})
    file(WRITE "${ivw_doxy_out}/help/inviwo.qhcp" ${INV_QHCP})

    add_custom_target("DOXY-QCH"
        COMMAND ${CMAKE_COMMAND} -E echo "Clean unused files"
        COMMAND ${CMAKE_COMMAND} -P "${ivw_doxy_out}/clean_help.cmake"
        COMMAND ${CMAKE_COMMAND} -E echo "Building inviwo.qch"
        COMMAND ${CMAKE_COMMAND} -E echo "Running: ${IVW_DOXY_QHELPGENERATOR} -o ${ivw_doxy_out}/help/inviwo.qch ${ivw_doxy_out}/help/html/index.qhp"
        COMMAND ${IVW_DOXY_QHELPGENERATOR} "-o" "${ivw_doxy_out}/help/inviwo.qch" "${ivw_doxy_out}/help/html/index.qhp"
        COMMAND ${CMAKE_COMMAND} -E echo "Building inviwo.qhc"
        COMMAND ${CMAKE_COMMAND} -E echo "Running: ${IVW_DOXY_QCOLLECTIONGENERATOR} -o ${ivw_doxy_out}/help/inviwo.qhc ${ivw_doxy_out}/help/inviwo.qhcp"
        COMMAND ${IVW_DOXY_QCOLLECTIONGENERATOR} "-o" "${ivw_doxy_out}/help/inviwo.qhc" "${ivw_doxy_out}/help/inviwo.qhcp"
        COMMAND ${CMAKE_COMMAND} -E echo "Copy ${ivw_doxy_out}/help/inviwo.q* to ${IVW_ROOT_DIR}/data/help/"
        COMMAND ${CMAKE_COMMAND} -E copy "${ivw_doxy_out}/help/inviwo.qch" "${IVW_ROOT_DIR}/data/help/"
        COMMAND ${CMAKE_COMMAND} -E copy "${ivw_doxy_out}/help/inviwo.qhc" "${IVW_ROOT_DIR}/data/help/"
        COMMAND ${CMAKE_COMMAND} -E copy "${ivw_doxy_out}/help/inviwo.qhcp" "${IVW_ROOT_DIR}/data/help/"

        WORKING_DIRECTORY ${OUTPUT_DIR}
        COMMENT "Generating QCH files"
        VERBATIM
    )
    add_dependencies("DOXY-QCH" "DOXY-Help")
    set_target_properties("DOXY-QCH" PROPERTIES FOLDER "doc" EXCLUDE_FROM_ALL TRUE)
endfunction()

function(make_doxygen_target modules_var)
    if(NOT ${IVW_DOXYGEN_PROJECT})
        return()
    endif()

    find_package(Perl QUIET)            # sets: PERL_FOUND, PERL_EXECUTABLE
    find_package(Doxygen QUIET)         # sets: DOXYGEN_FOUND, DOXYGEN_EXECUTABLE, 
                                        # DOXYGEN_DOT_FOUND, DOXYGEN_DOT_EXECUTABLE    
    find_package(PythonInterp QUIET)    # sets: PYTHONINTERP_FOUND PYTHON_EXECUTABLE

    if(NOT ${DOXYGEN_FOUND})
        ivw_message(WARNING "Tried to create doxygen project, but doxygen was not found")
        return()
    endif()

    if(${DOXYGEN_DOT_FOUND})
        get_filename_component(DOXYGEN_DOT_PATH ${DOXYGEN_DOT_EXECUTABLE} PATH)
    endif()

    set(ivw_doxy_dir ${IVW_ROOT_DIR}/tools/doxygen)
    set(ivw_doxy_out ${CMAKE_CURRENT_BINARY_DIR}/doc)
    set(tag_files "")
    set(dependency_list "")

    set(GENERATE_IMG "YES")
    ivw_format_doxy_arg(SHADER_INC_PATH ${IVW_SHADER_INCLUDE_PATHS})
    
    if(PYTHONINTERP_FOUND)
        set(PREFIX_PYTHON "")
        #set(PREFIX_PYTHON "${PYTHON_EXECUTABLE} ") # This is sometimes needed but gives errors on win7
        set(filer_patterns_list
            "\"*.frag=${PREFIX_PYTHON}${ivw_doxy_dir}/filter/glslfilter.py\""
            "\"*.vert=${PREFIX_PYTHON}${ivw_doxy_dir}/filter/glslfilter.py\""
            "\"*.geom=${PREFIX_PYTHON}${ivw_doxy_dir}/filter/glslfilter.py\""
            "\"*.glsl=${PREFIX_PYTHON}${ivw_doxy_dir}/filter/glslfilter.py\""
         )
        ivw_format_doxy_arg(FILER_PATTERNS ${filer_patterns_list})
    endif()

    set(extra_files "${ivw_doxy_dir}/style/img_downArrow.png")

    set(aliases_list
        "docpage{1}=\"\\ingroup processors \\n \#\\1\""
        "docpage{2}=\"\\ingroup processors \\n \#\\2\""
    )

    set(image_path_list "${IVW_ROOT_DIR}/data/help/images")

    set(all_sources 
        "${IVW_INCLUDE_DIR}"
        "${IVW_SOURCE_DIR}"
        "${IVW_APPLICATION_DIR}"
    )

    if(${MSVC})
        set(WARN_FORMAT "\$file(\$line): \$text")
    endif()

    # Group target
    add_custom_target("DOXY-ALL"
        WORKING_DIRECTORY ${ivw_doxy_out}
        COMMENT "Generating ALL API documentation with Doxygen"
        VERBATIM
    )
    set_target_properties("DOXY-ALL" PROPERTIES FOLDER "doc" EXCLUDE_FROM_ALL TRUE)

    # Core
    set(core_tag "${ivw_doxy_out}/core/ivwcore.tag")
    ivw_make_documentation(
        "${ivw_doxy_out}" 
        "Core" 
        "Core functionality of Inviwo" 
        "${IVW_CORE_INCLUDE_DIR};${IVW_CORE_SOURCE_DIR}" 
        "${core_tag}"
        "${tag_files}"
        "${extra_files}"
        "${image_path_list}"
        "${aliases_list}"
        ""
    )
    list(APPEND dependency_list "DOXY-Core")
    list(APPEND tag_files "${core_tag}=${ivw_doxy_out}/core/html")
    
    # Ot
    set(qt_tag "${ivw_doxy_out}/qt/ivwqt.tag")
    #list(APPEND tag_files "qtcore.tags=http://qt-project.org/doc/qt-5/")
    ivw_make_documentation(
        "${ivw_doxy_out}" 
        "Qt" 
        "Main Qt elements of Inviwo" 
        "${IVW_QT_INCLUDE_DIR};${IVW_QT_SOURCE_DIR}" 
        "${qt_tag}"
        "${tag_files}"
        "${extra_files}"
        "${image_path_list}"
        "${aliases_list}"
        ""
    )
    foreach(depends "${dependency_list}")
        add_dependencies("DOXY-Qt" ${depends})
    endforeach()
    list(APPEND dependency_list "DOXY-Qt")
    list(APPEND tag_files "${qt_tag}=${ivw_doxy_out}/qt/html")

    # Modules
    set(module_bases "")
    foreach(mod ${${modules_var}})
        list(APPEND module_bases ${${mod}_base})
        list(REMOVE_DUPLICATES module_bases)
        if(EXISTS "${${mod}_path}/docs/images")
            list(APPEND image_path_list "${${mod}_path}/docs/images")
        endif()
        list(APPEND all_sources ${${mod}_path})
    endforeach()
    ivw_find_unique_path_segements(unique_names "${module_bases}")
    
    set(index 0)
    foreach(base ${module_bases})
        list(GET unique_names ${index} name)
            
        string(REPLACE "/" "-" desc_name ${name})
        string(TOLOWER ${desc_name} desc_name_lower)
        
        set(inc_dirs "")
        foreach(mod ${${modules_var}})
            if(${${mod}_base} STREQUAL ${base})
                list(APPEND inc_dirs ${${mod}_path})
            endif()
        endforeach()
        
        set(module_tag "${ivw_doxy_out}/${desc_name_lower}/${desc_name_lower}.tag")
        ivw_make_documentation(
            "${ivw_doxy_out}" 
            "${desc_name}" 
            "Modules for ${desc_name}"  
            "${inc_dirs}" 
            "${module_tag}"
            "${tag_files}"
            "${extra_files}"
            "${image_path_list}"
            "${aliases_list}"
            ""
        )
        foreach(depends "${dependency_list}")
            add_dependencies("DOXY-${desc_name}" ${depends})
        endforeach()
        list(APPEND dependency_list "DOXY-${desc_name}")
        list(APPEND tag_files "${module_tag}=${ivw_doxy_out}/${desc_name_lower}/html")
        MATH(EXPR index "${index}+1")
    endforeach()
        
    # Apps
    set(app_tag "${ivw_doxy_out}/apps/ivwapps.tag")
    ivw_make_documentation(
        "${ivw_doxy_out}" 
        "Apps" 
        "Applications using Inviwo Core and Modules" 
        "${IVW_APPLICATION_DIR}" 
        "${app_tag}"
        "${tag_files}"
        "${extra_files}"
        "${image_path_list}"
        "${aliases_list}"
        ""
    )
    foreach(depends "${dependency_list}")
        add_dependencies("DOXY-Apps" ${depends})
    endforeach()
    list(APPEND dependency_list "DOXY-Apps")
    list(APPEND tag_files "${app_tag}={ivw_doxy_out}/apps/html")

    foreach(depends "${dependency_list}")
        add_dependencies("DOXY-ALL" ${depends})
    endforeach()


    # All In one.
    ivw_make_documentation(
        "${ivw_doxy_out}" 
        "Inviwo" 
        "Inviwo documentation" 
        "${all_sources}"
        "" 
        ""
        "${extra_files}"
        "${image_path_list}"
        "${aliases_list}"
        ""
    )
    add_dependencies("DOXY-ALL" "DOXY-Inviwo")


    # Help, used for the help inside invowo
    ivw_make_help("${ivw_doxy_dir}/help" "${all_sources}" "${image_path_list}")
    add_dependencies("DOXY-ALL" "DOXY-Help")
    add_dependencies("DOXY-ALL" "DOXY-QCH")

    # make a img-copy script
    set(COPY_SCRIPT_LIST
        "foreach(path ${image_path_list})"
        "    string(REPLACE \"\\\"\" \"\" new_path \${path})"
        "    string(REPLACE \"\\\"\" \"\" new_dest \${DEST_PATH})"
        "    message(\"Look for images in: \" \${new_path})"
        "    file(GLOB imgs \${new_path}/*)"
        "    foreach(img \${imgs})"
        "        message(\"Copy \${img} to \${new_dest}\")"
        "        file(COPY \${img} DESTINATION \${new_dest})"
        "    endforeach()"
        "endforeach()"
    )
    string(REPLACE ";" "\n" COPY_SCRIPT "${COPY_SCRIPT_LIST}")
    file(WRITE "${ivw_doxy_out}/copy_img.cmake" ${COPY_SCRIPT})

    # make a clean help script
    set(CLEAN_HELP_SCRIPT_LIST
        "file(GLOB save \"${ivw_doxy_out}/help/html/docpage*.html\")"
        "file(GLOB todelete \"${ivw_doxy_out}/help/html/*.js\" \"${ivw_doxy_out}/help/html/*.html\")"
        "list(REMOVE_ITEM todelete \${save})"
        "foreach(item \${todelete})"
        "    message(\"Remove: \${item}\")"
        "    file(REMOVE \${item})"
        "endforeach()"
    )
    string(REPLACE ";" "\n" CLEAN_HELP_SCRIPT "${CLEAN_HELP_SCRIPT_LIST}")
    file(WRITE "${ivw_doxy_out}/clean_help.cmake" ${CLEAN_HELP_SCRIPT})

 endfunction()