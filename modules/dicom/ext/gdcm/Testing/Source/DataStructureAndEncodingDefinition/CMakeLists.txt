add_subdirectory(Cxx)

if(GDCM_WRAP_PYTHON)
  add_subdirectory(Python)
endif()

if(GDCM_WRAP_JAVA)
  add_subdirectory(Java)
endif()
