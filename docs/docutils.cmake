function(ivw_private_format_doxy_arg retval)
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
        DOXY_DIR
        FILE_PATTERNS
        EXCLUDE_PATTERNS
    )
    set(multiValueArgs
        INCLUDE_PATHS
        INPUTS
        IMAGE_PATHS
        ALIASES
        INPUT_TAGS
        FILTER_PATTERNS
        ADDITIONAL_FLAGS
    )
    cmake_parse_arguments(PARSE_ARGV 0 ARG "${options}" "${oneValueArgs}" "${multiValueArgs}")

    ivw_private_format_doxy_arg(inputs ${ARG_INPUTS})
    ivw_private_format_doxy_arg(image_paths ${ARG_IMAGE_PATHS})
    ivw_private_format_doxy_arg(aliases ${ARG_ALIASES})
    ivw_private_format_doxy_arg(input_tags ${ARG_INPUT_TAGS})
    ivw_private_format_doxy_arg(filter_patterns ${ARG_FILTER_PATTERNS})
    ivw_private_format_doxy_arg(incpaths ${ARG_INCLUDE_PATHS})
    ivw_private_format_doxy_arg(example_paths ${IVW_EXTERNAL_MODULES})

    string(REGEX REPLACE ";" "\n" additional_flags "${ARG_ADDITIONAL_FLAGS}")
    string(TOLOWER ${ARG_NAME} name_lower)

    ivw_set_if(COND ARG_GENERATE_IMG TRUEVAL "YES" FALSEVAL "NO" RETVAL generate_images)
    ivw_set_if(COND ARG_GENERATE_QHP TRUEVAL "YES" FALSEVAL "NO" RETVAL generate_qhp)

    if(DOXYGEN_DOT_FOUND)
        get_filename_component(DOXYGEN_DOT_PATH ${DOXYGEN_DOT_EXECUTABLE} PATH)
    endif()

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
DISTRIBUTE_GROUP_DOC   = YES
SUBGROUPING            = NO
EXTRACT_LOCAL_CLASSES  = NO

TAB_SIZE               = 4

ALIASES                = ${aliases}
MARKDOWN_SUPPORT       = YES
BUILTIN_STL_SUPPORT    = YES

CASE_SENSE_NAMES       = YES

INPUT                  = ${ARG_DOXY_DIR}/markdown/Index.md \\
                         ${inputs}

ENABLE_PREPROCESSING   = YES
SEARCH_INCLUDES        = NO

IMAGE_PATH             = ${image_paths}

INCLUDE_PATH           = ${incpaths}

EXTENSION_MAPPING      = no_extension=C++ frag=C++ vert=C++ geom=C++ glsl=C++ comp=C++

FILE_PATTERNS          = ${ARG_FILE_PATTERNS}

RECURSIVE              = YES
EXCLUDE                =
EXCLUDE_PATTERNS       = ${ARG_EXCLUDE_PATTERNS}

EXCLUDE_SYMBOLS        = cl::* \\
                         TCLAP::* \\
                         clogs::* \\
                         glm::*

FILTER_PATTERNS        = ${filter_patterns}

USE_MDFILE_AS_MAINPAGE = ${ARG_DOXY_DIR}/markdown/Index.md

HTML_EXTRA_FILES       = 

EXAMPLE_PATH           = ${IVW_ROOT_DIR} \\
                         ${example_paths}

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

GENERATE_QHP           = ${generate_qhp}
QHP_NAMESPACE          = org.inviwo.${name_lower}
QHP_VIRTUAL_FOLDER     = doc

GENERATE_TREEVIEW      = YES
TREEVIEW_WIDTH         = 300

TAGFILES               = ${input_tags}
GENERATE_TAGFILE       = ${ARG_TAG_FILE}

ALLEXTERNALS           = NO
EXTERNAL_GROUPS        = YES
EXTERNAL_PAGES         = YES

HAVE_DOT               = ${DOXYGEN_DOT_FOUND}

CLASS_GRAPH            = ${generate_images}
COLLABORATION_GRAPH    = NO # To large to be useful most of the time, ${generate_images}
GROUP_GRAPHS           = ${generate_images}
UML_LOOK               = NO
UML_LIMIT_NUM_FIELDS   = 10
TEMPLATE_RELATIONS     = YES
INCLUDE_GRAPH          = NO # needs SEARCH_INCLUDES = YES, ${generate_images}
INCLUDED_BY_GRAPH      = NO # needs SEARCH_INCLUDES = YES, ${generate_images}

CALL_GRAPH             = NO
CALLER_GRAPH           = NO

