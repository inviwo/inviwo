#add_subdirectory(Cxx Python)
add_subdirectory(Cxx)

if(GDCM_WRAP_CSHARP)
  add_subdirectory(Csharp)
endif()

if(GDCM_WRAP_JAVA)
  if(VTK_WRAP_JAVA)
    if(EXISTS ${GDCM_VTK_JAVA_JAR})
      add_subdirectory(Java)
    endif()
  endif()
endif()
