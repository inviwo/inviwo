 #################################################################################
 #
 # Inviwo - Interactive Visualization Workshop
 #
 # Copyright (c) 2014-2019 Inviwo Foundation
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
 
include(CMakeParseArguments)

function(ivw_get_subdirs_recursive retval start) 
    set(res "")
    get_property(subdirs DIRECTORY ${start} PROPERTY SUBDIRECTORIES)
    list(APPEND res ${subdirs})
    foreach(subdir IN LISTS subdirs)
        ivw_get_subdirs_recursive(ret ${subdir})
        list(APPEND res ${ret})
        list(REMOVE_DUPLICATES res)
    endforeach()
    set(${retval} ${res} PARENT_SCOPE)
endfunction()

function(ivw_get_include_dirs retval) 
    set(res "")
    ivw_get_subdirs_recursive(subdirs ${IVW_ROOT_DIR})
    foreach(subdir IN LISTS subdirs)
        get_property(targets DIRECTORY ${subdir} PROPERTY BUILDSYSTEM_TARGETS)
        foreach(target IN LISTS targets)
            ivw_get_target_property_recursive(dirs ${target} INTERFACE_INCLUDE_DIRECTORIES True)
            list(APPEND res ${dirs})
            ivw_get_target_property_recursive(dirs ${target} INCLUDE_DIRECTORIES False)
            list(APPEND res ${dirs})
            list(REMOVE_DUPLICATES res)
        endforeach()
    endforeach()

    # filter generator expr
    set(res2 "")
    foreach(item IN LISTS res)
        string(REGEX MATCH [=[^\$<BUILD_INTERFACE:([^$<>]+)>$]=] repl ${item})
        if(CMAKE_MATCH_1)
            list(APPEND res2 "\"${CMAKE_MATCH_1}\"")
        endif()
        string(REGEX MATCH [=[^[^$<>]*$]=] repl ${item})
        if(CMAKE_MATCH_0)
            list(APPEND "res2" "\"${CMAKE_MATCH_0}\"")
        endif()
    endforeach()
    list(REMOVE_DUPLICATES res2)
    set(${retval} ${res2} PARENT_SCOPE)
endfunction()

if(${MSVC})
    option(IVW_DOXYGEN_PROJECT "Create Inviwo doxygen files" OFF)
    if(${IVW_DOXYGEN_PROJECT})
        message(STATUS 
            "In Visual Studio the doxygen project will rerun every time you run\n"
            "\"ALL_BUILD\" even if it is up to date. Hence, you propbably want\n"
            "to disable generation of doxygen on each build this can be done\n"
            "by right clicking on the doc folder in the \"Solution Explorer\" and\n"
            "select \"Unload Project\".")
    endif()
else()
    find_package(Doxygen QUIET)
    option(IVW_DOXYGEN_PROJECT "Create Inviwo doxygen files" ${DOXYGEN_FOUND})
endif()

option(IVW_DOXYGEN_OPEN_HTML_AFTER_BUILD "Open the generated doxygen HTML when build is done" OFF)


function(ivw_private_format_doxy_arg retval )
    string(REGEX REPLACE ";" " \\\\\n                         " result "${ARGN}")
    set(${retval} ${result} PARENT_SCOPE)
endfunction()


