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

function(make_template FILENAME DOXY_NAME BRIEF OUTPUT_DIR INPUT_LIST TAGFILE INPUT_TAG_LIST
                       EXTRA_FILE_LIST image_path_list aliases_list)

    set(PROJNAME ${DOXY_NAME})

    set(MAINPAGE "${IVW_ROOT_DIR}/README.md")

    list(APPEND INPUT_LIST ${MAINPAGE})
    string(REGEX REPLACE ";" " \\\\\n                         " INPUTS "${INPUT_LIST}")
    set(INPUTS ${INPUTS})
    
    string(REGEX REPLACE ";" " \\\\\n                         " INPUT_TAGS "${INPUT_TAG_LIST}")
    set(INPUT_TAGS ${INPUT_TAGS})

    string(REGEX REPLACE ";" " \\\\\n                         " EXTRA_FILES "${EXTRA_FILE_LIST}")
    set(EXTRA_FILES ${EXTRA_FILES})

    string(REGEX REPLACE ";" " \\\\\n                         " IMAGE_PATH "${image_path_list}")
    set(IMAGE_PATH ${IMAGE_PATH})

    string(REGEX REPLACE ";" " \\\\\n                         " ALIASES "${aliases_list}")
    set(ALIASES ${ALIASES})

    configure_file(${ivw_doxy_dir}/Doxygen.in ${FILENAME})
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

function(make_doxy_target OUTPUT_DIR DOXY_NAME image_path_list)
    string(TOLOWER ${DOXY_NAME} name_lower)
    add_custom_target("DOXY-${DOXY_NAME}"
        COMMAND ${CMAKE_COMMAND} -E echo "Building doxygen ${DOXY_NAME}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${OUTPUT_DIR}/doc/${name_lower}/html"
        COMMAND ${DOXYGEN_EXECUTABLE} "${OUTPUT_DIR}/${name_lower}.doxy"
        COMMAND ${CMAKE_COMMAND} -DDEST_PATH="${OUTPUT_DIR}/doc/${name_lower}/html" -P "${OUTPUT_DIR}/doc/copy_img.cmake"
        WORKING_DIRECTORY ${OUTPUT_DIR}
        COMMENT "Generating ${DOXY_NAME} API documentation with Doxygen"
        VERBATIM
    )
    set_target_properties("DOXY-${DOXY_NAME}" PROPERTIES FOLDER "doc" EXCLUDE_FROM_ALL TRUE)
endfunction()

function(make_documentation OUTPUT_DIR DOXY_NAME BRIEF INPUT_LIST TAGFILE INPUT_TAG_LIST 
         EXTRA_FILE_LIST, image_path_list aliases_list)
    string(TOLOWER ${DOXY_NAME} name_lower)

    make_template(
        "${OUTPUT_DIR}/${name_lower}.doxy" 
        "${DOXY_NAME}" 
        "${BRIEF}" 
        "${OUTPUT_DIR}/doc/${name_lower}"
        "${INPUT_LIST}" 
        "${TAGFILE}" 
        "${INPUT_TAG_LIST}"
        "${EXTRA_FILE_LIST}"
        "${image_path_list}"
        "${aliases_list}"
    )
    make_doxy_target(
        "${OUTPUT_DIR}"
        "${DOXY_NAME}"
        "${image_path_list}"
    )
endfunction()

