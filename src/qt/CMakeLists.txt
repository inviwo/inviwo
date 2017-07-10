if(IVW_QT_APPLICATION AND NOT IVW_MODULE_QTWIDGETS)
    set(IVW_MODULE_QTWIDGETS ON CACHE BOOL "Build Inviwo Qt Widgets" FORCE)
    ivw_message("IVW_MODULE_QTWIDGETS was set to build, due to dependency towards IVW_QT_APPLICATION")
endif()
if(IVW_QT_APPLICATION AND NOT IVW_QT_APPLICATION_BASE)
    set(IVW_QT_APPLICATION_BASE ON CACHE BOOL "Build base for qt applications. Used by Qt network editor and Qt minimal application" FORCE)
    ivw_message("IVW_QT_APPLICATION_BASE was set to build, due to dependency towards IVW_QT_APPLICATION")
endif()

if(IVW_QT_APPLICATION OR IVW_QT_APPLICATION_BASE)
    find_package(Qt5Core QUIET)
    if(NOT Qt5Core_FOUND) 
        ivw_message(FATAL_ERROR 
            "The inviwo app requiers Qt5, we were not able to find it. Consider adding Qt to the "
            "CMAKE_PREFIX_PATH, i.e. start cmake with the option -DCMAKE_PREFIX_PATH=\"/path/to/qt5\"")
    endif()

    # Create a qt.conf file in the same dir as the executable to help qt find its plugins
    # http://doc.qt.io/qt-5/qt-conf.html
    set(QT_BASE_PATH "${Qt5Core_DIR}/../../..")
    foreach(conf ${CMAKE_CONFIGURATION_TYPES})
        configure_file("${IVW_CMAKE_TEMPLATES}/qt_template.conf" "${EXECUTABLE_OUTPUT_PATH}/${conf}/qt.conf" @ONLY)
    endforeach()

    if(IVW_PACKAGE_PROJECT)
        if(WIN32 AND Qt5Core_FOUND)
            set(QT_BINARY_DIR "${Qt5Core_DIR}/../../../bin")
            install(FILES ${QT_BINARY_DIR}/Qt5CLucene${CMAKE_DEBUG_POSTFIX}.dll DESTINATION bin COMPONENT qt_modules CONFIGURATIONS Debug)
            install(FILES ${QT_BINARY_DIR}/Qt5CLucene.dll DESTINATION bin COMPONENT qt_modules CONFIGURATIONS Release)
            install(DIRECTORY ${QT_BINARY_DIR} DESTINATION bin/../ COMPONENT qt_widgets FILES_MATCHING PATTERN "icudt5*")
            install(DIRECTORY ${QT_BINARY_DIR} DESTINATION bin/../ COMPONENT qt_widgets FILES_MATCHING PATTERN "icuin5*")
            install(DIRECTORY ${QT_BINARY_DIR} DESTINATION bin/../ COMPONENT qt_widgets FILES_MATCHING PATTERN "icuuc5*")
        endif()
    endif()

    if(IVW_QT_APPLICATION_BASE)
        add_subdirectory(applicationbase)
    endif()

    if(IVW_QT_APPLICATION)
        add_subdirectory(editor)
    endif()
    
endif()