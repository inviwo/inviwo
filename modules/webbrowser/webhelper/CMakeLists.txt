# Process running CEF browser
set(WEBBROWSER_HELPER_SOURCES
    app_switches.h
    app_switches.cpp
    webotherapp.h
    webotherapp.cpp
    webrendererapp.h
    webrendererapp.cpp
    cefprocesshelper.cpp
)

set(WEBBROWSER_RESOURCES_WINDOWS
  inviwo_web_helper.exe.manifest # Resolves "Check failed: fallback_available" in debug mode
)
set(WEBBROWSER_RESOURCES_MACOSX
    resources/mac/helper-Info.plist
)
APPEND_PLATFORM_SOURCES(WEBBROWSER_RESOURCES)
source_group(WebBrowser\\\\resources FILES ${WEBBROWSER_RESOURCES})

set(WEBBROWSER_HELPER_SOURCES
    ${WEBBROWSER_HELPER_SOURCES}
    ${WEBBROWSER_RESOURCES}
)

# place helper in separate executable. 
set(CEF_HELPER_TARGET "cef_inviwo_helper" CACHE INTERNAL "CEF_HELPER_TARGET")

find_package(warn CONFIG REQUIRED)

if (NOT OS_MAC)
    add_executable(${CEF_HELPER_TARGET} WIN32 MACOSX_BUNDLE ${WEBBROWSER_HELPER_SOURCES})
	set(CEF_HELPER_OUTPUT_NAME "cef_web_helper")
	set_target_properties(${CEF_HELPER_TARGET} PROPERTIES OUTPUT_NAME ${CEF_HELPER_OUTPUT_NAME})

    add_dependencies(${CEF_HELPER_TARGET} libcef_dll_wrapper)
    # Build helper before module so that we can copy it
    add_dependencies(inviwo-module-webbrowser ${CEF_HELPER_TARGET})

    target_link_libraries(${CEF_HELPER_TARGET} PRIVATE
        $<$<NOT:$<BOOL:${OS_MAC}>>:libcef_lib>
        libcef_dll_wrapper
        ${CEF_STANDARD_LIBS}
        warn::warn
    )
    target_include_directories(${CEF_HELPER_TARGET} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        "${CEF_INCLUDE_PATH}"
    )

    # Use inviwo preprocessor definitions
    ivw_define_standard_definitions(WebBrowser ${CEF_HELPER_TARGET})
    ivw_define_standard_properties(${CEF_HELPER_TARGET})
    ivw_suppress_compiler_warnings(${CEF_HELPER_TARGET})
    target_compile_definitions(${CEF_HELPER_TARGET} PRIVATE
        $<$<BOOL:${USE_SANDBOX}>:CEF_USE_SANDBOX>
    )

    # Put in ext folder in developer IDE
    ivw_folder(${CEF_HELPER_TARGET} ext/CEF)
endif()

