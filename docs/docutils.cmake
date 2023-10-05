function(ivw_private_format_doxy_arg retval)
    string(REGEX REPLACE ";" " \\\\\n                         " result "${ARGN}")
    set(${retval} ${result} PARENT_SCOPE)
endfunction()

function(ivw_private_make_doxyfile)
    set(options
        GENERATE_IMG
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