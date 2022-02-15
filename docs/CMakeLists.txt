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
include(CMakeDependentOption)

option(IVW_DOXYGEN_PROJECT "Create Inviwo doxygen files" OFF)
cmake_dependent_option(IVW_DOXYGEN_OPEN_HTML_AFTER_BUILD
                       "Open the generated doxygen HTML when build is done"
                       OFF
                       "IVW_DOXYGEN_PROJECT"
                       OFF)

if(IVW_DOXYGEN_PROJECT)
    # sets: DOXYGEN_FOUND, DOXYGEN_EXECUTABLE, DOXYGEN_DOT_FOUND, DOXYGEN_DOT_EXECUTABLE
    find_package(Doxygen REQUIRED)
    find_package(Python3 COMPONENTS Interpreter REQUIRED)
    find_program(IVW_DOXYGEN_LATEX_PATH NAMES pdflatex pdflatex.exe latex latex.exe REQUIRED)
else()
    return()
endif()

if(MSVC AND IVW_DOXYGEN_PROJECT)
    message(STATUS
        "In Visual Studio the doxygen project will rerun every time you run\n"
        "\"ALL_BUILD\" even if it is up to date. Hence, you propbably want\n"
        "to disable generation of doxygen on each build this can be done\n"
        "by right clicking on the docs folder in the \"Solution Explorer\" and\n"
        "select \"Unload Project\".")
endif()

include(docutils.cmake)

ivw_retrieve_all_modules(reg_modules)
ivw_mod_name_to_mod_dep(modules ${reg_modules})

set(ivw_doxy_dir ${IVW_ROOT_DIR}/docs)
set(output_dir ${CMAKE_CURRENT_BINARY_DIR})
set(tag_files "")
set(dependency_list "")

set(filer_patterns_list
    "\"*.frag=${Python3_EXECUTABLE}${ivw_doxy_dir}/filter/glslfilter.py\""
    "\"*.vert=${Python3_EXECUTABLE}${ivw_doxy_dir}/filter/glslfilter.py\""
    "\"*.geom=${Python3_EXECUTABLE}${ivw_doxy_dir}/filter/glslfilter.py\""
    "\"*.glsl=${Python3_EXECUTABLE}${ivw_doxy_dir}/filter/glslfilter.py\""
    "\"*.comp=${Python3_EXECUTABLE}${ivw_doxy_dir}/filter/glslfilter.py\""
)

set(extra_files "${ivw_doxy_dir}/style/img_downArrow.png")
set(aliases_list
    "docpage{1}=\"\\ingroup processors\""
    "docpage{2}=\"\\ingroup processors\""
)
set(image_path_list
    "${IVW_ROOT_DIR}/data/help/images"
    "${IVW_ROOT_DIR}/docs/images"
)
set(all_input
    "${IVW_INCLUDE_DIR}"
    "${IVW_CORE_INCLUDE_DIR}"
    "${IVW_QT_INCLUDE_DIR}"
    "${IVW_APPLICATION_DIR}"
    "${IVW_ROOT_DIR}/docs"
)

set(file_patterns "*.hpp *.h *.cl *.dox")
set(glsl_file_patterns "*.comp *.frag *.vert *.geom *.glsl")
set(exclude_patterns "*.c *.cpp */moc_* */qrc_* */modules/*/ext/* */clogs/* *-test.cpp *sqlite3*")

if(MSVC)
    set(warn_format "\$file(\$line): \$text")
endif()

# Group target
add_custom_target(DOXY-ALL
    WORKING_DIRECTORY ${output_dir}
    COMMENT "Generating ALL API documentation with Doxygen"
    VERBATIM
)
set_target_properties("DOXY-ALL" PROPERTIES FOLDER "docs" EXCLUDE_FROM_ALL TRUE)

# Modules
set(module_bases "")
set(module_targets "")
foreach(mod IN LISTS modules)
    if(${${mod}_opt}) # Only include enabled modules
        list(APPEND all_input ${${mod}_incPath})
        list(APPEND module_targets ${${mod}_target})
        if(EXISTS "${${mod}_path}/docs/images")
            list(APPEND image_path_list "${${mod}_path}/docs/images")
        endif()
    endif()
endforeach()

