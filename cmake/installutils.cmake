#################################################################################
#
# Inviwo - Interactive Visualization Workshop
#
# Copyright (c) 2013-2019 Inviwo Foundation
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

# install related paths
if(APPLE)
    set(IVW_RUNTIME_INSTALL_DIR bin)
    set(IVW_BUNDLE_INSTALL_DIR .)
    set(IVW_LIBRARY_INSTALL_DIR Inviwo.app/Contents/MacOS)
    set(IVW_ARCHIVE_INSTALL_DIR ${IVW_LIBRARY_INSTALL_DIR})
    set(IVW_FRAMEWORK_INSTALL_DIR ${IVW_LIBRARY_INSTALL_DIR})
    set(IVW_INCLUDE_INSTALL_DIR include)
else()
    set(IVW_RUNTIME_INSTALL_DIR bin)
    set(IVW_BUNDLE_INSTALL_DIR "not used!!!")
    set(IVW_LIBRARY_INSTALL_DIR ${IVW_RUNTIME_INSTALL_DIR})
    set(IVW_ARCHIVE_INSTALL_DIR ${IVW_RUNTIME_INSTALL_DIR})
    set(IVW_FRAMEWORK_INSTALL_DIR "not used!!!")
    set(IVW_INCLUDE_INSTALL_DIR include)
endif()

#--------------------------------------------------------------------
# Add folder to module pack
macro(ivw_add_to_module_pack folder)
    if(APPLE)
        set(prefix "Inviwo.app/Contents/Resources/")
    else()
        set(prefix "")
    endif()
    get_filename_component(FOLDER_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    install(
        DIRECTORY ${folder}
        DESTINATION ${prefix}modules/${FOLDER_NAME}
        COMPONENT ${_cpackName}
    )
endmacro()

#--------------------------------------------------------------------
# A helper funtion to install targets
# usage: ivw_default_install_comp_targets(<cpack component> <list of targets)
function(ivw_default_install_comp_targets comp)
    # Dest type         Applies to
    #------------------------------------------------
    # ARCHIVE           Static libs, .lib
    # LIBRARY           Module libraries .so 
    # RUNTIME           Executables, DLLs
    # OBJECTS           Object libraries
    # FRAMEWORK         Targets marked as FRAMEWORK
    # BUNDLE            Targets marked as BUNDLE
    install(TARGETS ${ARGN}
            EXPORT "${ARGN}"
            RUNTIME DESTINATION ${IVW_RUNTIME_INSTALL_DIR}
            BUNDLE DESTINATION ${IVW_BUNDLE_INSTALL_DIR}
            ARCHIVE DESTINATION ${IVW_ARCHIVE_INSTALL_DIR}
            LIBRARY DESTINATION ${IVW_LIBRARY_INSTALL_DIR}
            FRAMEWORK DESTINATION ${IVW_FRAMEWORK_INSTALL_DIR}
            COMPONENT ${comp})

endfunction()

#--------------------------------------------------------------------
# A helper funtion to install module targets
function(ivw_default_install_targets)
    ivw_default_install_comp_targets(modules ${ARGN})
endfunction()

#--------------------------------------------------------------------
# Install files
function(ivw_private_install_module_dirs)
    if(APPLE)
        set(prefix "Inviwo.app/Contents/Resources/")
    else()
        set(prefix "")
    endif()
    get_filename_component(module_name ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    foreach(folder data docs tests/regression)
        set(dir ${CMAKE_CURRENT_SOURCE_DIR}/${folder})
        get_filename_component(base ${folder} DIRECTORY)
        if(EXISTS ${dir})
            install(
                DIRECTORY ${dir}
                DESTINATION ${prefix}modules/${module_name}/${base}
                COMPONENT modules
            )
        endif()
    endforeach()
endfunction()


#--------------------------------------------------------------------
# Adds special qt dependency and includes package variables to the project
macro(ivw_qt_add_to_install ivw_comp)
    foreach(qtarget ${ARGN})
        find_package(${qtarget} QUIET REQUIRED)
        if(${qtarget}_FOUND)
            if(WIN32)
                set(QTARGET_DIR "${${qtarget}_DIR}/../../../bin")
                install(FILES ${QTARGET_DIR}/${qtarget}${CMAKE_DEBUG_POSTFIX}.dll 
                        DESTINATION bin 
                        COMPONENT ${ivw_comp} 
                        CONFIGURATIONS Debug)
                install(FILES ${QTARGET_DIR}/${qtarget}.dll 
                        DESTINATION bin 
                        COMPONENT ${ivw_comp} 
                        CONFIGURATIONS Release RelWithDebInfo)
                foreach(plugin IN LISTS ${qtarget}_PLUGINS)
                    get_target_property(_loc ${plugin} LOCATION)
                    get_filename_component(_path ${_loc} PATH)
                    get_filename_component(_dirname ${_path} NAME)
                    install(FILES ${_loc} 
                            DESTINATION bin/${_dirname} 
                            COMPONENT ${ivw_comp})
                endforeach()
            elseif(APPLE)
                foreach(plugin IN LISTS ${qtarget}_PLUGINS)
                    get_target_property(_loc ${plugin} LOCATION)
                    get_filename_component(_path ${_loc} PATH)
                    get_filename_component(_dirname ${_path} NAME)
                    install(FILES ${_loc} 
                            DESTINATION Inviwo.app/Contents/plugins/${_dirname} 
                            COMPONENT ${ivw_comp})
                endforeach()
            endif()
        endif()
    endforeach()
endmacro()

function(ivw_register_package name)
    set(incdirs "")
    foreach(target ${ARGN})
        get_target_property(dirs ${target} INTERFACE_INCLUDE_DIRECTORIES)
        set(incdirs "${incdirs} ${dirs}")
    endforeach()
    file(WRITE "${CMAKE_BINARY_DIR}/pkg/Find${name}.cmake" 
         "# Fake Find file for ${name}\n"
         "set(${name}_FOUND ON)\n"
         "set(${name}_LIBRARIES ${ARGN})\n"
         "set(${name}_INCLUDE_DIRS ${incdirs})\n"
         )
 endfunction()


#--------------------------------------------------------------------
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
