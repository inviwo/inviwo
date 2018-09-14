subdirs(Cxx)

if(VTK_WRAP_PYTHON)
  subdirs(Python)
endif()

if(GDCM_WRAP_JAVA)
  if(VTK_WRAP_JAVA)
    if(EXISTS ${GDCM_VTK_JAVA_JAR})
      subdirs(Java)
    endif()
  endif()
endif()
