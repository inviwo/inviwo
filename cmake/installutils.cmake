#################################################################################
#
# Inviwo - Interactive Visualization Workshop
#
# Copyright (c) 2013-2020 Inviwo Foundation
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

# Components
# Application   All the executables and dll etc needed to run the app
# Development   Lib files, pdb files, headers, 
# Datasets      General datasets i.e. boron etc. Workspaces, scripts, transfer functions.    
# Testing       Regression files and data sets
 
# install related paths
if(APPLE)
# See 
# https://developer.apple.com/library/archive/documentation/CoreFoundation/Conceptual/CFBundles/BundleTypes/BundleTypes.html#//apple_ref/doc/uid/10000123i-CH101-SW1
	# Only show this "advanced" setting when packaging
	set(IVW_APP_INSTALL_NAME 
		"Inviwo" CACHE STRING "Application bundle name. 
		 Override if you are packaging a custom application. 
		 Installed libraries and modules 
		 will be placed inside bundle <name>.app")
    set(IVW_RUNTIME_INSTALL_DIR ${IVW_APP_INSTALL_NAME}.app/Contents/MacOS)
    set(IVW_BUNDLE_INSTALL_DIR .)
    set(IVW_LIBRARY_INSTALL_DIR ${IVW_APP_INSTALL_NAME}.app/Contents/MacOS)
    set(IVW_ARCHIVE_INSTALL_DIR ${IVW_LIBRARY_INSTALL_DIR})
    set(IVW_FRAMEWORK_INSTALL_DIR ${IVW_APP_INSTALL_NAME}.app/Contents/Frameworks)
    set(IVW_INCLUDE_INSTALL_DIR include)
    set(IVW_RESOURCE_INSTALL_PREFIX ${IVW_APP_INSTALL_NAME}.app/Contents/Resources/)
elseif(WIN32)
    set(IVW_RUNTIME_INSTALL_DIR bin)
    set(IVW_BUNDLE_INSTALL_DIR "not used!!!")
    set(IVW_LIBRARY_INSTALL_DIR ${IVW_RUNTIME_INSTALL_DIR})
    set(IVW_ARCHIVE_INSTALL_DIR ${IVW_RUNTIME_INSTALL_DIR})
    set(IVW_FRAMEWORK_INSTALL_DIR "not used!!!")
    set(IVW_INCLUDE_INSTALL_DIR include)
    set(IVW_RESOURCE_INSTALL_PREFIX "")
else()
    set(IVW_RUNTIME_INSTALL_DIR bin)
    set(IVW_BUNDLE_INSTALL_DIR "not used!!!")
    set(IVW_LIBRARY_INSTALL_DIR lib)
    set(IVW_ARCHIVE_INSTALL_DIR lib)
    set(IVW_FRAMEWORK_INSTALL_DIR "not used!!!")
    set(IVW_INCLUDE_INSTALL_DIR include)
    set(IVW_RESOURCE_INSTALL_PREFIX "")
endif()

set(IVW_PACKAGE_SELECT_APP "inviwo" CACHE STRING "Select which app to package")
set_property(CACHE IVW_PACKAGE_SELECT_APP PROPERTY STRINGS "inviwo")

# the INTERFACE_IVW_INSTALL_LIST is a property to hold a list of projects/components to install
# it is used in the packaging to creata a list of items to install. this list will be
# passed to CPACK_INSTALL_CMAKE_PROJECTS after a potential filtering
# The property can be set in GLOBAL, DIRECTORY and TARGET scope.
define_property(
    GLOBAL PROPERTY INTERFACE_IVW_INSTALL_LIST INHERITED
    BRIEF_DOCS "List of global installation components to install"
    FULL_DOCS "List of global installation components to install"
)
define_property(
    DIRECTORY PROPERTY INTERFACE_IVW_INSTALL_LIST INHERITED
    BRIEF_DOCS "List of installation components to install for directory"
    FULL_DOCS "List of installation components to install for directory"
)
define_property(
    TARGET PROPERTY INTERFACE_IVW_INSTALL_LIST INHERITED
    BRIEF_DOCS "List of installation components to install for target"
    FULL_DOCS "List of installation components to install for target"
)

# Adds the current project to the INTERFACE_IVW_INSTALL_LIST of the given scope
function(ivw_append_install_list)
    set(options DIRECTORY GLOBAL)
    set(oneValueArgs TARGET)
    set(multiValueArgs "")
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(ARG_TARGET) 
        set(scope TARGET ${ARG_TARGET})
    elseif(ARG_DIRECTORY)
        set(scope DIRECTORY)
    elseif(ARG_GLOBAL)
        set(scope GOBAL)
    else()
        message(ERROR "Error either TARGET, DIRECTORY, or GLOBAL must be specified")
    endif()

    get_property(install_list ${scope} PROPERTY INTERFACE_IVW_INSTALL_LIST)
    if(NOT install_list) 
        set(install_list "")
    endif()
    list(APPEND install_list 
        "${CMAKE_CURRENT_BINARY_DIR}|%|${CMAKE_PROJECT_NAME}|%|Application|%|/"
        "${CMAKE_CURRENT_BINARY_DIR}|%|${CMAKE_PROJECT_NAME}|%|Datasets|%|/"
        "${CMAKE_CURRENT_BINARY_DIR}|%|${CMAKE_PROJECT_NAME}|%|Testing|%|/"
        "${CMAKE_CURRENT_BINARY_DIR}|%|${CMAKE_PROJECT_NAME}|%|Development|%|/"
    )
    set_property(${scope} PROPERTY INTERFACE_IVW_INSTALL_LIST ${install_list})
endfunction()

# Filter the given INTERFACE_IVW_INSTALL_LIST for specified components
# List should be creted by calls to ivw_append_install_list
# see apps/inviwo/packaging.cmake for an example use
# Params:
#  * LIST an instace of INTERFACE_IVW_INSTALL_LIST to filter
#  * REMOVE_COMPONENTS list of components to remove from the given list
function(ivw_filter_install_list)
    set(options "")
    set(oneValueArgs LIST)
    set(multiValueArgs REMOVE_COMPONENTS)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(list ${${ARG_LIST}})
    foreach(item IN LISTS ARG_REMOVE_COMPONENTS)
        list(FILTER list EXCLUDE REGEX ".*\\|%\\|${item}\\|%\\|.*")
    endforeach()
    set(${ARG_LIST} ${list} PARENT_SCOPE)
endfunction()

# Add folder to module pack
macro(ivw_add_to_module_pack folder)
    get_filename_component(module ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    install(
        DIRECTORY ${folder}
        DESTINATION ${IVW_RESOURCE_INSTALL_PREFIX}modules/${module}
        COMPONENT Application
    )
endmacro()

# A helper funtion to install targets.  deprecated
function(ivw_default_install_comp_targets comp)
    ivw_default_install_targets(${ARGN})
endfunction()

# A helper funtion to install module targets
function(ivw_default_install_targets)
    foreach(target IN LISTS ARGN)
        install(TARGETS ${target} EXPORT "${target}"
            RUNTIME        # DLLs, Exes, 
                COMPONENT Application 
                DESTINATION ${IVW_RUNTIME_INSTALL_DIR}
            BUNDLE         # Targets marked as BUNDLE
                COMPONENT Application
                DESTINATION ${IVW_BUNDLE_INSTALL_DIR}
            ARCHIVE        # Static libs, .libs 
                COMPONENT Development 
                DESTINATION ${IVW_ARCHIVE_INSTALL_DIR}
            LIBRARY        # Shared libs - DLLs   
                COMPONENT Application
                DESTINATION ${IVW_LIBRARY_INSTALL_DIR}
            FRAMEWORK      # Targets marked as FRAMEWORK
                COMPONENT Application
                DESTINATION ${IVW_FRAMEWORK_INSTALL_DIR}
            PUBLIC_HEADER  # Public headers
                COMPONENT Development 
                DESTINATION ${IVW_INCLUDE_INSTALL_DIR}
            RESOURCE       # Resource files
                COMPONENT Application
                DESTINATION ${IVW_RESOURCE_INSTALL_PREFIX}
        )
        ivw_append_install_list(TARGET ${target})
    endforeach()
endfunction()

# Install files
function(ivw_private_install_module_dirs)
    get_filename_component(module ${CMAKE_CURRENT_SOURCE_DIR} NAME)

    install(
        DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data
        DESTINATION ${IVW_RESOURCE_INSTALL_PREFIX}modules/${module}/data
        COMPONENT Datasets
        OPTIONAL
    )
    install(
        DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/docs
        DESTINATION ${IVW_RESOURCE_INSTALL_PREFIX}modules/${module}/docs
        COMPONENT Application
        OPTIONAL
    )
    install(
        DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests
        DESTINATION ${IVW_RESOURCE_INSTALL_PREFIX}modules/${module}/tests
        COMPONENT Testing
        OPTIONAL
    )
    list(APPEND ivw_install_list
        "${CMAKE_CURRENT_BINARY_DIR};${CMAKE_PROJECT_NAME};Application;/"
        "${CMAKE_CURRENT_BINARY_DIR};${CMAKE_PROJECT_NAME};Datasets;/"
        "${CMAKE_CURRENT_BINARY_DIR};${CMAKE_PROJECT_NAME};Testing;/"
    )
endfunction()


# Adds special qt dependency and includes package variables to the project
macro(ivw_qt_add_to_install ivw_comp)
    foreach(qtarget ${ARGN})
        find_package(${qtarget} QUIET REQUIRED)
        if(${qtarget}_FOUND)
            if(WIN32)
                set(QTARGET_DIR "${${qtarget}_DIR}/../../../bin")
                install(FILES ${QTARGET_DIR}/${qtarget}${CMAKE_DEBUG_POSTFIX}.dll 
                        DESTINATION bin 
                        COMPONENT Application 
                        CONFIGURATIONS Debug)
                install(FILES ${QTARGET_DIR}/${qtarget}.dll 
                        DESTINATION bin 
                        COMPONENT Application 
                        CONFIGURATIONS Release RelWithDebInfo)
                foreach(plugin IN LISTS ${qtarget}_PLUGINS)
                    get_target_property(_loc ${plugin} LOCATION)
                    get_filename_component(_path ${_loc} PATH)
                    get_filename_component(_dirname ${_path} NAME)
                    install(FILES ${_loc} 
                            DESTINATION bin/${_dirname} 
                            COMPONENT Application)
                endforeach()
            elseif(APPLE)
                foreach(plugin IN LISTS ${qtarget}_PLUGINS)
                    get_target_property(_loc ${plugin} LOCATION)
                    get_filename_component(_path ${_loc} PATH)
                    get_filename_component(_dirname ${_path} NAME)
                    install(FILES ${_loc} 
                            DESTINATION ${IVW_APP_INSTALL_NAME}.app/Contents/plugins/${_dirname} 
                            COMPONENT Application)
                endforeach()
            endif()
        endif()
    endforeach()
    list(APPEND ivw_install_list
        "${CMAKE_CURRENT_BINARY_DIR};${CMAKE_PROJECT_NAME};Application;/"
    )
endmacro()

function(ivw_register_package name)
    set(incdirs "")
    foreach(target ${ARGN})
        get_target_property(dirs ${target} INTERFACE_INCLUDE_DIRECTORIES)
        if(dirs)
            list(APPEND incdirs ${dirs})
        endif()
    endforeach()
    string(TOLOWER "${name}" lowercase_name)
    string(TOUPPER "${name}" uppercase_name)
    set(${name}_DIR "${CMAKE_BINARY_DIR}/pkg/${lowercase_name}" CACHE PATH "" FORCE)
    
    string(REPLACE ";" " " incdirs "${incdirs}")
    string(REPLACE ";" " " libs "${ARGN}")
    set(contents 
        "set(${name}_FOUND ON)\n"
        "set(${name}_LIBRARIES ${libs})\n"
        "set(${name}_LIBRARY ${libs})\n"
        "set(${name}_INCLUDE_DIR ${incdirs})\n"
        "set(${name}_INCLUDE_DIRS ${incdirs})\n"
    )
    if(NOT name STREQUAL uppercase_name)
        list(APPEND contents
            "set(${uppercase_name}_FOUND ON)\n"
            "set(${uppercase_name}_INCLUDE_DIRS ${incdirs})\n"
            "set(${uppercase_name}_INCLUDE_DIR ${incdirs})\n"
            "set(${uppercase_name}_LIBRARIES ${libs})\n"
            "set(${uppercase_name}_LIBRARY ${libs})\n"
        )
    endif()

    file(WRITE "${CMAKE_BINARY_DIR}/pkg/${lowercase_name}/${name}Config.cmake" ${contents})
 endfunction()

# Make package for target(s) (with configure file etc) 
# usage: ivw_make_package(<name> <list of targets>)
function(ivw_make_package package_name)
    ivw_register_package(${package_name} ${ARGN})
 # Will uncomment in the future, this is for when we want to ship 
 # a version that also includes header files, so one can build modules 
 # without having to build all of inviwo. So far this is just work in progress
 # https://gitlab.kitware.com/cmake/community/wikis/doc/tutorials/How-to-create-a-ProjectConfig.cmake-file
#    include(CMakePackageConfigHelpers)

#    get_target_property(type ${target} TYPE)
#    if (${type} STREQUAL "INTERFACE_LIBRARY")
#        # VERSION property does not exist
#        set(${target}_VERSION 1.0.0)
#    else()
#        get_target_property(${target}_VERSION ${target} VERSION)
#        if (${target}_VERSION STREQUAL "${target}_VERSION-NOTFOUND")
#            set(${target}_VERSION 1.0.0)
#        endif()
#    endif()

#    write_basic_package_version_file(
#        "${CMAKE_CURRENT_BINARY_DIR}/cmake/${package_name}ConfigVersion.cmake"
#        VERSION ${${target}_VERSION}
#        COMPATIBILITY SameMajorVersion
#    )

#    export(EXPORT ${target}
#        FILE "${CMAKE_CURRENT_BINARY_DIR}/cmake/${package_name}Targets.cmake"
#        NAMESPACE ${target}::
#    )
#    # Export the package for use from the build-tree
#    # (this registers the build-tree with a global CMake-registry)
#    export(PACKAGE ${package_name})


#    if(EXISTS ${package_name}-config.cmake)
#        configure_file(${package_name}-config.cmake
#            "${CMAKE_CURRENT_BINARY_DIR}/cmake/${package_name}Config.cmake"
#            COPYONLY
#        )
#    else()
#        set(NAME ${target})
#        configure_file(${IVW_CMAKE_TEMPLATES}/template-config.cmake
#            "${CMAKE_CURRENT_BINARY_DIR}/cmake/${package_name}Config.cmake"
#            @ONLY
#        )
#    endif()
 
#    set(ConfigPackageLocation lib/cmake/${target})
#    install(EXPORT ${target}
#        FILE ${package_name}ConfigVersion.cmake
#        DESTINATION ${ConfigPackageLocation}
#    )
#    install(EXPORT ${target}
#        FILE ${package_name}Config.cmake
#        NAMESPACE ${package_name}::
#        DESTINATION ${ConfigPackageLocation}
#    )
# #   install(
# #       FILES
# #           "${CMAKE_CURRENT_BINARY_DIR}/cmake/${target}-config.cmake"
# #           "${CMAKE_CURRENT_BINARY_DIR}/cmake/${target}-config-version.cmake"
# #       DESTINATION
# #           ${ConfigPackageLocation}
# #       COMPONENT
# #           Devel
# #   )
endfunction()