GRAPHICAL_HIERARCHY    = NO
DIRECTORY_GRAPH        = NO
DOT_IMAGE_FORMAT       = png
INTERACTIVE_SVG        = ${generate_images}
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
LATEX_CMD_NAME         = \"${IVW_DOXYGEN_LATEX_PATH}\"

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
        DOXY_DIR
    )
    set(multiValueArgs
        INPUTS
        IMAGE_PATHS
        FILTER_PATTERNS
        INCLUDE_PATHS
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
    )

    ivw_private_make_doxyfile(
        NAME "${HARG_NAME}"
        BRIEF "Inviwo ${HARG_NAME} help"
        OUTPUT_DIR "${HARG_OUTPUT_DIR}"
        WARNING_FORMAT ${HARG_WARNING_FORMAT}
        INPUTS ${HARG_INPUTS}
        INCLUDE_PATHS ${HARG_INCLUDE_PATHS}
        IMAGE_PATHS ${HARG_IMAGE_PATHS}
        ALIASES ${aliases_list}
        FILTER_PATTERNS ${HARG_FILTER_PATTERNS}
        ADDITIONAL_FLAGS ${additional_flags_list}
        DOXY_DIR ${HARG_DOXY_DIR}
        GENERATE_QHP
    )

    find_program(IVW_DOXYGEN_QHELPGENERATOR "qhelpgenerator")

    add_custom_target("DOXY-HELP-${HARG_NAME}"
        COMMAND ${CMAKE_COMMAND} -E echo "Running Doxygen for ${HARG_NAME}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${HARG_OUTPUT_DIR}/${name_lower}/html"
        COMMAND ${DOXYGEN_EXECUTABLE} "${HARG_OUTPUT_DIR}/${name_lower}.doxy"

        COMMAND ${CMAKE_COMMAND} -DQHP_FILE="${HARG_OUTPUT_DIR}/${name_lower}/html/index.qhp"
                                 -DSTRIPPED_FILE="${HARG_OUTPUT_DIR}/${name_lower}/html/index-stripped.qhp"
                                 -P "${ivw_doxy_dir}/strip-qhp.cmake"

        COMMAND ${CMAKE_COMMAND} -E echo "Building ${name_lower}.qch"
        COMMAND ${CMAKE_COMMAND} -E echo "Running: ${IVW_DOXYGEN_QHELPGENERATOR} -o ${HARG_OUTPUT_DIR}/${name_lower}.qch \
                                 ${HARG_OUTPUT_DIR}/${name_lower}/html/index-stripped.qhp"
        COMMAND ${IVW_DOXYGEN_QHELPGENERATOR} "-o" "${HARG_OUTPUT_DIR}/${name_lower}.qch"
                                            "${HARG_OUTPUT_DIR}/${name_lower}/html/index-stripped.qhp"

        COMMAND ${CMAKE_COMMAND} -E make_directory "${HARG_DOC_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy "${HARG_OUTPUT_DIR}/${name_lower}.qch" "${HARG_DOC_DIR}/"

        WORKING_DIRECTORY ${HARG_OUTPUT_DIR}
        COMMENT "Building Qt QCH files for ${HARG_NAME}"
        VERBATIM
    )
    set_target_properties("DOXY-HELP-${HARG_NAME}" PROPERTIES FOLDER "docs/qch" EXCLUDE_FROM_ALL TRUE)
endfunction()

function(ivw_get_glsl_dirs retval)
    set(glslpaths "")
    foreach(mod IN LISTS ARGN)
        if(${${mod}_opt})
            if(EXISTS "${${mod}_path}/glsl")
                list(APPEND glslpaths "\"${${mod}_path}/glsl\"")
            endif()
            ivw_mod_name_to_mod_dep(depmods ${${mod}_dependencies})
            ivw_get_glsl_dirs(deps ${depmods})
            list(APPEND glslpaths ${deps})
        endif()
    endforeach()
    list(REMOVE_DUPLICATES glslpaths)
    list(SORT glslpaths)
    set(${retval} ${glslpaths} PARENT_SCOPE)
endfunction()

function(ivw_get_include_dirs retval)
    set(inc_dirs "")
    foreach(target IN LISTS ARGN)
        ivw_get_target_property_recursive(dirs ${target} INTERFACE_INCLUDE_DIRECTORIES True)
        list(APPEND inc_dirs ${dirs})
        ivw_get_target_property_recursive(dirs ${target} INCLUDE_DIRECTORIES False)
        list(APPEND inc_dirs ${dirs})
        list(REMOVE_DUPLICATES inc_dirs)
    endforeach()
    
    # filter generator expr
    set(inc_dirs_nogen "")
    foreach(item IN LISTS inc_dirs)
        string(REGEX MATCH [=[^\$<BUILD_INTERFACE:([^$<>]+)>$]=] repl ${item})
        if(CMAKE_MATCH_1)
            list(APPEND inc_dirs_nogen "${CMAKE_MATCH_1}")
        endif()
        string(REGEX MATCH [=[^[^$<>]*$]=] repl ${item})
        if(CMAKE_MATCH_0)
            list(APPEND inc_dirs_nogen "${CMAKE_MATCH_0}")
        endif()
    endforeach()
    list(REMOVE_DUPLICATES inc_dirs_nogen)
    
    # filter non existing folders
    set(inc_dirs_exist "")
    foreach(item IN LISTS inc_dirs_nogen)
        if(EXISTS ${item})
            get_filename_component(real ${item} REALPATH)
            list(APPEND inc_dirs_exist "\"${real}\"")
        endif()
    endforeach()
     list(REMOVE_DUPLICATES inc_dirs_exist)
     list(SORT inc_dirs_exist)

    set(${retval} ${inc_dirs_exist} PARENT_SCOPE)
endfunction()