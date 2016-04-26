if(IVW_QT_APPLICATION OR IVW_TINY_QT_APPLICATION)
    find_package(Qt5Core QUIET REQUIRED)
    find_package(Qt5Gui QUIET REQUIRED)
    find_package(Qt5Widgets QUIET REQUIRED)
   
    if(IVW_QT_APPLICATION)
        find_package(Qt5Help QUIET REQUIRED)
    endif()      
      
    qt5_wrap_cpp(MOC_FILES ${MOC_FILES})
endif()

ivw_add_minimal_applications()

if(IVW_QT_APPLICATION)
    add_subdirectory(inviwo)
endif()