if (OS_MAC)
	# Mac: CEF uses 3 different applications according to following structure:
	# "<app name> + Helper<type>", e.g. Inviwo Helper (GPU)
	set(CEF_HELPER_OUTPUT_NAME "${IVW_APP_INSTALL_NAME} Helper")
	set(CEF_TARGET inviwo-module-webbrowser)
    
    if(USE_SANDBOX)
        # Logical target used to link the cef_sandbox library.
        ADD_LOGICAL_TARGET("cef_sandbox_lib" "${CEF_SANDBOX_LIB_DEBUG}" "${CEF_SANDBOX_LIB_RELEASE}")
    endif()

    # Create the multiple Helper app bundle targets.
    foreach(_suffix_list ${CEF_HELPER_APP_SUFFIXES})
        # Convert to a list and extract the suffix values.
        string(REPLACE ":" ";" _suffix_list ${_suffix_list})
        list(GET _suffix_list 0 _name_suffix)
        list(GET _suffix_list 1 _target_suffix)
        list(GET _suffix_list 2 _plist_suffix)

        # Define Helper target and output names.
        set(_helper_target "${CEF_HELPER_TARGET}${_target_suffix}")
        set(_helper_output_name "${CEF_HELPER_OUTPUT_NAME}${_name_suffix}")

        # Create Helper-specific variants of the helper-Info.plist file. Do this
        # manually because the configure_file command (which is executed as part of
        # MACOSX_BUNDLE_INFO_PLIST) uses global env variables and would insert the
        # wrong values with multiple targets.
        set(_helper_info_plist "${CMAKE_CURRENT_BINARY_DIR}/helper-Info${_target_suffix}.plist")
        file(READ "${CMAKE_CURRENT_SOURCE_DIR}/resources/mac/helper-Info.plist" _plist_contents)
        string(REPLACE "\${EXECUTABLE_NAME}" "${_helper_output_name}" _plist_contents ${_plist_contents})
        string(REPLACE "\${PRODUCT_NAME}" "${_helper_output_name}" _plist_contents ${_plist_contents})
        string(REPLACE "\${BUNDLE_ID_SUFFIX}" "${_plist_suffix}" _plist_contents ${_plist_contents})
        file(WRITE ${_helper_info_plist} ${_plist_contents})

        # Create Helper executable target.
        add_executable(${_helper_target} MACOSX_BUNDLE ${WEBBROWSER_HELPER_SOURCES})
		# Inviwo: Use Inviwo properties instead of CEF 
        #SET_EXECUTABLE_TARGET_PROPERTIES(${_helper_target})
        add_dependencies(${_helper_target} libcef_dll_wrapper)
        target_link_libraries(${_helper_target} libcef_dll_wrapper ${CEF_STANDARD_LIBS} warn::warn)
        set_target_properties(${_helper_target} PROPERTIES
            MACOSX_BUNDLE_INFO_PLIST ${_helper_info_plist}
            # Remove XCode warning (must match plist)
            XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "org.inviwo.helper${_plist_suffix}"
            # See also https://www.magpcss.org/ceforum/viewtopic.php?f=6&t=16481&start=10
            XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS "${CMAKE_CURRENT_SOURCE_DIR}/resources/mac/helper${_target_suffix}-entitlements.plist"
            OUTPUT_NAME ${_helper_output_name}
        )
        # Output paths for the app bundles: bin/configuration, for example bin/Debug.
        # Note: A custom app must change IVW_APP_INSTALL_NAME to the name of the app.
        set(CEF_APP "$<TARGET_BUNDLE_DIR:${_helper_target}>/../${IVW_APP_INSTALL_NAME}.app")

        set(_helper_app "$<TARGET_BUNDLE_DIR:${_helper_target}>") # full/path/to/helper.app
        if(USE_SANDBOX)
            target_link_libraries(${_helper_target} cef_sandbox_lib)
        endif()
        target_include_directories(${_helper_target} PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
            "${CEF_INCLUDE_PATH}"
        )

        # Use inviwo preprocessor definitions
        ivw_define_standard_definitions(WebBrowser ${_helper_target})
        ivw_define_standard_properties(${_helper_target})
        ivw_suppress_compiler_warnings(${_helper_target})
        target_compile_definitions(${_helper_target} PRIVATE
            $<$<BOOL:${USE_SANDBOX}>:CEF_USE_SANDBOX>
        )
        # Put in ext folder in developer IDE
        ivw_folder(${_helper_target} ext/CEF)
        # Add the Helper as a dependency of the main executable target.
        add_dependencies(${CEF_TARGET} "${_helper_target}")

        # Copy files into the bin directory (used during development)
        add_custom_command(
            TARGET ${_helper_target}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${_helper_app}"
            "${CEF_APP}/Contents/Frameworks/${_helper_output_name}.app"
            VERBATIM
        )
        # Copy the helper app bundle into the Frameworks directory (used by installer)
        install(DIRECTORY "${_helper_app}"
            DESTINATION "${IVW_APP_INSTALL_NAME}.app/Contents/Frameworks/${_helper_output_name}.app"
            COMPONENT Application)
    endforeach()

elseif (WIN32)
	# Inviwo: Needed? (Windows only) Add the custom manifest files to the executable.
	#ADD_WINDOWS_CEF_MANIFEST("${CEF_TARGET_OUT_DIR}" "${WEBBROWSER_MODULE_PATH}" "${CEF_HELPER_TARGET}" "exe")

	# Copy helper executable into the bin directory (used by installer)
	install(TARGETS ${CEF_HELPER_TARGET} RUNTIME DESTINATION "${IVW_RUNTIME_INSTALL_DIR}" COMPONENT Application)
else () # Linux
	# Copy helper executable into the bin directory (used by installer)
	install(TARGETS ${CEF_HELPER_TARGET} RUNTIME DESTINATION "${IVW_RUNTIME_INSTALL_DIR}" COMPONENT Application)
endif()
