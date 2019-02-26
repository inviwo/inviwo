cmake_minimum_required(VERSION 2.8.7)

# Choose behavior based on whether we are building inside the GDCM tree.
if(GDCM_BINARY_DIR)
add_subdirectory(Cxx)

if(GDCM_WRAP_PYTHON)
  add_subdirectory(Python)
endif()

if(GDCM_WRAP_CSHARP)
  add_subdirectory(Csharp)
endif()

if(GDCM_WRAP_JAVA)
  add_subdirectory(Java)
endif()

else()

  # We are building outside the GDCM tree.  Build the examples directly.
  project(GDCMExamples)

  # Find and load the VTK configuration.
  find_package(GDCM REQUIRED)
  include(${GDCM_USE_FILE})

  add_subdirectory(Cxx)

endif()
