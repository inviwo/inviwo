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

#--------------------------------------------------------------------
# Add folder to module pack
macro(ivw_add_to_module_pack folder)
    if(IVW_PACKAGE_PROJECT)
        get_filename_component(FOLDER_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
        if(APPLE)
            install(DIRECTORY ${folder}
                     DESTINATION Inviwo.app/Contents/Resources/modules/${FOLDER_NAME}
                     COMPONENT ${_cpackName})
        else()
            install(DIRECTORY ${folder}
                     DESTINATION modules/${FOLDER_NAME}
                     COMPONENT ${_cpackName})
        endif()
    endif()
endmacro()

#--------------------------------------------------------------------
# A helper funtion to install targets
# usage: ivw_default_install_comp_targets(<cpack component> <list of targets)
function(ivw_default_install_comp_targets comp)
    if(NOT IVW_PACKAGE_PROJECT)
        return()
    endif()

    # Dest type         Applies to
    #------------------------------------------------
    # ARCHIVE           Static libs, .lib
    # LIBRARY           Module libraries .so 
    # RUNTIME           Executables, DLLs
    # OBJECTS           Object libraries
    # FRAMEWORK         Targets marked as FRAMEWORK
    # BUNDLE            Targets marked as BUNDLE

    if(WIN32)
        install(TARGETS ${ARGN} 
                EXPORT "${ARGN}-targets"
                RUNTIME DESTINATION bin
                ARCHIVE DESTINATION bin
                LIBRARY DESTINATION bin
                COMPONENT ${comp})
    elseif(APPLE)
        install(TARGETS ${ARGN}
                RUNTIME DESTINATION bin
                BUNDLE DESTINATION .
                ARCHIVE DESTINATION Inviwo.app/Contents/MacOS
                LIBRARY DESTINATION Inviwo.app/Contents/MacOS
                COMPONENT ${comp})
    else()
        install(TARGETS ${ARGN}
                RUNTIME DESTINATION bin
                ARCHIVE DESTINATION lib
                LIBRARY DESTINATION lib
                COMPONENT ${comp})
    endif()
endfunction()

#--------------------------------------------------------------------
# A helper funtion to install module targets
function(ivw_default_install_targets)
    ivw_default_install_comp_targets(modules ${ARGN})
endfunction()

#--------------------------------------------------------------------
# Install files
function(ivw_private_install_module_dirs)
    if(IVW_PACKAGE_PROJECT) 
        get_filename_component(module_name ${CMAKE_CURRENT_SOURCE_DIR} NAME)
        foreach(folder data docs)
            set(dir ${CMAKE_CURRENT_SOURCE_DIR}/${folder})
            if(EXISTS ${dir})
                if(APPLE)
                    install(DIRECTORY ${dir}
                            DESTINATION Inviwo.app/Contents/Resources/modules/${module_name}
                            COMPONENT modules)
                else()
                    install(DIRECTORY ${dir}
                            DESTINATION modules/${module_name}
                            COMPONENT modules)
                endif()
            endif()
        endforeach()
    endif()
endfunction()


#--------------------------------------------------------------------
# Adds special qt dependency and includes package variables to the project
macro(ivw_qt_add_to_install ivw_comp)
    foreach(qtarget ${ARGN})
        find_package(${qtarget} QUIET REQUIRED)
        if(IVW_PACKAGE_PROJECT)
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
                    foreach(plugin ${${qtarget}_PLUGINS})
                        get_target_property(_loc ${plugin} LOCATION)
                        get_filename_component(_path ${_loc} PATH)
                        get_filename_component(_dirname ${_path} NAME)
                        install(FILES ${_loc} 
                                DESTINATION bin/${_dirname} 
                                COMPONENT ${ivw_comp})
                    endforeach()
                elseif(APPLE)
                    foreach(plugin ${${qtarget}_PLUGINS})
                        get_target_property(_loc ${plugin} LOCATION)
                        get_filename_component(_path ${_loc} PATH)
                        get_filename_component(_dirname ${_path} NAME)
                        install(FILES ${_loc} 
                                DESTINATION Inviwo.app/Contents/plugins/${_dirname} 
                                COMPONENT ${ivw_comp})
                    endforeach()
                endif()
            endif()
        endif()
    endforeach()
endmacro()


#--------------------------------------------------------------------
# Make package (with configure file etc)
macro(ivw_make_package package_name target)
 # Will uncomment in the future, this is for when we want to ship 
 # a version that also includes header files, so one can build modules 
 # without having to build all of inviwo. So far this is just work in progress
 #   include(CMakePackageConfigHelpers)

 #   set(${target}_VERSION 1.0.0)

 #   write_basic_package_version_file(
 #       "${CMAKE_CURRENT_BINARY_DIR}/cmake/${target}-config-version.cmake"
 #       VERSION ${target}_VERSION
 #       COMPATIBILITY AnyNewerVersion
 #   )

 #   export(EXPORT ${target}-targets
 #       FILE "${CMAKE_CURRENT_BINARY_DIR}/cmake/${target}-targets.cmake"
 #       NAMESPACE ${target}::
 #   )

 #   if(EXISTS ${package_name}-config.cmake)
 #       configure_file(${package_name}-config.cmake
 #           "${CMAKE_CURRENT_BINARY_DIR}/cmake/${target}-config.cmake"
 #           COPYONLY
 #       )
 #   else()
 #       set(NAME ${target})
 #       configure_file(${IVW_CMAKE_TEMPLATES}/template-config.cmake
 #           "${CMAKE_CURRENT_BINARY_DIR}/cmake/${target}-config.cmake"
 #           @ONLY
 #       )
 #   endif()
 
 #   set(ConfigPackageLocation lib/cmake/${target})
 #   install(EXPORT ${target}-targets
 #       FILE 
 #           ${target}-targets.cmake
 #       NAMESPACE 
 #           ${target}::
 #       DESTINATION 
 #           ${ConfigPackageLocation}
 #   )
 #   install(
 #       FILES
 #           "${CMAKE_CURRENT_BINARY_DIR}/cmake/${target}-config.cmake"
 #           "${CMAKE_CURRENT_BINARY_DIR}/cmake/${target}-config-version.cmake"
 #       DESTINATION
 #           ${ConfigPackageLocation}
 #       COMPONENT
 #           Devel
 #   )
endmacro()
