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
 
option(IVW_DOXYGEN_PROJECT "Create Inviwo doxygen files" ON)

function(make_template FILENAME DOXY_NAME BRIEF OUTPUT_DIR INPUT_LIST TAGFILE INPUT_TAG_LIST
                       EXTRA_FILE_LIST IMAGE_PATH_LIST ALIASES_LIST)

    set(PROJNAME ${DOXY_NAME})

    set(MAINPAGE "${IVW_ROOT_DIR}/README.md")

    list(APPEND INPUT_LIST ${MAINPAGE})
    string(REGEX REPLACE ";" " \\\\\n                         " INPUTS "${INPUT_LIST}")
    set(INPUTS ${INPUTS})
    
    string(REGEX REPLACE ";" " \\\\\n                         " INPUT_TAGS "${INPUT_TAG_LIST}")
    set(INPUT_TAGS ${INPUT_TAGS})

    string(REGEX REPLACE ";" " \\\\\n                         " EXTRA_FILES "${EXTRA_FILE_LIST}")
    set(EXTRA_FILES ${EXTRA_FILES})

    string(REGEX REPLACE ";" " \\\\\n                         " IMAGE_PATH "${IMAGE_PATH_LIST}")
    set(IMAGE_PATH ${IMAGE_PATH})

    string(REGEX REPLACE ";" " \\\\\n                         " ALIASES "${ALIASES_LIST}")
    set(ALIASES ${ALIASES})

    configure_file(${IVW_DOXY_DIR}/Doxygen.in ${FILENAME})
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

function(make_doxy_target OUTPUT_DIR DOXY_NAME IMAGE_PATH_LIST)
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
    set_target_properties("DOXY-${DOXY_NAME}" PROPERTIES FOLDER "doc" EXCLUDE_FROM_DEFAULT_BUILD 1)
endfunction()

function(make_documentation OUTPUT_DIR DOXY_NAME BRIEF INPUT_LIST TAGFILE INPUT_TAG_LIST 
         EXTRA_FILE_LIST, IMAGE_PATH_LIST ALIASES_LIST)
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
        "${IMAGE_PATH_LIST}"
        "${ALIASES_LIST}"
    )
    make_doxy_target(
        "${OUTPUT_DIR}"
        "${DOXY_NAME}"
        "${IMAGE_PATH_LIST}"
    )
endfunction()

