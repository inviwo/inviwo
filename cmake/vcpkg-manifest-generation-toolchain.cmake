
# Creates a new vcpkg in  ${CMAKE_BINARY_DIR}/vcpkg.json by
# looking at the main inviwo vcpkg.json and any deps.json files in 
# IVW_EXTERNAL_MODULES. 

include_guard(GLOBAL)

if(EXISTS ${CMAKE_SOURCE_DIR}/vcpkg.json)
    file(READ ${CMAKE_SOURCE_DIR}/vcpkg.json main_manifest)

    # We need to create absolute paths for all overlays since we will move the manifest file
    string(JSON overlay_count ERROR_VARIABLE error 
        LENGTH ${main_manifest} "vcpkg-configuration" "overlay-ports"
    )
    if(overlay_count)
        set(i 0)
        while(i LESS overlay_count)
            string(JSON overlay ERROR_VARIABLE error 
                GET ${main_manifest} "vcpkg-configuration" "overlay-ports" ${i}
            )
            if(error)
                message(FATAL_ERROR "${error}")
            endif()
            if(NOT IS_ABSOLUTE overlay)
                file(REAL_PATH ${overlay} overlayAbs BASE_DIRECTORY ${CMAKE_SOURCE_DIR})
                string(JSON main_manifest ERROR_VARIABLE error 
                    SET ${main_manifest} "vcpkg-configuration" "overlay-ports" ${i} 
                        "\"${overlayAbs}\""
                )
                if(error)
                    message(FATAL_ERROR "${error}")
                endif()
            endif()
            math(EXPR i "${i} + 1")
        endwhile()
    endif()

    string(JSON count LENGTH ${main_manifest} dependencies)

    # add the dependencies and overlays of all the external modules
    foreach(item IN LISTS IVW_EXTERNAL_MODULES)
        message(STATUS "Adding external module: ${item}")
        if(EXISTS ${item}/deps.json)
            file(READ ${item}/deps.json module_manifest)

            # add any overlay and make it an absolute path
            string(JSON overlay_count ERROR_VARIABLE error 
                LENGTH ${module_manifest} "overlay-ports"
            )
            if(overlay_count)
                set(i 0)
                while(i LESS len)
                    string(JSON overlay ERROR_VARIABLE error 
                        GET ${module_manifest} "overlay-ports" ${i}
                    )
                    if(error)
                        message(FATAL_ERROR "${error}")
                    endif()
                    if(NOT IS_ABSOLUTE overlay)
                        file(REAL_PATH ${overlay} overlay BASE_DIRECTORY ${CMAKE_SOURCE_DIR})
                    endif()
                    math(EXPR overlay_count "${overlay_count} + 1")
                    string(JSON main_manifest ERROR_VARIABLE error 
                        SET ${main_manifest} "vcpkg-configuration" "overlay-ports" ${overlay_count}
                            "\"${overlay}\""
                    )
                    if(error)
                        message(FATAL_ERROR "${error}")
                    endif()
                    math(EXPR i "${i} + 1")
                endwhile()
            endif()

            # Copy all the dependencies 
            string(JSON len ERROR_VARIABLE error 
                LENGTH ${module_manifest} dependencies
            )
            if(error)
                message(FATAL_ERROR "${error}")
            endif()

            set(i 0)
            while(i LESS len)
                string(JSON dep ERROR_VARIABLE error 
                    GET ${module_manifest} dependencies ${i}
                )
                if(error)
                    message(FATAL_ERROR "${dep}: ${error}")
                endif()
                string(JSON depType ERROR_VARIABLE error 
                    TYPE ${module_manifest} dependencies ${i}
                )
                if(error)
                    message(FATAL_ERROR "${depType}; ${error}")
                endif()

                math(EXPR count "${count} + 1")

                if(depType STREQUAL "OBJECT")
                    string(JSON main_manifest ERROR_VARIABLE error 
                        SET ${main_manifest} dependencies ${count} "${dep}"
                    )
                    if(error)
                        message(FATAL_ERROR "${error}")
                    endif()
                else()
                    string(JSON main_manifest ERROR_VARIABLE error 
                        SET ${main_manifest} dependencies ${count} "\"${dep}\""
                    )
                    if(error)
                        message(FATAL_ERROR "${error}")
                    endif()
                endif()

                math(EXPR i "${i} + 1")
            endwhile()
        endif()
    endforeach()

    file(WRITE ${CMAKE_BINARY_DIR}/vcpkg.json ${main_manifest})

    set(ivw_vcpkg_override_manifest_dir ${CMAKE_BINARY_DIR})
endif()

# load the vcpkg toolchain file, pointing it to our new manifest
set(VCPKG_MANIFEST_DIR ${ivw_vcpkg_override_manifest_dir})
include($ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)