function(ivw_private_make_doxyfile)
    set(options 
        GENERATE_IMG 
        GENERATE_QHP
    )
    set(oneValueArgs 
        NAME 
        BRIEF 
        OUTPUT_DIR 
        WARNING_FORMAT 
        TAG_FILE 
    )
    set(multiValueArgs 
        INPUTS 
        IMAGE_PATHS 
        ALIASES 
        INPUT_TAGS
        FILTER_PATTERNS
        ADDITIONAL_FLAGS
    )
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

    ivw_private_format_doxy_arg(inputs ${ARG_INPUTS})
    ivw_private_format_doxy_arg(image_paths ${ARG_IMAGE_PATHS})
    ivw_private_format_doxy_arg(aliases ${ARG_ALIASES})
    ivw_private_format_doxy_arg(input_tags ${ARG_INPUT_TAGS})
    ivw_private_format_doxy_arg(filter_patterns ${ARG_FILTER_PATTERNS})

    ivw_get_include_dirs(incpaths)
    ivw_private_format_doxy_arg(incpaths ${incpaths})

    string(REGEX REPLACE ";" "\n" additional_flags "${ARG_ADDITIONAL_FLAGS}")

    string(TOLOWER ${ARG_NAME} name_lower)

    set(doxyfile "\
PROJECT_NAME           = \"${ARG_NAME}\"
PROJECT_NUMBER         = \"${IVW_VERSION}\"
PROJECT_BRIEF          = \"${ARG_BRIEF}\"
PROJECT_LOGO           = \"${IVW_RESOURCES_DIR}/inviwo/inviwo_dark.png\"
OUTPUT_DIRECTORY       = \"${ARG_OUTPUT_DIR}/${name_lower}\"
WARNINGS               = YES
WARN_NO_PARAMDOC       = NO
WARN_IF_UNDOCUMENTED   = NO
WARN_FORMAT            = \"${ARG_WARNING_FORMAT}\"
QUIET                  = YES
CREATE_SUBDIRS         = NO
ALLOW_UNICODE_NAMES    = YES
JAVADOC_AUTOBRIEF      = NO
QT_AUTOBRIEF           = NO
FULL_PATH_NAMES        = NO
SHOW_FILES             = NO
SOURCE_BROWSER         = NO
VERBATIM_HEADERS       = NO
SORT_GROUP_NAMES       = YES
EXTRACT_LOCAL_CLASSES  = NO

TAB_SIZE               = 4

ALIASES                = ${aliases}
MARKDOWN_SUPPORT       = YES
BUILTIN_STL_SUPPORT    = YES

CASE_SENSE_NAMES       = YES

INPUT                  = ${IVW_ROOT_DIR}/README.md \\
                         ${inputs}

ENABLE_PREPROCESSING   = YES
SEARCH_INCLUDES        = NO

IMAGE_PATH             = ${image_paths}

INCLUDE_PATH           = ${incpaths}

EXTENSION_MAPPING      = no_extension=C++ frag=C++ vert=C++ geom=C++ glsl=C++

FILE_PATTERNS          = *.c \\
                         *.cpp \\
                         *.hpp \\
                         *.h \\
                         *.cl \\
                         *.frag \\
                         *.vert \\
                         *.geom \\
                         *.dox \\
                         *.glsl

RECURSIVE              = YES
EXCLUDE                =
EXCLUDE_PATTERNS       = */moc_* \\
                         */qrc_* \\
                         */modules/*/ext/* \\
                         */clogs/* \\
                         *-test.cpp \\
                         *sqlite3* \\

EXCLUDE_SYMBOLS        = cl::* \\
                         TCLAP::* \\
                         clogs::* \\
                         glm::*

FILTER_PATTERNS        = ${filter_patterns}

USE_MDFILE_AS_MAINPAGE = ${IVW_ROOT_DIR}/README.md

HTML_EXTRA_FILES       = ${ivw_doxy_dir}/style/img_downArrow.png

EXAMPLE_PATH           = ${IVW_ROOT_DIR}

HTML_COLORSTYLE_HUE    = 240
HTML_COLORSTYLE_SAT    = 6
HTML_COLORSTYLE_GAMMA  = 80

HTML_DYNAMIC_SECTIONS  = YES
HTML_INDEX_NUM_ENTRIES = 100

GENERATE_LATEX         = NO

GENERATE_DOCSET        = NO
DOCSET_FEEDNAME        = \"Interactive Visualization Workshop\"
DOCSET_BUNDLE_ID       = org.inviwo
DOCSET_PUBLISHER_ID    = org.inviwo
DOCSET_PUBLISHER_NAME  = Inviwo

GENERATE_QHP           = ${ARG_GENERATE_QHP}
QHP_NAMESPACE          = org.inviwo.${name_lower}
QHP_VIRTUAL_FOLDER     = doc

GENERATE_TREEVIEW      = YES
TREEVIEW_WIDTH         = 300

TAGFILES               = ${input_tags}
GENERATE_TAGFILE       = ${ARG_TAG_FILE}

ALLEXTERNALS           = NO
EXTERNAL_GROUPS        = YES
EXTERNAL_PAGES         = YES

PERL_PATH              = ${PERL_EXECUTABLE}

CLASS_DIAGRAMS         = ${ARG_GENERATE_IMG}
HAVE_DOT               = ${DOXYGEN_DOT_FOUND}

CLASS_GRAPH            = ${ARG_GENERATE_IMG}
COLLABORATION_GRAPH    = NO # To large to be useful most of the time, ${ARG_GENERATE_IMG}
GROUP_GRAPHS           = ${ARG_GENERATE_IMG}
UML_LOOK               = NO
UML_LIMIT_NUM_FIELDS   = 10
TEMPLATE_RELATIONS     = YES
INCLUDE_GRAPH          = NO # needs SEARCH_INCLUDES = YES, ${ARG_GENERATE_IMG}
INCLUDED_BY_GRAPH      = NO # needs SEARCH_INCLUDES = YES, ${ARG_GENERATE_IMG}

CALL_GRAPH             = NO
CALLER_GRAPH           = NO

GRAPHICAL_HIERARCHY    = NO
DIRECTORY_GRAPH        = NO
DOT_IMAGE_FORMAT       = png
INTERACTIVE_SVG        = ${ARG_GENERATE_IMG}
DOT_PATH               = \"${DOXYGEN_DOT_PATH}\"
DOTFILE_DIRS           = 
DOT_GRAPH_MAX_NODES    = 200
MAX_DOT_GRAPH_DEPTH    = 0
DOT_TRANSPARENT        = NO
DOT_MULTI_TARGETS      = YES
GENERATE_LEGEND        = YES
DOT_CLEANUP            = YES
PREDEFINED             = DOXYGEN_SHOULD_SKIP_THIS
SHOW_INCLUDE_FILES     = YES
ALPHABETICAL_INDEX     = YES

${additional_flags}
")
    if(NOT EXISTS "${ARG_OUTPUT_DIR}")
        file(MAKE_DIRECTORY ${ARG_OUTPUT_DIR})
    endif()
    file(WRITE ${ARG_OUTPUT_DIR}/${name_lower}.doxy ${doxyfile})
endfunction()

 # Help, used for the help inside inviwo
function(ivw_private_make_help)
    set(options "")
    set(oneValueArgs 
        NAME
        HTML_DIR
        WARNING_FORMAT 
        OUTPUT_DIR
        DOC_DIR
    )
    set(multiValueArgs 
        INPUTS
        IMAGE_PATHS
        FILTER_PATTERNS
    )
    cmake_parse_arguments(HARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

    string(TOLOWER ${HARG_NAME} name_lower)

    set(aliases_list
        "docpage{1}=\"\\page docpage-\\1 \\1\""
        "docpage{2}=\"\\page docpage-\\1 \\2\""
    )

    set(additional_flags_list 
        "LAYOUT_FILE            = ${HARG_HTML_DIR}/layout.xml"
        "HTML_STYLESHEET        = ${HARG_HTML_DIR}/stylesheet.css"
        "HTML_HEADER            = ${HARG_HTML_DIR}/header.html"
        "HTML_FOOTER            = ${HARG_HTML_DIR}/footer.html"
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

    ivw_private_make_doxyfile(
        NAME "${HARG_NAME}"
        BRIEF "Inviwo ${HARG_NAME} help"
        OUTPUT_DIR "${HARG_OUTPUT_DIR}"
        WARNING_FORMAT ${HARG_WARNING_FORMAT}
        INPUTS ${HARG_INPUTS}
        IMAGE_PATHS ${HARG_IMAGE_PATHS}
        ALIASES ${aliases_list}
        FILTER_PATTERNS ${HARG_FILTER_PATTERNS}
        ADDITIONAL_FLAGS ${additional_flags_list}
        GENERATE_QHP
    )

    find_program(IVW_DOXY_QHELPGENERATOR "qhelpgenerator")

    add_custom_target("DOXY-HELP-${HARG_NAME}"
        COMMAND ${CMAKE_COMMAND} -E echo "Building help for ${ARG_NAME}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${HARG_OUTPUT_DIR}/${name_lower}/html"
        COMMAND ${DOXYGEN_EXECUTABLE} "${HARG_OUTPUT_DIR}/${name_lower}.doxy"

        COMMAND ${CMAKE_COMMAND} -DQHP_FILE="${HARG_OUTPUT_DIR}/${name_lower}/html/index.qhp"
                                 -DSTRIPPED_FILE="${HARG_OUTPUT_DIR}/${name_lower}/html/index-stripped.qhp"
                                 -P "${ivw_doxy_dir}/strip-qhp.cmake"

        COMMAND ${CMAKE_COMMAND} -E echo "Building ${name_lower}.qch"
        COMMAND ${CMAKE_COMMAND} -E echo "Running: ${IVW_DOXY_QHELPGENERATOR} -o ${HARG_OUTPUT_DIR}/${name_lower}.qch ${HARG_OUTPUT_DIR}/${name_lower}/html/index-stripped.qhp"
        COMMAND ${IVW_DOXY_QHELPGENERATOR} "-o" "${HARG_OUTPUT_DIR}/${name_lower}.qch" 
                                            "${HARG_OUTPUT_DIR}/${name_lower}/html/index-stripped.qhp"

        COMMAND ${CMAKE_COMMAND} -E make_directory "${HARG_DOC_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy "${HARG_OUTPUT_DIR}/${name_lower}.qch" "${HARG_DOC_DIR}/"

        WORKING_DIRECTORY ${HARG_OUTPUT_DIR}
        COMMENT "Building Qt QCH files for ${HARG_NAME}"
        VERBATIM
    )
    set_target_properties("DOXY-HELP-${HARG_NAME}" PROPERTIES FOLDER "doc/qch" EXCLUDE_FROM_ALL TRUE)
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
    message(WARNING "Tried to create doxygen project, but doxygen was not found")
        return()
    endif()

    if(${DOXYGEN_DOT_FOUND})
        get_filename_component(DOXYGEN_DOT_PATH ${DOXYGEN_DOT_EXECUTABLE} PATH)
    endif()

    set(ivw_doxy_dir ${IVW_ROOT_DIR}/tools/doxygen)
    set(IVW_DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/doc" 
        CACHE PATH "Path to put the doxygen output")
    set(output_dir ${IVW_DOXYGEN_OUTPUT_DIRECTORY})
    set(tag_files "")
    set(dependency_list "")

    set(GENERATE_IMG "YES")
    
    if(PYTHONINTERP_FOUND)
        #set(PREFIX_PYTHON "")
        set(PREFIX_PYTHON "${PYTHON_EXECUTABLE} ") # This is sometimes needed but gives errors on win7
        set(filer_patterns_list
            "\"*.frag=${PREFIX_PYTHON}${ivw_doxy_dir}/filter/glslfilter.py\""
            "\"*.vert=${PREFIX_PYTHON}${ivw_doxy_dir}/filter/glslfilter.py\""
            "\"*.geom=${PREFIX_PYTHON}${ivw_doxy_dir}/filter/glslfilter.py\""
            "\"*.glsl=${PREFIX_PYTHON}${ivw_doxy_dir}/filter/glslfilter.py\""
         )
    else()
        set(filer_patterns_list "")
    endif()

    set(extra_files "${ivw_doxy_dir}/style/img_downArrow.png")
    set(aliases_list
        "docpage{1}=\"\\ingroup processors \\n \#\\1\""
        "docpage{2}=\"\\ingroup processors \\n \#\\2\""
    )
    set(image_path_list "${IVW_ROOT_DIR}/data/help/images")
    set(all_input 
        "${IVW_INCLUDE_DIR}"
        "${IVW_SOURCE_DIR}"
        "${IVW_CORE_INCLUDE_DIR}"
        "${IVW_CORE_SOURCE_DIR}"
        "${IVW_QT_INCLUDE_DIR}"
        "${IVW_QT_SOURCE_DIR}"
        "${IVW_APPLICATION_DIR}"
        "${IVW_ROOT_DIR}/docs"
    )

    if(${MSVC})
        set(warn_format "\$file(\$line): \$text")
    endif()

    # Group target
    add_custom_target("DOXY-ALL"
        WORKING_DIRECTORY ${output_dir}
        COMMENT "Generating ALL API documentation with Doxygen"
        VERBATIM
    )
    set_target_properties("DOXY-ALL" PROPERTIES FOLDER "doc" EXCLUDE_FROM_ALL TRUE)

    # Modules
    set(module_bases "")
    foreach(mod ${${modules_var}})
        if(${${mod}_opt}) # Only include enabled modules
            list(APPEND all_input ${${mod}_path})
            if(EXISTS "${${mod}_path}/docs/images")
                list(APPEND image_path_list "${${mod}_path}/docs/images")
            endif()
        endif()
    endforeach()

    # Inviwo
    ivw_private_make_doxyfile(
        NAME "Inviwo"
        BRIEF "Inviwo documentation"
        OUTPUT_DIR "${output_dir}"
        WARNING_FORMAT ${warn_format}
        INPUTS ${all_input}
        IMAGE_PATHS ${image_path_list}
        ALIASES ${aliases_list}
        TAG_FILE ${output_dir}/inviwo/inviwo.tag
        FILTER_PATTERNS ${filer_patterns_list}
        GENERATE_IMG
    )

    add_custom_target("DOXY-Clear"
        COMMAND ${CMAKE_COMMAND} -E remove_directory "${output_dir}/inviwo"
        WORKING_DIRECTORY ${output_dir}
        COMMENT "Clear the old documentation"
        VERBATIM
    )
    set_target_properties("DOXY-Clear" PROPERTIES FOLDER "doc" EXCLUDE_FROM_ALL TRUE)

    add_custom_target("DOXY-Inviwo"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${output_dir}/inviwo/html"
        COMMAND ${DOXYGEN_EXECUTABLE} "${output_dir}/inviwo.doxy"
        WORKING_DIRECTORY ${output_dir}
        COMMENT "Generating Inviwo API documentation with Doxygen"
        VERBATIM
    )
    set_target_properties("DOXY-Inviwo" PROPERTIES FOLDER "doc" EXCLUDE_FROM_ALL TRUE)
    add_dependencies("DOXY-ALL" "DOXY-Inviwo")

    if(${IVW_DOXYGEN_OPEN_HTML_AFTER_BUILD})
        if(WIN32)
            set(OPEN_COMMAND "start")
        elseif(APPLE)
            set(OPEN_COMMAND "open")
        else()
            set(OPEN_COMMAND "xdg-open")
        endif()
        add_custom_command(TARGET DOXY-Inviwo 
            POST_BUILD
            COMMAND ${OPEN_COMMAND} 
            ARGS "${output_dir}/inviwo/html/index.html"
        )
    endif()

    add_custom_target("DOXY-generate-processor-previews"
        COMMAND inviwo --save-previews "${output_dir}/inviwo/html" --quit
        WORKING_DIRECTORY ${output_dir}
        COMMENT "Generate preview images of processors to be used in Inviwo Doxygen API documentation"
        VERBATIM
    )
    add_dependencies("DOXY-generate-processor-previews" "DOXY-Clear")
    set_target_properties("DOXY-generate-processor-previews" 
                            PROPERTIES FOLDER "doc" EXCLUDE_FROM_ALL TRUE)

    add_dependencies("DOXY-Inviwo" "DOXY-generate-processor-previews" "DOXY-Clear")

    # Help, used for the help inside inviwo
    set(module_bases "")
    foreach(mod ${${modules_var}})
        if(${${mod}_opt}) # Only include enabled modules
            if(EXISTS "${${mod}_path}/docs/images")
                set(image_path "${${mod}_path}/docs/images")
            else()
                set(image_path "")
            endif()

             ivw_private_make_help(
                NAME ${${mod}_class}
                OUTPUT_DIR "${output_dir}/help"
                HTML_DIR "${ivw_doxy_dir}/help" 
                DOC_DIR "${${mod}_path}/docs"
                INPUTS "${${mod}_path}"
                IMAGE_PATHS "${image_path}"
                FILTER_PATTERNS ${filer_patterns_list}
                WARNING_FORMAT ${warn_format}
            )
            add_dependencies("DOXY-ALL" "DOXY-HELP-${${mod}_class}")
        endif()
    endforeach()
 endfunction()