function(make_help INPUT_DIR SOURCE_LIST IMAGE_PATH_LIST)
    # Help, used for the help inside invowo
    set(GENERATE_QHP "YES")
    set(HTML_LAYOUT_FILE "${INPUT_DIR}/layout.xml")
    set(HTML_STYLESHEET "${INPUT_DIR}/stylesheet.css")
    set(HTML_HEADER "${INPUT_DIR}/header.html")
    set(HTML_FOOTER "${INPUT_DIR}/footer.html")
    set(ALIASES_LIST
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
    set(ADITIONAL_FLAGS ${ADITIONAL_FLAGS})

    make_documentation(
        "${IVW_DOXY_OUT}" 
        "Help" 
        "Inviwo help"  
        "${SOURCE_LIST}"
        "" 
        ""
        "${IVW_DOXY_EXTRA_FILES}"
        "${IMAGE_PATH_LIST}"
        "${ALIASES_LIST}"
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
    file(WRITE "${IVW_DOXY_OUT}/doc/help/inviwo.qhcp" ${INV_QHCP})

    add_custom_target("DOXY-QCH"
        COMMAND ${CMAKE_COMMAND} -E echo "Building doxygen ${QCH}"
        COMMAND ${IVW_DOXY_QHELPGENERATOR} "-o" "${IVW_DOXY_OUT}/doc/help/inviwo.qch" "${IVW_DOXY_OUT}/doc/help/html/index.qhp"
        COMMAND ${IVW_DOXY_QCOLLECTIONGENERATOR} "-o" "${IVW_DOXY_OUT}/doc/help/inviwo.qhc" "${IVW_DOXY_OUT}/doc/help/inviwo.qhcp"
        COMMAND ${CMAKE_COMMAND} -E copy "${IVW_DOXY_OUT}/doc/help/inviwo.qch" "${IVW_ROOT_DIR}/data/help/"
        COMMAND ${CMAKE_COMMAND} -E copy "${IVW_DOXY_OUT}/doc/help/inviwo.qhc" "${IVW_ROOT_DIR}/data/help/"
        COMMAND ${CMAKE_COMMAND} -E copy "${IVW_DOXY_OUT}/doc/help/inviwo.qhcp" "${IVW_ROOT_DIR}/data/help/"

        WORKING_DIRECTORY ${OUTPUT_DIR}
        COMMENT "Generating QCH files"
        VERBATIM
    )
    add_dependencies("DOXY-QCH" "DOXY-Help")
    set_target_properties("DOXY-QCH" PROPERTIES FOLDER "doc" EXCLUDE_FROM_DEFAULT_BUILD 1)

endfunction()


if(${IVW_DOXYGEN_PROJECT})
    find_package(Perl QUIET)            # sets, PERL_FOUND, PERL_EXECUTABLE
    find_package(Doxygen QUIET)         # sets, DOXYGEN_FOUND, DOXYGEN_EXECUTABLE, 
                                        # DOXYGEN_DOT_FOUND, DOXYGEN_DOT_EXECUTABLE    
    find_package(PythonInterp QUIET)    # sets, PYTHONINTERP_FOUND PYTHON_EXECUTABLE

    if(NOT ${DOXYGEN_FOUND})
        return()
    endif()

    if(${DOXYGEN_DOT_FOUND})
        get_filename_component(DOXYGEN_DOT_PATH ${DOXYGEN_DOT_EXECUTABLE} PATH)
    endif()

    set(IVW_DOXY_DIR ${IVW_ROOT_DIR}/tools/doxygen)
    set(IVW_DOXY_OUT ${CMAKE_CURRENT_BINARY_DIR})
    set(IVW_DOXY_TAG_FILES "")
    set(IVW_DOXY_DEPENDS "")

    string(REGEX REPLACE ";" " \\\\\n                         " SHADER_INC_PATH "${IVW_SHADER_INCLUDE_PATHS}")
    set(SHADER_INC_PATH ${SHADER_INC_PATH})

    if(PYTHONINTERP_FOUND)
        set(FILER_PATTERNS_LIST
            "\"*.frag=${IVW_DOXY_DIR}/filter/glslfilter.py\""
            "\"*.vert=${IVW_DOXY_DIR}/filter/glslfilter.py\""
            "\"*.geom=${IVW_DOXY_DIR}/filter/glslfilter.py\""
            "\"*.glsl=${IVW_DOXY_DIR}/filter/glslfilter.py\""
         )
        string(REGEX REPLACE ";" " \\\\\n                         " FILER_PATTERNS "${FILER_PATTERNS_LIST}")
        set(FILER_PATTERNS ${FILER_PATTERNS})
    endif()

    set(IVW_DOXY_EXTRA_FILES 
        "${IVW_DOXY_DIR}/style/img_downArrow.png"
    )

    set(ALIASES_LIST
        "docpage{1}=\"\\ingroup processors \\n \#\\1\""
        "docpage{2}=\"\\ingroup processors \\n \#\\2\""
    )

    set(IMAGE_PATH_LIST 
        "${IVW_ROOT_DIR}/data/help/images"
    )

    set(IVW_DOXY_ALL_SOURCES 
        "${IVW_INCLUDE_DIR}"
        "${IVW_SOURCE_DIR}"
        "${IVW_APPLICATION_DIR}"
    )

    if(${MSVC})
        set(WARN_FORMAT "\$file(\$line): \$text")
    endif()

    add_custom_target("DOXY-ALL"
        WORKING_DIRECTORY ${IVW_DOXY_OUT}
        COMMENT "Generating ALL API documentation with Doxygen"
        VERBATIM
    )
    set_target_properties("DOXY-ALL" PROPERTIES FOLDER "doc")

    # Core
    set(IVW_DOXY_TAG_CORE "${IVW_DOXY_OUT}/doc/core/ivwcore.tag")
    make_documentation(
        "${IVW_DOXY_OUT}" 
        "Core" 
        "Core functionality of Inviwo" 
        "${IVW_CORE_INCLUDE_DIR};${IVW_CORE_SOURCE_DIR}" 
        "${IVW_DOXY_TAG_CORE}"
        "${IVW_DOXY_TAG_FILES}"
        "${IVW_DOXY_EXTRA_FILES}"
        "${IMAGE_PATH_LIST}"
        "${ALIASES_LIST}"
    )
    list(APPEND IVW_DOXY_DEPENDS "DOXY-Core")
    list(APPEND IVW_DOXY_TAG_FILES "${IVW_DOXY_TAG_CORE}=${IVW_DOXY_OUT}/doc/core/html")
    
    # OT
    set(IVW_DOXY_TAG_QT "${IVW_DOXY_OUT}/doc/qt/ivwqt.tag")
    #list(APPEND IVW_DOXY_TAG_FILES "qtcore.tags=http://qt-project.org/doc/qt-5/")
    make_documentation(
        "${IVW_DOXY_OUT}" 
        "Qt" 
        "Main Qt elements of Inviwo" 
        "${IVW_QT_INCLUDE_DIR};${IVW_QT_SOURCE_DIR}" 
        "${IVW_DOXY_TAG_QT}"
        "${IVW_DOXY_TAG_FILES}"
        "${IVW_DOXY_EXTRA_FILES}"
        "${IMAGE_PATH_LIST}"
        "${ALIASES_LIST}"
    )
    foreach(depends "${IVW_DOXY_DEPENDS}")
        add_dependencies("DOXY-Qt" ${depends})
    endforeach()
    list(APPEND IVW_DOXY_DEPENDS "DOXY-Qt")
    list(APPEND IVW_DOXY_TAG_FILES "${IVW_DOXY_TAG_QT}=${IVW_DOXY_OUT}/doc/qt/html")

    # Modules
    set(IVW_DOXY_MODULE_BASES "")
    foreach(module ${IVW_MODULE_PATHS})
        get_filename_component(folder_name ${module} NAME)
        get_filename_component(path_name ${module} PATH)
        list(APPEND IVW_DOXY_MODULE_BASES ${path_name})
        list(REMOVE_DUPLICATES IVW_DOXY_MODULE_BASES)
        if(EXISTS "${module}/images")
            list(APPEND IMAGE_PATH_LIST "${module}/images")
        endif()
        list(APPEND IVW_DOXY_ALL_SOURCES ${module})
    endforeach()
    get_unique_names(unique_names "${IVW_DOXY_MODULE_BASES}")
    
    set(index 0)
    foreach(base ${IVW_DOXY_MODULE_BASES})
        list(GET unique_names ${index} name)
            
        string(REPLACE "/" "-" desc_name ${name})
        string(TOLOWER ${desc_name} desc_name_lower)
        
        set(inc_dirs "")
        foreach(module ${IVW_MODULE_PATHS})
            get_filename_component(path_name ${module} PATH)
            if ( path_name STREQUAL base )
                list(APPEND inc_dirs ${module})
            endif()
        endforeach()
        
        set(IVW_DOXY_TAG_MODULE "${IVW_DOXY_OUT}/doc/${desc_name_lower}/${desc_name_lower}.tag")
        make_documentation(
            "${IVW_DOXY_OUT}" 
            "${desc_name}" 
            "Modules for ${desc_name}"  
            "${inc_dirs}" 
            "${IVW_DOXY_TAG_MODULE}"
            "${IVW_DOXY_TAG_FILES}"
            "${IVW_DOXY_EXTRA_FILES}"
            "${IMAGE_PATH_LIST}"
            "${ALIASES_LIST}"
        )
        foreach(depends "${IVW_DOXY_DEPENDS}")
            add_dependencies("DOXY-${desc_name}" ${depends})
        endforeach()
        list(APPEND IVW_DOXY_DEPENDS "DOXY-${desc_name}")
        list(APPEND IVW_DOXY_TAG_FILES "${IVW_DOXY_TAG_MODULE}=${IVW_DOXY_OUT}/doc/${desc_name_lower}/html")
        MATH(EXPR index "${index}+1")
    endforeach()
        
    # Apps
    set(IVW_DOXY_TAG_APPS "${IVW_DOXY_OUT}/doc/apps/ivwapps.tag")
    make_documentation(
        "${IVW_DOXY_OUT}" 
        "Apps" 
        "Applications using Inviwo Core and Modules" 
        "${IVW_APPLICATION_DIR}" 
        "${IVW_DOXY_TAG_APPS}"
        "${IVW_DOXY_TAG_FILES}"
        "${IVW_DOXY_EXTRA_FILES}"
        "${IMAGE_PATH_LIST}"
        "${ALIASES_LIST}"
    )
    foreach(depends "${IVW_DOXY_DEPENDS}")
        add_dependencies("DOXY-Apps" ${depends})
    endforeach()
    list(APPEND IVW_DOXY_DEPENDS "DOXY-Apps")
    list(APPEND IVW_DOXY_TAG_FILES "${IVW_DOXY_TAG_APPS}={IVW_DOXY_OUT}/doc/apps/html")

    foreach(depends "${IVW_DOXY_DEPENDS}")
        add_dependencies("DOXY-ALL" ${depends})
    endforeach()


    # All In one.
    make_documentation(
        "${IVW_DOXY_OUT}" 
        "Inviwo" 
        "Inviwo documentation" 
        "${IVW_DOXY_ALL_SOURCES}"
        "" 
        ""
        "${IVW_DOXY_EXTRA_FILES}"
        "${IMAGE_PATH_LIST}"
        "${ALIASES_LIST}"
    )
    add_dependencies("DOXY-ALL" "DOXY-Inviwo")


    # Help, used for the help inside invowo
    make_help("${IVW_DOXY_DIR}/help" "${IVW_DOXY_ALL_SOURCES}" "${IMAGE_PATH_LIST}")
    add_dependencies("DOXY-ALL" "DOXY-Help")
    add_dependencies("DOXY-ALL" "DOXY-QCH")

    foreach(path ${IMG_PATHS})
        message(${path})
    endforeach()

    # make a img-copy script
    set(COPY_SCRIPT_LIST
        "foreach(path ${IMAGE_PATH_LIST})"
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
    file(WRITE "${IVW_DOXY_OUT}/doc/copy_img.cmake" ${COPY_SCRIPT})

 endif()