function(make_help INPUT_DIR SOURCE_LIST image_path_list)
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

    set(ADITIONAL_FLAGS_LIST 
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
    string(REGEX REPLACE ";" " \n " ADITIONAL_FLAGS "${ADITIONAL_FLAGS_LIST}")
    set(ADITIONAL_FLAGS ${ADITIONAL_FLAGS}) # ADITIONAL_FLAGS is appended to Doxygen.in

    make_documentation(
        "${ivw_doxy_out}" 
        "Help" 
        "Inviwo help"  
        "${SOURCE_LIST}"
        "" 
        ""
        "${ivw_doxy_extra_files}"
        "${image_path_list}"
        "${aliases_list}"
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
    file(WRITE "${ivw_doxy_out}/doc/help/inviwo.qhcp" ${INV_QHCP})

    add_custom_target("DOXY-QCH"
        COMMAND ${CMAKE_COMMAND} -E echo "Clean unused files"
        COMMAND ${CMAKE_COMMAND} -P "${ivw_doxy_out}/doc/clean_help.cmake"
        COMMAND ${CMAKE_COMMAND} -E echo "Building inviwo.qch"
        COMMAND ${CMAKE_COMMAND} -E echo "Running: ${IVW_DOXY_QHELPGENERATOR} -o ${ivw_doxy_out}/doc/help/inviwo.qch ${ivw_doxy_out}/doc/help/html/index.qhp"
        COMMAND ${IVW_DOXY_QHELPGENERATOR} "-o" "${ivw_doxy_out}/doc/help/inviwo.qch" "${ivw_doxy_out}/doc/help/html/index.qhp"
        COMMAND ${CMAKE_COMMAND} -E echo "Building inviwo.qhc"
        COMMAND ${CMAKE_COMMAND} -E echo "Running: ${IVW_DOXY_QCOLLECTIONGENERATOR} -o ${ivw_doxy_out}/doc/help/inviwo.qhc ${ivw_doxy_out}/doc/help/inviwo.qhcp"
        COMMAND ${IVW_DOXY_QCOLLECTIONGENERATOR} "-o" "${ivw_doxy_out}/doc/help/inviwo.qhc" "${ivw_doxy_out}/doc/help/inviwo.qhcp"
        COMMAND ${CMAKE_COMMAND} -E echo "Copy ${ivw_doxy_out}/doc/help/inviwo.q* to ${IVW_ROOT_DIR}/data/help/"
        COMMAND ${CMAKE_COMMAND} -E copy "${ivw_doxy_out}/doc/help/inviwo.qch" "${IVW_ROOT_DIR}/data/help/"
        COMMAND ${CMAKE_COMMAND} -E copy "${ivw_doxy_out}/doc/help/inviwo.qhc" "${IVW_ROOT_DIR}/data/help/"
        COMMAND ${CMAKE_COMMAND} -E copy "${ivw_doxy_out}/doc/help/inviwo.qhcp" "${IVW_ROOT_DIR}/data/help/"

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

    find_package(Perl QUIET)            # sets, PERL_FOUND, PERL_EXECUTABLE
    find_package(Doxygen QUIET)         # sets, DOXYGEN_FOUND, DOXYGEN_EXECUTABLE, 
                                        # DOXYGEN_DOT_FOUND, DOXYGEN_DOT_EXECUTABLE    
    find_package(PythonInterp QUIET)    # sets, PYTHONINTERP_FOUND PYTHON_EXECUTABLE

    if(NOT ${DOXYGEN_FOUND})
        ivw_message(WARN "Tried to create doxygen project, but doxygen was not found")
        return()
    endif()

    if(${DOXYGEN_DOT_FOUND})
        get_filename_component(DOXYGEN_DOT_PATH ${DOXYGEN_DOT_EXECUTABLE} PATH)
    endif()

    set(ivw_doxy_dir ${IVW_ROOT_DIR}/tools/doxygen)
    set(ivw_doxy_out ${CMAKE_CURRENT_BINARY_DIR})
    set(ivw_doxy_tag_files "")
    set(ivw_doxy_depends "")

    set(GENERATE_IMG "YES")
    string(REGEX REPLACE ";" " \\\\\n                         " SHADER_INC_PATH "${IVW_SHADER_INCLUDE_PATHS}")
    set(SHADER_INC_PATH ${SHADER_INC_PATH})

    if(PYTHONINTERP_FOUND)
        set(FILER_PATTERNS_LIST
            "\"*.frag=${ivw_doxy_dir}/filter/glslfilter.py\""
            "\"*.vert=${ivw_doxy_dir}/filter/glslfilter.py\""
            "\"*.geom=${ivw_doxy_dir}/filter/glslfilter.py\""
            "\"*.glsl=${ivw_doxy_dir}/filter/glslfilter.py\""
         )
        string(REGEX REPLACE ";" " \\\\\n                         " FILER_PATTERNS "${FILER_PATTERNS_LIST}")
        set(FILER_PATTERNS ${FILER_PATTERNS})
    endif()

    set(ivw_doxy_extra_files 
        "${ivw_doxy_dir}/style/img_downArrow.png"
    )

    set(aliases_list
        "docpage{1}=\"\\ingroup processors \\n \#\\1\""
        "docpage{2}=\"\\ingroup processors \\n \#\\2\""
    )

    set(image_path_list 
        "${IVW_ROOT_DIR}/data/help/images"
    )

    set(ivw_doxy_all_sources 
        "${IVW_INCLUDE_DIR}"
        "${IVW_SOURCE_DIR}"
        "${IVW_APPLICATION_DIR}"
    )

    if(${MSVC})
        set(WARN_FORMAT "\$file(\$line): \$text")
    endif()

    add_custom_target("DOXY-ALL"
        WORKING_DIRECTORY ${ivw_doxy_out}
        COMMENT "Generating ALL API documentation with Doxygen"
        VERBATIM
    )
    set_target_properties("DOXY-ALL" PROPERTIES FOLDER "doc")

    # Core
    set(ivw_doxy_tag_core "${ivw_doxy_out}/doc/core/ivwcore.tag")
    make_documentation(
        "${ivw_doxy_out}" 
        "Core" 
        "Core functionality of Inviwo" 
        "${IVW_CORE_INCLUDE_DIR};${IVW_CORE_SOURCE_DIR}" 
        "${ivw_doxy_tag_core}"
        "${ivw_doxy_tag_files}"
        "${ivw_doxy_extra_files}"
        "${image_path_list}"
        "${aliases_list}"
    )
    list(APPEND ivw_doxy_depends "DOXY-Core")
    list(APPEND ivw_doxy_tag_files "${ivw_doxy_tag_core}=${ivw_doxy_out}/doc/core/html")
    
    # OT
    set(ivw_doxy_tag_qt "${ivw_doxy_out}/doc/qt/ivwqt.tag")
    #list(APPEND ivw_doxy_tag_files "qtcore.tags=http://qt-project.org/doc/qt-5/")
    make_documentation(
        "${ivw_doxy_out}" 
        "Qt" 
        "Main Qt elements of Inviwo" 
        "${IVW_QT_INCLUDE_DIR};${IVW_QT_SOURCE_DIR}" 
        "${ivw_doxy_tag_qt}"
        "${ivw_doxy_tag_files}"
        "${ivw_doxy_extra_files}"
        "${image_path_list}"
        "${aliases_list}"
    )
    foreach(depends "${ivw_doxy_depends}")
        add_dependencies("DOXY-Qt" ${depends})
    endforeach()
    list(APPEND ivw_doxy_depends "DOXY-Qt")
    list(APPEND ivw_doxy_tag_files "${ivw_doxy_tag_qt}=${ivw_doxy_out}/doc/qt/html")

    # Modules
    set(ivw_doxy_module_bases "")
    foreach(mod ${${modules_var}})
        list(APPEND ivw_doxy_module_bases ${${mod}_base})
        list(REMOVE_DUPLICATES ivw_doxy_module_bases)
        if(EXISTS "${${mod}_path}/docs/images")
            list(APPEND image_path_list "${${mod}_path}/docs/images")
        endif()
        list(APPEND ivw_doxy_all_sources ${${mod}_path})
    endforeach()
    get_unique_names(unique_names "${ivw_doxy_module_bases}")
    
    set(index 0)
    foreach(base ${ivw_doxy_module_bases})
        list(GET unique_names ${index} name)
            
        string(REPLACE "/" "-" desc_name ${name})
        string(TOLOWER ${desc_name} desc_name_lower)
        
        set(inc_dirs "")
        foreach(mod ${${modules_var}})
            if(${${mod}_path} STREQUAL base)
                list(APPEND inc_dirs ${module})
            endif()
        endforeach()
        
        set(ivw_doxy_tag_module "${ivw_doxy_out}/doc/${desc_name_lower}/${desc_name_lower}.tag")
        make_documentation(
            "${ivw_doxy_out}" 
            "${desc_name}" 
            "Modules for ${desc_name}"  
            "${inc_dirs}" 
            "${ivw_doxy_tag_module}"
            "${ivw_doxy_tag_files}"
            "${ivw_doxy_extra_files}"
            "${image_path_list}"
            "${aliases_list}"
        )
        foreach(depends "${ivw_doxy_depends}")
            add_dependencies("DOXY-${desc_name}" ${depends})
        endforeach()
        list(APPEND ivw_doxy_depends "DOXY-${desc_name}")
        list(APPEND ivw_doxy_tag_files "${ivw_doxy_tag_module}=${ivw_doxy_out}/doc/${desc_name_lower}/html")
        MATH(EXPR index "${index}+1")
    endforeach()
        
    # Apps
    set(ivw_doxy_tag_apps "${ivw_doxy_out}/doc/apps/ivwapps.tag")
    make_documentation(
        "${ivw_doxy_out}" 
        "Apps" 
        "Applications using Inviwo Core and Modules" 
        "${IVW_APPLICATION_DIR}" 
        "${ivw_doxy_tag_apps}"
        "${ivw_doxy_tag_files}"
        "${ivw_doxy_extra_files}"
        "${image_path_list}"
        "${aliases_list}"
    )
    foreach(depends "${ivw_doxy_depends}")
        add_dependencies("DOXY-Apps" ${depends})
    endforeach()
    list(APPEND ivw_doxy_depends "DOXY-Apps")
    list(APPEND ivw_doxy_tag_files "${ivw_doxy_tag_apps}={ivw_doxy_out}/doc/apps/html")

    foreach(depends "${ivw_doxy_depends}")
        add_dependencies("DOXY-ALL" ${depends})
    endforeach()


    # All In one.
    make_documentation(
        "${ivw_doxy_out}" 
        "Inviwo" 
        "Inviwo documentation" 
        "${ivw_doxy_all_sources}"
        "" 
        ""
        "${ivw_doxy_extra_files}"
        "${image_path_list}"
        "${aliases_list}"
    )
    add_dependencies("DOXY-ALL" "DOXY-Inviwo")


    # Help, used for the help inside invowo
    make_help("${ivw_doxy_dir}/help" "${ivw_doxy_all_sources}" "${image_path_list}")
    add_dependencies("DOXY-ALL" "DOXY-Help")
    add_dependencies("DOXY-ALL" "DOXY-QCH")

    foreach(path ${IMG_PATHS})
        message(${path})
    endforeach()

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
    file(WRITE "${ivw_doxy_out}/doc/copy_img.cmake" ${COPY_SCRIPT})

    # make a clean help script
    set(CLEAN_HELP_SCRIPT_LIST
        "file(GLOB save \"${ivw_doxy_out}/doc/help/html/docpage*.html\")"
        "file(GLOB todelete \"${ivw_doxy_out}/doc/help/html/*.js\" \"${ivw_doxy_out}/doc/help/html/*.html\")"
        "list(REMOVE_ITEM todelete \${save})"
        "foreach(item \${todelete})"
        "    message(\"Remove: \${item}\")"
        "    file(REMOVE \${item})"
        "endforeach()"
    )
    string(REPLACE ";" "\n" CLEAN_HELP_SCRIPT "${CLEAN_HELP_SCRIPT_LIST}")
    file(WRITE "${ivw_doxy_out}/doc/clean_help.cmake" ${CLEAN_HELP_SCRIPT})

 endfunction()