# Process running CEF browser
set(WEBBROWSER_HELPER_SOURCES
    app_switches.h
    app_switches.cpp
    webrendererapp.h
    webrendererapp.cpp
    cefprocesshelper.cpp
)

set(WEBBROWSER_RESOURCES_WINDOWS
  inviwo_web_helper.exe.manifest # Resolves "Check failed: fallback_available" in debug mode
)
set(WEBBROWSER_RESOURCES_MACOSX
    mac/helper-Info.plist
)
APPEND_PLATFORM_SOURCES(WEBBROWSER_RESOURCES)
source_group(WebBrowser\\\\resources FILES ${WEBBROWSER_RESOURCES})

set(WEBBROWSER_HELPER_SOURCES
    ${WEBBROWSER_HELPER_SOURCES}
    ${WEBBROWSER_RESOURCES}
)

# place helper in separate executable. CEF uses app name + helper by default
set(CEF_HELPER_TARGET "inviwo_helper" CACHE INTERNAL "CEF_HELPER_TARGET")
set(CEF_HELPER_OUTPUT_NAME "Inviwo Helper")

add_executable(${CEF_HELPER_TARGET} WIN32 MACOSX_BUNDLE ${WEBBROWSER_HELPER_SOURCES})
add_dependencies(${CEF_HELPER_TARGET} libcef_dll_wrapper)
# Build helper before module so that we can copy it
add_dependencies(inviwo-module-webbrowser ${CEF_HELPER_TARGET})
target_link_libraries(${CEF_HELPER_TARGET} PRIVATE 
    $<$<NOT:$<BOOL:${OS_MACOSX}>>:libcef_lib>
    libcef_dll_wrapper 
    ${CEF_STANDARD_LIBS}
    inviwo::warn
)
target_include_directories(${CEF_HELPER_TARGET} PRIVATE 
	${CMAKE_CURRENT_SOURCE_DIR}
	"${CEF_INCLUDE_PATH}"
)

# Use inviwo preprocessor definitions  
ivw_define_standard_definitions(WebBrowser ${CEF_HELPER_TARGET})
ivw_define_standard_properties(${CEF_HELPER_TARGET})
ivw_suppress_compiler_warnings(${CEF_HELPER_TARGET})

# Put in ext folder in developer IDE
ivw_folder(${CEF_HELPER_TARGET} ext/CEF)

if (OS_MACOSX)
    # Output paths for the app bundles: bin/configuration, for example bin/Debug.
    # Note: This is hardcoded to inviwo app.
    # Other apps need to copy the helper and the cef framework to its framework dir,
    # i.e. add the two commands add_custom_command and install below.
    set(CEF_APP "$<TARGET_BUNDLE_DIR:${CEF_HELPER_TARGET}>/../inviwo.app")
    set(CEF_HELPER_APP "$<TARGET_BUNDLE_DIR:${CEF_HELPER_TARGET}>") # full/path/to/helper.app

    # Copy files into the bin directory (used during development)
    add_custom_command(
        TARGET ${CEF_HELPER_TARGET}
        POST_BUILD
        # Copy the CEF framework into the Frameworks directory.
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CEF_BINARY_DIR}/Chromium Embedded Framework.framework"
        "${CEF_APP}/Contents/Frameworks/Chromium Embedded Framework.framework"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CEF_HELPER_APP}"
        "${CEF_APP}/Contents/Frameworks/${CEF_HELPER_OUTPUT_NAME}.app"
        VERBATIM
    )
    # Copy the helper app bundle into the Frameworks directory (used by installer)
    install(DIRECTORY "${CEF_HELPER_APP}"
            DESTINATION "Inviwo.app/Contents/Frameworks/${CEF_HELPER_OUTPUT_NAME}.app"
            COMPONENT inviwo-module-webbrowser)

    set_target_properties(${CEF_HELPER_TARGET} PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/mac/helper-Info.plist
    )
elseif (WIN32)
	# Inviwo: Needed? (Windows only) Add the custom manifest files to the executable.
	#ADD_WINDOWS_CEF_MANIFEST("${CEF_TARGET_OUT_DIR}" "${WEBBROWSER_MODULE_PATH}" "${CEF_HELPER_TARGET}" "exe")

	# Copy helper executable into the bin directory (used by installer)
	install(TARGETS ${CEF_HELPER_TARGET} RUNTIME DESTINATION bin COMPONENT inviwo-module-webbrowser)
else () # Linux
	# Copy helper executable into the bin directory (used by installer)
	install(TARGETS ${CEF_HELPER_TARGET} RUNTIME DESTINATION bin COMPONENT inviwo-module-webbrowser)
endif()