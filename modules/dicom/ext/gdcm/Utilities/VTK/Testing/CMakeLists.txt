add_subdirectory(Cxx)

if(VTK_WRAP_PYTHON)
  add_subdirectory(Python)
endif()

if(GDCM_WRAP_JAVA)
  if(VTK_WRAP_JAVA)
    if(EXISTS ${GDCM_VTK_JAVA_JAR})
      add_subdirectory(Java)
    endif()
  endif()
endif()