# include paths
ivw_get_include_dirs(incpaths ${module_targets})
ivw_get_glsl_dirs(glslpaths ${modules})
# Inviwo
ivw_private_make_doxyfile(
    NAME "Inviwo"
    BRIEF "Inviwo documentation"
    DOXY_DIR "${ivw_doxy_dir}"
    OUTPUT_DIR "${output_dir}"
    WARNING_FORMAT ${warn_format}
    INPUTS ${all_input}
    INCLUDE_PATHS "${incpaths} ${glslpaths}"
    FILE_PATTERNS "${file_patterns} ${glsl_file_patterns}"
    EXCLUDE_PATTERNS ${exclude_patterns}
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
set_target_properties("DOXY-Clear" PROPERTIES FOLDER "docs" EXCLUDE_FROM_ALL TRUE)
add_custom_target("DOXY-Inviwo"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${output_dir}/inviwo/html"
    COMMAND ${DOXYGEN_EXECUTABLE} "${output_dir}/inviwo.doxy"
    WORKING_DIRECTORY ${output_dir}
    COMMENT "Generating Inviwo API documentation with Doxygen"
    VERBATIM
    # Copy over custom CSS. HTML_EXTRA_STYLESHEET does not override everything correctly.
    COMMAND ${CMAKE_COMMAND} -E copy "${ivw_doxy_dir}/style/doxygen.css" "${output_dir}/inviwo/html/doxygen.css"
    COMMAND ${CMAKE_COMMAND} -E copy "${ivw_doxy_dir}/style/navtree.css" "${output_dir}/inviwo/html/navtree.css"
    COMMAND ${CMAKE_COMMAND} -E copy "${ivw_doxy_dir}/style/search.css" "${output_dir}/inviwo/html/search.css"
    COMMAND ${CMAKE_COMMAND} -E copy "${ivw_doxy_dir}/style/tabs.css" "${output_dir}/inviwo/html/tabs.css"
    COMMAND ${CMAKE_COMMAND} -E copy "${ivw_doxy_dir}/style/Montserrat-Regular.ttf" "${output_dir}/inviwo/html/Montserrat-Regular.ttf"
    COMMAND ${CMAKE_COMMAND} -E copy "${ivw_doxy_dir}/style/Montserrat-Bold.ttf" "${output_dir}/inviwo/html/Montserrat-Bold.ttf"
    COMMAND ${CMAKE_COMMAND} -E copy "${ivw_doxy_dir}/style/Merriweather-Regular.ttf" "${output_dir}/inviwo/html/Merriweather-Regular.ttf"
)
set_target_properties("DOXY-Inviwo" PROPERTIES FOLDER "docs" EXCLUDE_FROM_ALL TRUE)
add_dependencies("DOXY-ALL" "DOXY-Inviwo")
if(IVW_DOXYGEN_OPEN_HTML_AFTER_BUILD)
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

string(TIMESTAMP YEAR "%Y") # used in conf.py.in
set(PYMODULES "") # used in index.rst.in
get_property(pymodules GLOBAL PROPERTY IVW_PYMODULE_LIST)
get_property(pyapps GLOBAL PROPERTY IVW_PYAPP_LIST)
foreach(module IN LISTS pymodules pyapps)
    get_target_property(name ${module} NAME)
    string(APPEND PYMODULES "    ${name}\n")
endforeach()

configure_file("${ivw_doxy_dir}/python-api/conf.py.in" ${output_dir}/python-api-in/conf.py @ONLY)
configure_file("${ivw_doxy_dir}/python-api/index.rst.in" ${output_dir}/python-api-in/index.rst @ONLY)
add_custom_target("DOXY-Python"
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${output_dir}/python-api-in/_generated ${output_dir}/python-api
    COMMAND ${CMAKE_COMMAND} -E env PYTHONPATH=$<TARGET_FILE_DIR:inviwopy> 
            ${Python3_EXECUTABLE} -m sphinx -a -b html ${output_dir}/python-api-in ${output_dir}/python-api
    WORKING_DIRECTORY ${output_dir}
    COMMENT "Generate Inviwo python API documentation with sphinx"
    VERBATIM
)

add_dependencies(DOXY-Python ${pymodules} ${pyapps})
set_target_properties("DOXY-Python" PROPERTIES FOLDER "docs" EXCLUDE_FROM_ALL TRUE)
add_dependencies("DOXY-ALL" "DOXY-Python")

add_custom_target("DOXY-generate-processor-previews"
    COMMAND inviwo --save-previews "${output_dir}/inviwo/html" --quit
    WORKING_DIRECTORY ${output_dir}
    COMMENT "Generate preview images of processors to be used in Inviwo Doxygen API documentation"
    VERBATIM
)
add_dependencies("DOXY-generate-processor-previews" "DOXY-Clear")
set_target_properties("DOXY-generate-processor-previews"
                       PROPERTIES FOLDER "docs" EXCLUDE_FROM_ALL TRUE)
add_dependencies("DOXY-Inviwo" "DOXY-generate-processor-previews")

# Help, used for the help inside inviwo
set(module_bases "")
foreach(mod IN LISTS modules)
    # Only include enabled modules, skip the core module
    if(${${mod}_opt} AND (NOT ${mod} STREQUAL "INVIWOCOREMODULE")) 
        ivw_set_if(COND EXISTS "${${mod}_path}/docs/images" 
                   TRUEVAL "${${mod}_path}/docs/images" 
                   FALSEVAL "" 
                   RETVAL image_path)

        ivw_get_include_dirs(incpaths ${${mod}_target})
        ivw_get_glsl_dirs(glslpaths ${mod})
        ivw_private_make_help(
            NAME ${${mod}_name}
            OUTPUT_DIR "${output_dir}/help"
            HTML_DIR "${ivw_doxy_dir}/help"
            DOC_DIR "${${mod}_path}/docs"
            DOXY_DIR "${ivw_doxy_dir}"
            INPUTS "${${mod}_path}"
            INCLUDE_PATHS ${incpaths} ${glslpaths}
            IMAGE_PATHS "${image_path}"
            FILTER_PATTERNS ${filer_patterns_list}
            WARNING_FORMAT ${warn_format}
        )
        add_dependencies("DOXY-ALL" "DOXY-HELP-${${mod}_name}")
    endif()
endforeach()
